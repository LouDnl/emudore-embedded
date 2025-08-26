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

#include <c64.h>
#include <util.h>

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
  /* main emulator loop */
  while(runloop)
  {
    #if DEBUGGER_SUPPORT
    if(!debugger_->emulate()) break;
    #endif
    #if DESKTOP
    /* callback executes _before_ first emulation run */
    if(callback_ && !callback_()) break;
    #endif
    /* Cart */
    if(!cart_->emulate()) break;
    /* CPU */
    if(!cpu_->emulate()) break;
    /* CIA1 */
    if(!cia1_->emulate()) break;
    /* CIA2 */
    if(!cia2_->emulate()) break;
    /* VIC-II */
    if(!vic_->emulate()) break;
    /* IO (SDL2 keyboard input) */
    if(!io_->emulate()) break;
  }
}

/**
 * @brief run a single c64 emulation loop
 * ignores emulation return statements and
 * has no debugger support
 */
unsigned int C64::emulate()
{
  if (runloop) {
    /* Cart */
    cart_->emulate();
    /* CPU */
    cpu_->emulate();
    /* CIA1 */
    cia1_->emulate();
    /* CIA2 */
    cia2_->emulate();
    /* VIC-II */
    vic_->emulate();
    /* IO (SDL2 keyboard input) */
    io_->emulate();
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
    if (io) io_->emulate();
    /* return current cpu cycles */
    return cpu_->cycles();
  }
  return 0;
}
