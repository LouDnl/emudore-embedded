

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
}

Sid::~Sid()
{
  if(us_) {
    usbsid->USBSID_Reset();
    usbsid->USBSID_Close();
  }
}

uint_fast64_t Sid::wait_ns(unsigned int cycles)
{
  timespec_get(&now, TIME_UTC);
  double dur = cycles * us_CPUcycleDuration;  /* duration in nanoseconds */
  // auto target_time = m_LastTime.tv_nsec + dur;  /* ns to wait since m_LastTime (now + duration for actual wait time minus time spent) */
  auto target_time = now.tv_nsec + dur;  /* ns to wait since m_LastTime (now + duration for actual wait time) */
  duration_t target_delta = (duration_t)(int_fast64_t)(target_time - now.tv_nsec);
  auto wait_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(target_delta);
  if (wait_nsec.count() > 0) {
    std::this_thread::sleep_for(wait_nsec);
  }
  int_fast64_t waited_cycles = (wait_nsec.count() * us_InvCPUcycleDurationNanoSeconds);
  timespec_get(&m_LastTime, TIME_UTC);
  return waited_cycles;

}

void Sid::set_cycles()
{
  unsigned int now = cpu_->cycles();
  // unsigned int cycles = (now - prev_flush_cpu_cycles_);
  unsigned int cycles = (now - prev_cpu_cycles_);
  /* TODO: USE ACTUAL REFRESH RATE */
  unsigned int fr = 19950; /* ((1 / 50.125) * 1000000) */
  if (cycles >= fr) {
    printf("SID Flush called @ %u cycles, last was at %u, diff %u\n",
      now, prev_cpu_cycles_,
      cycles);
      wait_ns(fr);
  }
  // /* prev_cpu_cycles_ =  */prev_flush_cpu_cycles_ = now;
  /* prev_cpu_cycles_ =  */prev_cpu_cycles_ = now;
}

uint8_t Sid::read_register(uint16_t r)
{
  return mem_->read_byte_no_io(r);
}

void Sid::write_register(uint16_t r, uint8_t v)
{
  unsigned int now = cpu_->cycles();
  unsigned int cycles = (now - prev_cpu_cycles_);
  printf("[WR] $%04X:%02X %u\n", r, v, cycles);
  mem_->write_byte_no_io(r,v); /* Write to memory space */
  if (cycles) wait_ns(cycles);
  if (us_) {

    usbsid->USBSID_WriteRingCycled((uint8_t)(r&0xFF), v, cycles);
    // usbsid->USBSID_WriteCycled((uint8_t)(r&0xFF), v, (uint16_t)cycles);
  }
  prev_cpu_cycles_ = now;
}
