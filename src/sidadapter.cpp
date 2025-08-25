/*
 * MOS Sound Interface Device (SID) USB hardware communication
 * Copyright (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * sidadapter.h
 *
 * Made for emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if DESKTOP
#include <time.h>
#endif

#include <c64.h>
#include <sidfile.h>

#if EMBEDDED
extern "C" uint16_t cycled_delay_operation(uint16_t cycles);
extern "C" void cycled_write_operation(uint8_t address, uint8_t data, uint16_t cycles);
extern "C" uint16_t cycled_delayed_write_operation(uint8_t address, uint8_t data, uint16_t cycles);
extern "C" uint8_t cycled_read_operation(uint8_t address, uint16_t cycles);
extern "C" void reset_sid(void);
#endif

#if DESKTOP
#define NANO 1000000000L
struct timespec m_StartTime, m_LastTime, timenow;
static double us_CPUcycleDuration               = NANO / (float)cycles_per_sec;          /* CPU cycle duration in nanoseconds */
static double us_InvCPUcycleDurationNanoSeconds = 1.0 / (NANO / (float)cycles_per_sec);  /* Inverted CPU cycle duration in nanoseconds */
typedef std::chrono::nanoseconds duration_t;   /* Duration in nanoseconds */
struct timespec m_test1, m_test2; /* TODO: REMOVE */
#endif

Sid::Sid(C64 * c64) :
  c64_(c64)
{
#if USBSID_DRIVER
  // m_StartTime = m_LastTime = now = std::chrono::steady_clock::now();
  // clock_gettime(CLOCK_REALTIME, &m_StartTime);
  timespec_get(&m_StartTime, TIME_UTC);
  m_LastTime = m_StartTime;
  m_test1 = m_test2 = m_StartTime; /* TODO: REMOVE */
  usbsid = new USBSID_NS::USBSID_Class();
  if(usbsid->USBSID_Init(true, true) != 0) {
    us_ = false;
  } else {
    usbsid->USBSID_SetClockRate(USBSID_NS::PAL, true);
    us_ = true;
  }
#endif
  sid_main_clk = sid_flush_clk = sid_delay_clk = sid_write_clk = sid_read_clk = 0;
  sid_read_cycles = sid_write_cycles = 0;
}

Sid::~Sid()
{
#if USBSID_DRIVER
  if(us_) {
    usbsid->USBSID_Reset();
    usbsid->USBSID_Close();
    us_ = false;
  }
#endif
}

void Sid::reset()
{
  sid_main_clk = sid_flush_clk = sid_delay_clk = sid_write_clk = sid_read_clk = c64_->cpu_->cycles();
  sid_read_cycles = sid_write_cycles = 0;
  // TODO: Reset memory registers to 0
  #if DESKTOP && USBSID_DRIVER
  if (us_) usbsid->USBSID_Reset();
  #elif EMBEDDED
  reset_sid();
  #endif
}

#if DESKTOP && !USBSID_DRIVER
uint_fast64_t Sid::wait_ns(unsigned int cycles)
{ /* TODO: Account for time spent in function calculating */
  timespec_get(&timenow, TIME_UTC);
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
  auto target_time = timenow.tv_nsec + dur;  /* ns to wait since m_LastTime (now + duration for actual wait time) */
  duration_t target_delta = (duration_t)(int_fast64_t)(target_time - timenow.tv_nsec);
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
#endif

void Sid::sid_flush()
{
  const unsigned int now = c64_->cpu_->cycles();
  unsigned int cycles = (now - /* sid_flush_clk */sid_main_clk);
  // printf("SID Flush called for %d cycles delay @ %u, last was at %u (diff %u) main clock %u\n",
  //   cycles, now, sid_flush_clk, (now-sid_flush_clk), sid_main_clk);
  if (now < sid_main_clk) { /* Reset / flush */
    sid_read_cycles = 0;
    sid_write_cycles = 0;
    sid_main_clk = sid_flush_clk = now;
    sid_delay_clk = sid_write_clk = sid_read_clk = now;
    return;
  }
  while(cycles > 0xFFFF) {
    // printf("SID Flush called @ %u cycles (cycles >= 0xFFFF), last was at %u, diff %u, main clock %u\n",
    //   now, sid_flush_clk, cycles, sid_main_clk);
    cycles -= 0xFFFF;
  }
  if (int(cycles - (sid_write_cycles+sid_read_cycles)) > 0) {
    #if DESKTOP && USBSID_DRIVER
    if (us_) usbsid->USBSID_WaitForCycle(cycles - (sid_write_cycles+sid_read_cycles));
    #elif EMBEDDED
    cycled_delay_operation(cycles - (sid_write_cycles+sid_read_cycles));
    #else
    wait_ns(cycles - (sid_write_cycles+sid_read_cycles));
    #endif
  }
  sid_main_clk = sid_delay_clk = sid_flush_clk = now;
  sid_read_cycles = sid_write_cycles = 0;
  return;
}

unsigned int Sid::sid_delay()
{
  #if USBSID_DRIVER
  timespec_get(&m_test1, TIME_UTC);
  #endif
  unsigned int now = c64_->cpu_->cycles();
  unsigned int cycles = (now - /* sid_delay_clk */sid_main_clk);
  while (cycles > 0xFFFF) {
    /* printf("SID delay called @ %u cycles (cycles > 0xFFFF), last was at %u, diff %u, main clock %u\n",
      now, sid_delay_clk, cycles, sid_main_clk); */
    cycles -= 0xFFFF;
    #if DESKTOP && USBSID_DRIVER
    if (us_) usbsid->USBSID_WaitForCycle(0xFFFF);
    #elif EMBEDDED
    cycled_delay_operation(0xFFFF);
    #else
    wait_ns(0xFFFF);
    #endif
  }
  sid_main_clk = sid_delay_clk = now;
  return cycles;
}

uint8_t Sid::read_register(uint8_t r, uint8_t sidno)
{
  uint8_t v = 0;
  r = ((sidno*0x20) | r);
  unsigned int cycles = sid_delay();
  #if DESKTOP
  v = c64_->mem_->read_byte_no_io(r); /* No hardware SID reading */
  #elif EMBEDDED
  v = cycled_read_operation(r,cycles); // TODO: cycled delayed read operation?
  #endif
  if (c64_->mem_->getlogrw(6)) {
    D("[RD%d] $%02X:%02X C:%u RDC:%u\n", sidno, r, v, cycles, sid_read_cycles);
  }
  sid_read_cycles += cycles;
  sid_main_clk = sid_read_clk = c64_->cpu_->cycles();
  return v;
}

void Sid::write_register(uint8_t r, uint8_t v, uint8_t sidno)
{
  r = ((sidno*0x20) | r);
  unsigned int cycles = sid_delay();
  #if DESKTOP && USBSID_DRIVER
  if (us_) {
    usbsid->USBSID_WaitForCycle(cycles);
    usbsid->USBSID_WriteRingCycled(r, v, cycles);
  }
  #elif EMBEDDED
  cycled_delayed_write_operation(r,v,cycles);
  #else
  wait_ns(cycles);
  #endif
  if (c64_->mem_->getlogrw(6)) {
    D("[WR%d] $%02X:%02X C:%u WRC:%u\n", sidno, r, v, cycles, sid_write_cycles);
  }
  sid_write_cycles += cycles;
  sid_main_clk = sid_write_clk = c64_->cpu_->cycles();
}
