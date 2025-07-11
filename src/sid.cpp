

#include "sid.h"
#include "util.h"

#define NANO 1000000000L
struct timespec m_StartTime, m_LastTime, now;
static double us_CPUcycleDuration               = NANO / (float)cycles_per_sec;          /* CPU cycle duration in nanoseconds */
static double us_InvCPUcycleDurationNanoSeconds = 1.0 / (NANO / (float)cycles_per_sec);  /* Inverted CPU cycle duration in nanoseconds */
typedef std::chrono::nanoseconds duration_t;   /* Duration in nanoseconds */

Sid::Sid()
{
  // m_StartTime = m_LastTime = now = std::chrono::steady_clock::now();
  // clock_gettime(CLOCK_REALTIME, &m_StartTime);
  timespec_get(&m_StartTime, TIME_UTC);
  m_LastTime = m_StartTime;
  usbsid = new USBSID_NS::USBSID_Class();
  if(usbsid->USBSID_Init(true, true) != 0) {
    us_ = false;
  } else {
    us_ = true;
  }
  sid_main_clk = sid_alarm_clk = 0;
}

Sid::~Sid()
{
  if(us_) {
    usbsid->USBSID_Reset();
    usbsid->USBSID_Close();
    us_ = false;
  }
}

void Sid::reset()
{
  sid_main_clk = sid_alarm_clk = cpu_->cycles();
  if (us_) usbsid->USBSID_Reset();
}

uint_fast64_t Sid::wait_ns(unsigned int cycles)
{ /* TODO: Account for time spent in function calculating */
  timespec_get(&now, TIME_UTC);
  double dur = cycles * us_CPUcycleDuration;  /* duration in nanoseconds */
  // HAS NEGATIVE CYCLES, OOPS
  // auto target_time = m_LastTime.tv_nsec + dur;  /* ns to wait since m_LastTime (now + duration for actual wait time minus time spent) */
  // GOOD BUT TOO SLOW FOR DIGI
  auto target_time = now.tv_nsec + dur;  /* ns to wait since m_LastTime (now + duration for actual wait time) */
  duration_t target_delta = (duration_t)(int_fast64_t)(target_time - now.tv_nsec);
  auto wait_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(target_delta);
  // printf("%ld\n",wait_nsec.count());
  if (wait_nsec.count() > 0) {
    std::this_thread::sleep_for(wait_nsec);
  }
  int_fast64_t waited_cycles = (wait_nsec.count() * us_InvCPUcycleDurationNanoSeconds);
  timespec_get(&m_LastTime, TIME_UTC);
  return waited_cycles;

}

void Sid::sid_flush()
{
  const unsigned int now = cpu_->cycles();
  unsigned int cycles = (now - sid_main_clk);
  // printf("SID Flush called @ %u cycles, last was at %u, diff %u\n",
    // now, sid_main_clk, cycles);
  while(cycles >= 19950) { /* TODO: USE ACTUAL HZ AND CIA TIMING  */
    printf("SID Flush called @ %u cycles, last was at %u, diff %u\n",
    now, sid_main_clk, cycles);
    cycles -= 19550;
    wait_ns(cycles);
  }
  sid_main_clk = now;
  return;
}

unsigned int Sid::sid_delay()
{
  unsigned int now = cpu_->cycles();
  if (now < sid_main_clk) {
    sid_main_clk = now;
    return 0;
  }
  unsigned int cycles = (now - sid_main_clk);
  while (cycles > 0xffff) {
    // wait_ns(cycles);
    cycles -= 0xffff;
  }
  sid_main_clk = now;
  wait_ns(cycles);
  return cycles;
}

uint8_t Sid::read_register(uint16_t r)
{
  uint8_t v = mem_->read_byte_no_io(r);
  if (us_) {
    // printf("[WR] $%04X:%02X %u\n", r, v, cycles);
  } else {
    printf("[RD] $%04X:%02X\n", r, v);
    // printf("[RD] $%04X:%02X %u\n", r, v, cycles);
  }
  return v;
}

void Sid::write_register(uint16_t r, uint8_t v)
{
  // unsigned int now = cpu_->cycles();
  // unsigned int cycles = (now - sid_main_clk);
  unsigned int cycles = sid_delay();
  mem_->write_byte_no_io(r,v); /* Write to memory space */
  // if (cycles) wait_ns(cycles);
  if (us_) {
    usbsid->USBSID_WriteRingCycled((uint8_t)(r&0xFF), v, cycles);
    // printf("[WR] $%04X:%02X %u\n", r, v, cycles);
  } else {
    printf("[WR] $%04X:%02X %u\n", r, v, cycles);
  }
  // sid_main_clk = now;
}
