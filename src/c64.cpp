/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * c64.cpp
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

#include "timer.cpp"

#include <c64.h>
#include <util.h>

#if EMBEDDED
#include "globals.h"
#endif

bool C64::log_timings = false;
bool C64::is_cynthcart = false;
bool C64::is_rsid = false;


C64::C64(
  bool sdl,
  bool bin, bool cart,
  bool blog, bool aci,
  #if DESKTOP
  const std::string &f
  #elif EMBEDDED
  uint8_t * b_, uint8_t * c_,
  uint8_t * k_, uint8_t * p_
  #endif
  ) :
  nosdl(sdl),
  isbinary(bin), /* Unused */
  havecart(cart),
  bankswlog(blog),
  acia(aci),
  #if DESKTOP
  cartfile(f)
  #elif EMBEDDED
  basic_(b_),
  chargen_(c_),
  kernal_(k_),
  binary_(p_)
  #endif
{
  /* create and init C64 */
  /* init cpu */
  cpu_  = new Cpu(this);
  /* init memory & DMA */
  mem_  = new Memory(this);
  /* init cart slot */
  cart_ = new Cart(this);
  /* init PLA */
  pla_  = new PLA(this);
  /* init cia1 */
  cia1_ = new Cia1(this);
  /* init cia2 */
  cia2_ = new Cia2(this);
  /* init vic-ii */
  vic_  = new Vic(this);
  /* init SID */
  sid_  = new Sid(this);
  /* init io */
  io_   = new IO(this,nosdl);

  /* Resets needed before start */
  cpu_->reset();
  cia1_->reset();
  cia2_->reset();

  /* r2 support */
  #if DEBUGGER_SUPPORT
  debugger_ = new Debugger();
  debugger_->memory(mem_);
  debugger_->cpu(cpu_);
  #endif

  runloop = true; /* Enable looping */
}

C64::~C64()
{
  runloop = false;
  delete cpu_;
  delete mem_;
  delete cia1_;
  delete cia2_;
  delete vic_;
  delete sid_;
  delete io_;
  delete pla_;
  #if DEBUGGER_SUPPORT
  delete debugger_;
  #endif

  #if EMBEDDED
  reset_sid();
  #endif
}

/**
 * @brief run c64 continuously
 */
void C64::start()
{
  BenchmarkTimer * BT;
  if (log_timings) BT = new BenchmarkTimer();
  static uint64_t _dbg,_cb,_cart,_cpu,_cia1,_cia2,_vic,_io,delay,delay_c;
  static unsigned int _prev_cycles;
  /* main emulator loop */
  while(runloop)
  {
    if (log_timings) BT->receive_data(cpu_->cycles(),delay,delay_c,_dbg,_cb,_cart,_cpu,_cia1,_cia2,_vic,_io);
    #if DEBUGGER_SUPPORT
    if (log_timings) BT->MeasurementStart();
    if(!debugger_->emulate()) break;
    if (log_timings) BT->MeasurementEnd();
    if (log_timings) _dbg = BT->MeasurementResult();
    #endif
    #if DESKTOP
    if (log_timings) BT->MeasurementStart();
    /* callback executes _before_ first emulation run */
    if(callback_ && cpu_->pc() == 0xa65c) callback_();
    if (log_timings) BT->MeasurementEnd();
    if (log_timings) _cb = BT->MeasurementResult();
    #endif
    /* Cart */
    if (log_timings) BT->MeasurementStart();
    if(!cart_->emulate()) break;
    if (log_timings) BT->MeasurementEnd();
    if (log_timings) _cart = BT->MeasurementResult();
    /* CPU */
    if (log_timings) BT->MeasurementStart();
    if(!cpu_->emulate()) break;
    if (log_timings) BT->MeasurementEnd();
    if (log_timings) _cpu = BT->MeasurementResult();
    /* CIA1 */
    if (log_timings) BT->MeasurementStart();
    if(!cia1_->emulate()) break;
    if (log_timings) BT->MeasurementEnd();
    if (log_timings) _cia1 = BT->MeasurementResult();
    /* CIA2 */
    if (log_timings) BT->MeasurementStart();
    if(!cia2_->emulate()) break;
    if (log_timings) BT->MeasurementEnd();
    if (log_timings) _cia2 = BT->MeasurementResult();
    /* VIC-II */
    if (log_timings) BT->MeasurementStart();
    if(!vic_->emulate()) break;
    if (log_timings) BT->MeasurementEnd();
    if (log_timings) _vic = BT->MeasurementResult();
    /* IO (SDL2 keyboard input) */
    if (log_timings) BT->MeasurementStart();
    if(!io_->emulate()) break;
    if (log_timings) BT->MeasurementEnd();
    if (log_timings) _io = BT->MeasurementResult();

    _prev_cycles = cpu_->cycles();
  }
}

