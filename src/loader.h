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

#ifndef EMUDORE_LOADER_H
#define EMUDORE_LOADER_H


#include <fstream>
#include "c64.h"

/* Pre declarations */
class SidFile;


/**
 * @brief Program loader
 */
class Loader
{
  private:
    bool booted_up_ = false;
    C64 *c64_;
    IO *io_;
    Cpu *cpu_;
    Memory *mem_;
    PLA *pla_;
    Vic *vic_;
    Sid *sid_;
    SidFile *sidfile_;
    std::ifstream is_;
    enum kFormat
    {
      kNone,
      kBasic,
      kBIN,
      kPRG,
      kD64,
      kCRT,
      kSID
    };
    kFormat format_;

    uint16_t pl_loadaddr;
    uint16_t pl_initaddr;
    uint16_t pl_playaddr;
    uint16_t pl_datalength;
    uint8_t *pl_databuffer;
    uint32_t pl_sidspeed;
    int pl_songs;
    int pl_song_number;
    int pl_sidflags;
    int pl_curr_sidspeed;
    int pl_chiptype;
    int pl_clockspeed;
    int pl_sidversion;

    int pl_clock_speed;
    int pl_raster_lines;
    int pl_rasterrow_cycles;
    int pl_frame_cycles;
    int pl_refresh_rate;

    bool autorun = true;
    bool lowercase = false;
    bool memrwlog = false;
    bool cia1rwlog = false;
    bool cia2rwlog = false;
    bool sidrwlog = false;
    bool iorwlog = false;
    bool plarwlog = false;

    void load_basic();
    void load_bin();
    void load_prg();
    void load_d64();
    void load_crt();
    void load_sid();
    void load_sidplayerA(uint16_t play, uint16_t init, int songno);
    void load_sidplayerB(uint16_t play, uint16_t init, int songno);
    void load_sidplayerC(uint16_t play, uint16_t init, int songno);
    void print_sid_info(); /* TODO: Print on C64 screen */
    uint16_t read_short_le();
  public:
    Loader(C64 *c64);
    void bas(const std::string &f);
    void bin(const std::string &f);
    void prg(const std::string &f);
    void d64(const std::string &f);
    void crt(const std::string &f);
    void sid(const std::string &f);
    void process_args(int argc, char **argv);
    void handle_args();
    bool emulate();
    char *file;
    /* constants */
    static const uint16_t kBasicPrgStart = 0x0801;
    static const uint16_t kBasicTxtTab   = 0x002b;
    static const uint16_t kBasicVarTab   = 0x002d;
    static const uint16_t kBasicAryTab   = 0x002f;
    static const uint16_t kBasicStrEnd   = 0x0031;
};


#endif /* EMUDORE_LOADER_H */
