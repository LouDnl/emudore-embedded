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
#ifndef EMUDORE_SID_H
#define EMUDORE_SID_H

#include <cstdint>

#include "cpu.h"
#include "memory.h"
#include "cia1.h"
#include "cia2.h"
#include "vic.h"
#include "io.h"

/**
 * @brief MOS 6581 SID (Sound Interface Device)
 *
 * - Memory area : $D400-$D7FF
 * - Tasks       : Sound
 */
class Sid
{
  private:
    Cpu *cpu_;
    Memory *mem_;
    Cia1 *cia1_;
    Cia2 *cia2_;
    Vic *vic_;
    IO *io_;

    unsigned int prev_flush_cpu_cycles_;
    unsigned int prev_cpu_cycles_;
  public:
    Sid();
    // ~Sid();
    void cpu(Cpu *v){cpu_ = v;};
    void mem(Memory *v){mem_ = v;};
    void cia1(Cia1 *v){cia1_ = v;};
    void cia2(Cia2 *v){cia2_ = v;};
    void vic(Vic *v){vic_ = v;};
    void io(IO *v){io_ = v;};

    void set_cycles(void);
    uint8_t read_register(uint8_t r);
    void write_register(uint16_t r, uint8_t v);
};

#endif