/**
 * @brief run a single c64 emulation loop
 * ignores emulation return statements and
 * has no debugger support
 */
// uint64_t cart_c = 0, cpu_c = 0, cia1_c = 0, cia2_c = 0, vic_c = 0, io_c = 0;
uint64_t cart_b = 0, cpu_b = 0, cia1_b = 0, cia2_b = 0, vic_b = 0, io_b = 0;
unsigned int cart_a = 0, cpu_a = 0, cia1_a = 0, cia2_a = 0, vic_a = 0, io_a = 0;
char enemy = '0';
unsigned int C64::emulate()
{
  if (runloop) {
    #if DESKTOP
    if(callback_) callback_();
    #endif
    /* Cart */
    #if EMBEDDED
    cart_b = to_us_since_boot(get_absolute_time());
    #endif
    cart_->emulate();
    #if EMBEDDED
    cart_a = (to_us_since_boot(get_absolute_time()) - cart_b);
    if (cart_a > 3) enemy = 'T';
    #endif
    /* CPU */
    #if EMBEDDED
    cpu_b = to_us_since_boot(get_absolute_time());
    #endif
    cpu_->emulate();
    #if EMBEDDED
    cpu_a = (to_us_since_boot(get_absolute_time()) - cpu_b);
    if (cpu_a > 3) enemy = 'C';
    #endif
    /* CIA1 */
    #if EMBEDDED
    cia1_b = to_us_since_boot(get_absolute_time());
    #endif
    cia1_->emulate();
    #if EMBEDDED
    cia1_a = (to_us_since_boot(get_absolute_time()) - cia1_b);
    if (cia1_a > 3) enemy = '1';
    #endif
    /* CIA2 */
    #if EMBEDDED
    cia2_b = to_us_since_boot(get_absolute_time());
    #endif
    cia2_->emulate();
    #if EMBEDDED
    cia2_a = (to_us_since_boot(get_absolute_time()) - cia2_b);
    if (cia2_a > 3) enemy = '2';
    #endif
    /* VIC-II */
    #if EMBEDDED
    vic_b = to_us_since_boot(get_absolute_time());
    #endif
    vic_->emulate();
    #if EMBEDDED
    vic_a = (to_us_since_boot(get_absolute_time()) - vic_b);
    if (vic_a > 3) enemy = 'V';
    #endif
    /* IO (SDL2 keyboard input) */
    #if EMBEDDED
    io_b = to_us_since_boot(get_absolute_time());
    #endif
    io_->emulate();
    #if EMBEDDED
    io_a = (to_us_since_boot(get_absolute_time()) - io_b);
    if (io_a > 3) enemy = 'I';
    #endif
    /* return current cpu cycles */
    return cpu_->cycles();
  }
  return 0;
}

/**
 * @brief run a single c64 emulation loop
 * ignores emulation return statements and
 * has no debugger support
 */
unsigned int  C64::emulate_specified(
  bool cpu, bool cia1, bool cia2,
  bool vic, bool io,   bool cart
)
{
  if (runloop) {
    #if DESKTOP
    if(callback_) callback_();
    #endif
    /* Cart */
    if (cart) cart_->emulate();
    /* CPU */
    if (cpu) cpu_->emulate();
    /* CIA1 */
    if (cia1) cia1_->emulate();
    /* CIA2 */
    if (cia2) cia2_->emulate();
    /* VIC-II */
    if (vic) vic_->emulate();
    /* IO (SDL2 keyboard input) */
    if (io) {
      #if DESKTOP
      if (!
      #endif
        io_->emulate()
      #if EMBEDDED
        ;
      #endif
      #if DESKTOP
      ) {
        runloop = false;
      }
      #endif
    }
    /* return current cpu cycles */
    return cpu_->cycles();
  }
  return 0;
}
