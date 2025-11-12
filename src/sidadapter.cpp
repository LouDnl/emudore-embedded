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

extern "C" uint8_t cycled_read_operation(uint8_t address, uint16_t cycles);

extern "C" void write_operation(uint8_t address, uint8_t data);
extern "C" void cycled_write_operation(uint8_t address, uint8_t data, uint16_t cycles);
extern "C" uint16_t cycled_delayed_write_operation(uint8_t address, uint8_t data, uint16_t cycles);

extern "C" void reset_sid(void);
#endif

#if DESKTOP && !USBSID_DRIVER
#define NANO 1000000000L
static double us_CPUcycleDuration               = NANO / (float)cycles_per_sec;          /* CPU cycle duration in nanoseconds */
#endif

unsigned int Sid::sid_main_clk = 0;
unsigned int Sid::sid_flush_clk = 0;
unsigned int Sid::sid_delay_clk = 0;
unsigned int Sid::sid_read_clk = 0;
unsigned int Sid::sid_write_clk = 0;
unsigned int Sid::sid_read_cycles = 0;
unsigned int Sid::sid_write_cycles = 0;


Sid::Sid(C64 * c64) :
  c64_(c64)
{
#if USBSID_DRIVER
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

  D("[EMU] SID adapter initialized.\n");
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
{ auto start = std::chrono::steady_clock::now();
  auto duration_ns = duration_t((long)(cycles * us_CPUcycleDuration));
  auto target_time = (start + duration_ns);

  do {
  } while (std::chrono::steady_clock::now() < target_time);

  auto end = std::chrono::steady_clock::now();
  auto actual_ns = static_cast<long long>(std::chrono::duration_cast<duration_t>(end - start).count());
  return (uint_fast64_t)actual_ns;
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
    #if DESKTOP && !USBSID_DRIVER
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
  #endif
  unsigned int now = c64_->cpu_->cycles();
  unsigned int cycles = (now - /* sid_delay_clk */sid_main_clk);
  while (cycles > 0xFFFF) {
    /* printf("SID delay called @ %u cycles (cycles > 0xFFFF), last was at %u, diff %u, main clock %u\n",
      now, sid_delay_clk, cycles, sid_main_clk); */
    cycles -= 0xFFFF;
    #if DESKTOP && !USBSID_DRIVER
    wait_ns(0xFFFF);
    #endif
  }
  sid_main_clk /* = sid_delay_clk */ = now;
  return cycles;
}

uint8_t Sid::read_register(uint8_t r, uint8_t sidno)
{
  uint8_t v = 0;
  r = ((sidno*0x20) | r);
  unsigned int cycles = sid_delay();
  #if DESKTOP
  v = c64_->mem_->read_byte_no_io(r);  /* No hardware SID reading */
  #elif EMBEDDED
  v = cycled_read_operation(r,0);  /* no delay cycles */
  #else
  wait_ns(cycles);
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
  // sid_main_clk = sid_write_clk = c64_->cpu_->cycles();
  r = ((sidno*0x20) | r);
  unsigned int cycles = sid_delay();
  #if DESKTOP && USBSID_DRIVER
  if (us_) {
    // usbsid->USBSID_WaitForCycle(cycles);
    usbsid->USBSID_WriteRingCycled(r, v, cycles);
  }
  #elif EMBEDDED
  cycled_write_operation(r,v,0);  /* no delay cycles */
  #else
  wait_ns(cycles);
  #endif
  if (c64_->mem_->getlogrw(6)) {
    D("[WR%d] $%02X:%02X C:%u WRC:%u\n", sidno, r, v, cycles, sid_write_cycles);
  }
  sid_write_cycles += cycles;
  sid_main_clk = sid_write_clk = c64_->cpu_->cycles();
}
