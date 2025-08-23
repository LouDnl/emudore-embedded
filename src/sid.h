/*
 * MOS Sound Interface Device (SID) USB hardware communication
 * Copyright (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * sid.h
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

#ifndef EMUDORE_SID_H
#define EMUDORE_SID_H


#include <cstdint>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include <USBSID.h>


/**
 * @brief MOS 6581 SID (Sound Interface Device)
 *
 * - Memory area : $D400-$D7FF
 * - Tasks       : Sound
 */
class Sid
{
  private:
    C64 *c64_;

    USBSID_NS::USBSID_Class *usbsid;
    bool us_ = false;

    unsigned int sid_main_clk = 0;
    unsigned int sid_flush_clk = 0;
    unsigned int sid_delay_clk = 0;
    unsigned int sid_read_clk = 0;
    unsigned int sid_write_clk = 0;
    unsigned int sid_write_cycles = 0;
    /* unsigned int sid_alarm_clk = 0; */

    bool sid_playing = false;
    bool logsidrw = false;

    uint_fast64_t wait_ns(unsigned int cycles);

  public:
    Sid(C64 * c64);
    ~Sid();

    void reset(void);

    unsigned int sid_delay();
    void sid_flush(void);
    uint8_t read_register(uint8_t r, uint8_t sidno);
    void write_register(uint8_t r, uint8_t v, uint8_t sidno);

    /* SID play workaround */
    void set_playing(bool playing) { sid_playing = playing; };
    bool isSIDplaying() { return sid_playing; };

    /* Utils */
    void set_sidrwlog(bool v) { logsidrw = v; };

};

#endif
