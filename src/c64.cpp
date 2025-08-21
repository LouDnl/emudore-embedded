/*
 * emudore, Commodore 64 emulator
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

#include <c64.h>
#include <util.h>


C64::C64(bool sdl, bool bin, bool crt, bool blog) :
  nosdl(sdl),
  isbinary(bin),
  havecart(crt),
  bankswlog(blog)
{ /* BUG: Auto PRG start seems broken at times? */
  /* create and init C64 */
  /* init cpu */
  cpu_  = new Cpu(this);
  /* init memory & DMA */
  mem_  = new Memory(this);
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
  /* Always create cart object */
  cart_ = new Cart(this);

  /* Resets before start */
  cpu_->reset();
  cia1_->reset();
  cia2_->reset();
  pla_->reset();
  cart_->reset();

 /* r2 support */
#ifdef DEBUGGER_SUPPORT
  debugger_ = new Debugger();
  debugger_->memory(mem_);
  debugger_->cpu(cpu_);
#endif
}

C64::~C64()
{
  delete cpu_;
  delete mem_;
  delete cia1_;
  delete cia2_;
  delete vic_;
  delete sid_;
  delete io_;
  delete pla_;
#ifdef DEBUGGER_SUPPORT
  delete debugger_;
#endif
}

void C64::start()
{
  /* main emulator loop */
  while(true)
  {
#ifdef DEBUGGER_SUPPORT
    if(!debugger_->emulate())
      break;
#endif /* TODO: Remove unneeded if statements (deprecate return values on some emulation runs) */
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
    pla_->emulate();
    if(havecart) { cart_->emulate(); }
    /* callback executes _after_ first emulation run */
    if(callback_ && !callback_())
      break;
  }
}

/**
 * @brief runs Klaus Dormann's 6502 test suite
 *
 * https://github.com/Klaus2m5/6502_65C02_functional_tests
 */
void C64::test_cpu()
{
  uint16_t pc=0;
  /* unmap C64 ROMs */
  mem_->write_byte(Memory::kAddrMemoryLayout, 0);
  /* load tests into RAM */
  mem_->load_ram("tests/6502_functional_test.bin",0x400);
  cpu_->pc(0x400);
  while(true)
  {
    if(pc == cpu_->pc())
    {
      D("infinite loop at %x\n",pc);
      break;
    }
    else if(cpu_->pc() == 0x3463)
    {
      D("test passed!\n");
      break;
    }
    pc = cpu_->pc();
    if(!cpu_->emulate())
      break;
  }
}
