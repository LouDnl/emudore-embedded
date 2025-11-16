/*
 * MOS Programmable Logic Array (PLA)
 * Copyright (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * pla.h
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

#ifndef EMUDORE_C64PLA_H
#define EMUDORE_C64PLA_H

#if DESKTOP
#include <iostream>
#endif
#include <cstdint>
#if EMBEDDED
#include <stdlib.h>
#endif


/**
 * @brief Commodore 64 PLA
 *
 *
 *
 */
class PLA
{
  private:

    /* Main pointer */
    C64 *c64_;

    /* Cart yes or no? */
    bool havecart = false;

    /* Roms loaded? */
    bool l_basic   = false;
    bool l_chargen = false;
    bool l_kernal  = false;

    /* Memory banks */
    uint8_t data_direction_default = 0x2F; /* (https://www.c64-wiki.com/wiki/Zeropage) */
    uint8_t banks_at_boot = 0x1F;
    uint8_t banks_[7];

    /* Devices */
    uint8_t *disk;

    /* Debug logging */
    bool logplabank  = false;

    /* default bank setup is m31 0x1F */
    uint8_t default_bankmode;

  public:
    PLA(C64 * c64); /* Setup memory on init */
    PLA(C64 * c64, uint8_t banksetup); /* Setup memory with custom intializer on init */
    ~PLA();

    /** Bank Switching Zones
     *
     * @brief as described at https://www.c64-wiki.com/wiki/Bank_Switching#Bank_Switching_Zones
     * ID Hex Address Dec Address Page         Size Contents
     *  0 $0000-$0FFF 0-4095      Page 0-15       4 kBytes RAM (which the system requires and must appear in each mode)
     *  1 $1000-$7FFF 4096-32767  Page 16-127    28 kBytes RAM or is unmapped
     *  2 $8000-$9FFF 32768-40959 Page 128-159    8 kBytes RAM or cartridge ROM
     *  3 $A000-$BFFF 40960-49151 Page 160-191    8 kBytes RAM, BASIC interpretor ROM, cartridge ROM or is unmapped
     *  4 $C000-$CFFF 49152-53247 Page 192-207    4 kBytes RAM or is unmapped
     *  5 $D000-$DFFF 53248-57343 Page 208-223    4 kBytes RAM, Character generator ROM, or I/O registers and Color RAM
     *  6 $E000-$FFFF 57344-65535 Page 224-255    8 kBytes RAM, KERNAL ROM or cartridge ROM
     */
    enum Banks
    { kBankRam0    = 0, /* RAM, cannot be changed */
      kBankRam1    = 1, /* RAM or unmapped */
      kBankCart    = 2, /* RAM or cartridge ROM */
      kBankBasic   = 3, /* RAM, Basic ROM, cartridge ROM or unmapped */
      kBankRam2    = 4, /* RAM or unmapped */
      kBankChargen = 5, /* RAM, Character ROM, I/O registers and Color RAM */
      kBankKernal  = 6, /* RAM, Kernal ROM or cartridge ROM */
    };
    const char * BankModeNames[5] = {
      "kROM","kRAM","kIO","kCLO","kCHI"
    };

    /* Bank Switching Bit ID's */
    enum kBankCfg
    { /* CPU Control Lines */
      kROM, /* bit 0, weight  1 */
      kRAM, /* bit 1, weight  2 */
      kIO,  /* bit 2, weight  4 */
      /* Expansion Port Pins 8 and 9 */
      kCLO, /* bit 3, weight  8 */
      kCHI, /* bit 4, weight 16 */
      kUNM = -1, /* Unmapped */
    };

    /* Bank Switching Bits */
    static const uint8_t kLORAM   = 1 << kROM; /* 0 */
    static const uint8_t kHIRAM   = 1 << kRAM; /* 1 */
    static const uint8_t kCHARGEN = 1 << kIO;  /* 2 */
    static const uint8_t kGAME    = 1 << kCLO; /* 3 */
    static const uint8_t kEXROM   = 1 << kCHI; /* 4 */

    /** Bank Switching Modes
     * @brief as decribed at https://www.c64-wiki.com/wiki/Bank_Switching#Mode_Table
     */
    enum Modes
    { /* Visible = 1, Not visible = 0 */
      m31 = (kEXROM|kGAME|kCHARGEN|kHIRAM|kLORAM),
      m30 = (kEXROM|kGAME|kCHARGEN|kHIRAM),
      m29 = (kEXROM|kGAME|kCHARGEN|       kLORAM),
      m28 = (kEXROM|kGAME|kCHARGEN),
      m27 = (kEXROM|kGAME|         kHIRAM|kLORAM),
      m26 = (kEXROM|kGAME|         kHIRAM),
      m25 = (kEXROM|kGAME|                kLORAM),
      m24 = (kEXROM|kGAME),
      m23 = (kEXROM|      kCHARGEN|kHIRAM|kLORAM),
      m22 = (kEXROM|      kCHARGEN|kHIRAM),
      m21 = (kEXROM|      kCHARGEN|       kLORAM),
      m20 = (kEXROM|      kCHARGEN),
      m19 = (kEXROM|               kHIRAM|kLORAM),
      m18 = (kEXROM|               kHIRAM),
      m17 = (kEXROM|                      kLORAM),
      m16 = (kEXROM),
      m15 = (       kGAME|kCHARGEN|kHIRAM|kLORAM),
      m14 = (       kGAME|kCHARGEN|kHIRAM),
      m13 = (       kGAME|kCHARGEN|       kLORAM),
      m12 = (       kGAME|kCHARGEN),
      m11 = (       kGAME|         kHIRAM|kLORAM),
      m10 = (       kGAME|         kHIRAM),
      m09 = (       kGAME|                kLORAM),
      m08 = (       kGAME),
      m07 = (             kCHARGEN|kHIRAM|kLORAM),
      m06 = (             kCHARGEN|kHIRAM),
      m05 = (             kCHARGEN|       kLORAM),
      m04 = (             kCHARGEN),
      m03 = (                      kHIRAM|kLORAM),
      m02 = (                      kHIRAM),
      m01 = (                             kLORAM),
      m00 = 0,
    };

    /* Internal bank switching, public for Loader::bin */
    void switch_banks(uint8_t v);

    /* Memory banking / glue logic */
    uint8_t memory_banks(int v){return banks_[v];};
    void setup_memory_banks(uint8_t v);
    void runtime_bank_switching(uint8_t v);

    /* read/write disk images */
    static const size_t disksize = 0x2AB00; // max 174848 bytes with 683 sectors of 256 bytes
    uint8_t *DISKptr;
    int disk_size;
    void c1541(void); /* Not working */

    /* Read/Write */
    void write_pla(uint16_t addr, uint8_t v);
    uint8_t read_pla(uint16_t addr);

    void reset(void);
    void emulate(void);

    /* Debug logging */
    void setbanklogging(bool v){logplabank = v;};
    void logbanksetup(void);
};


#endif /* EMUDORE_C64PLA_H */
