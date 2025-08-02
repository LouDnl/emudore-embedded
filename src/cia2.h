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
#ifndef EMUDORE_CIA2_H
#define EMUDORE_CIA2_H

#include "io.h"
#include "cpu.h"
#include "memory.h"

/**
 * @brief MOS 6526 Complex Interface Adapter #2
 *
 * - Memory area : $DD00-$DDFF
 * - Tasks       : Serial bus, RS-232, VIC banking, NMI control
 */
class Cia2
{
  private:
    Cpu *cpu_;
    IO *io_;
    Memory *mem_;

    unsigned int prev_cpu_cycles_;

    /* From cRSID/C64.c */
    unsigned short _fake_samplerate = 44100;
    short _cia_secondcount = 0;
    short _cia_tenthsecondcount = (_fake_samplerate/10);

  public:
    Cia2();
    void reset(void);
    void cpu(Cpu *v){ cpu_ = v;};
    void memory(Memory *v){ mem_ = v;};
    void io(IO *v){ io_ = v;};

    void write_register(uint8_t r, uint8_t v);
    uint8_t read_register(uint8_t r);
    uint16_t vic_base_address();
    bool emulate();

    /* constants */
    enum kInputMode
    {
      kModePHI2,
      kModeCNT,
      kModeTimerA,
      kModeTimerACNT
    };
    enum kRunMode
    {
      kModeRestart,
      kModeOneTime
    };
};

#endif
