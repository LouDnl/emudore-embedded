
#include <time.h>

#include "sid.h"
#include "util.h"

#define NANO 1000000000L
struct timespec m_StartTime, m_LastTime, now;
static double us_CPUcycleDuration               = NANO / (float)cycles_per_sec;          /* CPU cycle duration in nanoseconds */
static double us_InvCPUcycleDurationNanoSeconds = 1.0 / (NANO / (float)cycles_per_sec);  /* Inverted CPU cycle duration in nanoseconds */
typedef std::chrono::nanoseconds duration_t;   /* Duration in nanoseconds */

struct timespec m_test1, m_test2; /* TODO: REMOVE */

Sid::Sid()
{
  // m_StartTime = m_LastTime = now = std::chrono::steady_clock::now();
  // clock_gettime(CLOCK_REALTIME, &m_StartTime);
  timespec_get(&m_StartTime, TIME_UTC);
  m_LastTime = m_StartTime;
  m_test1 = m_test2 = m_StartTime; /* TODO: REMOVE */
  usbsid = new USBSID_NS::USBSID_Class();
  if(usbsid->USBSID_Init(true, true) != 0) {
    us_ = false;
  } else {
    us_ = true;
  }
  sid_main_clk = sid_flush_clk = sid_delay_clk = sid_write_clk = sid_read_clk = sid_write_cycles = 0;
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
  sid_main_clk = sid_flush_clk = sid_delay_clk = sid_write_clk = sid_read_clk = cpu_->cycles();
  sid_write_cycles = 0;
  if (us_) usbsid->USBSID_Reset();
}

uint_fast64_t Sid::wait_ns(unsigned int cycles)
{ /* TODO: Account for time spent in function calculating */
  timespec_get(&now, TIME_UTC);
  long int dur = cycles * us_CPUcycleDuration;  /* duration in nanoseconds */
  int_fast64_t wait_nano = (dur - (m_test2.tv_nsec-m_test1.tv_nsec));
  struct timespec rem;
  struct timespec req= {
      // 0, (dur)
      0, wait_nano
      //  (int)(miliseconds / 1000),     /* secs (Must be Non-Negative) */
      //  (miliseconds % 1000) * 1000000 /* nano (Must be in range of 0 to 999999999) */
   };
  // HAS NEGATIVE CYCLES, OOPS
  // auto target_time = m_LastTime.tv_nsec + dur;  /* ns to wait since m_LastTime (now + duration for actual wait time minus time spent) */
  // GOOD BUT TOO SLOW FOR DIGI
  auto target_time = now.tv_nsec + dur;  /* ns to wait since m_LastTime (now + duration for actual wait time) */
  duration_t target_delta = (duration_t)(int_fast64_t)(target_time - now.tv_nsec);
  auto wait_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(target_delta);
  // printf("%ld\n",wait_nsec.count());
  timespec_get(&m_test2, TIME_UTC);
  // if (wait_nsec.count() > 0) {
  //   std::this_thread::sleep_for(wait_nsec-m_test2.tv_nsec);
  // }
  // nanosleep((const struct timespec[]){{0, dur}}, NULL);
  // nanosleep(&req,&req);
  nanosleep(&rem,&req);
  // int_fast64_t waited_cycles = (wait_nsec.count() * us_InvCPUcycleDurationNanoSeconds);
  // int_fast64_t waited_cycles = (dur - (m_test2.tv_nsec-m_test1.tv_nsec));
  // int_fast64_t waited_cycles = dur;
  timespec_get(&m_LastTime, TIME_UTC);

  // printf("T1 %lu T2 %lu DIFF %lu N %lu C %lu\n",
  //   m_test1.tv_nsec,m_test2.tv_nsec,
  //   (m_test2.tv_nsec-m_test1.tv_nsec),
  //   wait_nsec.count(),
  //   cycles);
  // return waited_cycles;
  return wait_nano;
}

void Sid::sid_flush()
{
  const unsigned int now = cpu_->cycles();
  unsigned int cycles = (now - sid_flush_clk);
  if (now < sid_main_clk) { /* Reset / flush */
    sid_write_cycles = 0;
    sid_main_clk = sid_flush_clk = now;
    sid_delay_clk = sid_write_clk = sid_read_clk = now;
    return;
  }
  // printf("SID Flush called @ %u cycles, last was at %u, diff %u\n",
    // now, sid_main_clk, cycles);
  while(cycles > 0xFFFF) {
    printf("SID Flush called @ %u cycles (cycles >= 0xFFFF), last was at %u, diff %u, main clock %u\n",
    now, sid_flush_clk, cycles, sid_main_clk);
    cycles -= 0xFFFF;
    // wait_ns(0xFFFF);
  }
  if (int(cycles - sid_write_cycles) > 0) {
    /* printf("SID Flush called @ %u cycles, last was at %u, diff %u main clock %u\n",
      now, sid_flush_clk, (cycles - sid_write_cycles), sid_main_clk); */
    wait_ns((cycles - sid_write_cycles));
  }
  sid_main_clk = sid_flush_clk = now;
  sid_write_cycles = 0;
  return;
}

unsigned int Sid::sid_delay(unsigned int now)
{
  timespec_get(&m_test1, TIME_UTC);
  /* unsigned int now = cpu_->cycles(); */
  unsigned int cycles = (now - sid_delay_clk);
  while (cycles > 0xffff) {
    /* printf("SID delay called @ %u cycles (cycles > 0xFFFF), last was at %u, diff %u, main clock %u\n",
      now, sid_delay_clk, cycles, sid_main_clk); */
    cycles -= 0xFFFF;
    wait_ns(0xFFFF);
  }
  sid_main_clk = sid_delay_clk = now;
  return cycles;
}

uint8_t Sid::read_register(uint8_t r, uint8_t sidno)
{
  uint8_t v = 0;
  if (us_) {
    v = mem_->read_byte_no_io(r); /* No reading for now */
    if (logsidrw) D("[RD] $%02X:%02X %u\n", r, v, cpu_->cycles()-sid_read_clk);
  } else {
    v = mem_->read_byte_no_io(r);
    // printf("[RD] $%02X:%02X\n", r, v);
    if (logsidrw) D("[RD] $%02X:%02X %u\n", r, v, cpu_->cycles()-sid_read_clk);
  }
  sid_main_clk = sid_read_clk = cpu_->cycles();
  return v;
}

void Sid::write_register(uint8_t r, uint8_t v, uint8_t sidno)
{
  r = ((sidno*0x20) | r);
  const unsigned int now = cpu_->cycles();
  unsigned int cycles = sid_delay(now);
  // mem_->write_byte_no_io(r,v); /* Write to memory space */
  // if (cycles) wait_ns(cycles);
  if (us_) {
    usbsid->USBSID_WriteRingCycled(r, v, cycles);
    if (logsidrw) D("[WR%d] $%02X:%02X %u\n", sidno, r, v, cycles);
  } else {
    if (logsidrw) D("[WR] $%02X:%02X C:%u WRC:%u\n", r, v, cycles, sid_write_cycles);
  }
  sid_write_cycles += cycles;
  sid_main_clk = sid_write_clk = now;
}
