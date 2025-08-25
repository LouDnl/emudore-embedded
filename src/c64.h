/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * c64.h
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

#ifndef EMUDORE_C64_H
#define EMUDORE_C64_H

#if DESKTOP
#include <functional>
#endif

/* forward declarations */
class C64;
class Cpu;
class PLA;
class Cia1;
class Cia2;
class Vic;
class IO;
class Cart;
class Sid;

#include <memory.h>
#include <cpu.h>
#include <pla.h>
#include <cia1.h>
#include <cia2.h>
#include <vic.h>
#include <io.h>
#include <cart.h>
#include <sidadapter.h>
#include <util.h>

#if DEBUGGER_SUPPORT
#include <debugger.h>
#endif

#if EMBEDDED
extern "C" uint16_t cycled_delay_operation(uint16_t cycles);
extern "C" void cycled_write_operation(uint8_t address, uint8_t data, uint16_t cycles);
extern "C" uint16_t cycled_delayed_write_operation(uint8_t address, uint8_t data, uint16_t cycles);
extern "C" void reset_sid(void);
#endif


/**
 * @brief Commodore 64
 *
 * This class glues together all the different
 * components in a Commodore 64 computer
 */
class C64
{
  private:
    bool runloop;
  public: /* Public so subclasses can access them */
    C64(
      bool sdl, bool bin, bool cart,
      bool blog, bool aci,
    #if DESKTOP
      const std::string &f
    #elif EMBEDDED
      uint8_t * b_, uint8_t * c_,
      uint8_t * k_, uint8_t * p_
    #endif
    );
    ~C64();

    Cpu *cpu_;
    PLA *pla_;
    Memory *mem_;
    Cia1 *cia1_;
    Cia2 *cia2_;
    Vic *vic_;
    IO *io_;
    Cart *cart_;
    Sid *sid_;

  #if DESKTOP
  private:
    std::function<bool()> callback_;
  #if DEBUGGER_SUPPORT
    Debugger *debugger_;
  #endif
  public:
  #endif /* DESKTOP */
    bool nosdl = false;
    bool isbinary = false;
    bool bankswlog = false;
    /* Specific for CRT files */
    bool havecart = false;
    std::string cartfile;
    /* Specific for Midi like Cynthcart that uses MC68B50 */
    bool acia = false;

    #if EMBEDDED
    /* Binary pointers */
    uint8_t * basic_;
    uint8_t * chargen_;
    uint8_t * kernal_;
    uint8_t * binary_;
    #endif

    void emulate();
    void start();

    bool is_looping(void){return runloop;};
    bool disable_looping(void){runloop=false;return runloop;};

    #if DESKTOP
    void callback(std::function<bool()> cb){callback_ = cb;};
    #endif
    Cpu * cpu(){return cpu_;};
    PLA * pla(){return pla_;};
    Memory * memory(){return mem_;};
    Cia1 * cia1(){return cia1_;};
    Cia2 * cia2(){return cia2_;};
    Vic * vic(){return vic_;};
    IO * io(){return io_;};
    Cart * cart(){return cart_;};
    Sid * sid(){return sid_;};

};


#endif /* EMUDORE_C64_H */
