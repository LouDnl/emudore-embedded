/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * memory.h
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

#ifndef EMUDORE_MEMORY_H
#define EMUDORE_MEMORY_H


#include <iostream>
#include <cstdint>


/**
 * @brief DRAM
 *
 * - @c $0000-$00FF  Page 0        Zeropage addressing
 * - @c $0100-$01FF  Page 1        Enhanced Zeropage contains the stack
 * - @c $0200-$02FF  Page 2        Operating System and BASIC pointers
 * - @c $0300-$03FF  Page 3        Operating System and BASIC pointers
 * - @c $0400-$07FF  Page 4-7      Screen Memory
 * - @c $0800-$9FFF  Page 8-159    Free BASIC program storage area (38911 bytes)
 * - @c $A000-$BFFF  Page 160-191  Free machine language program storage area (when switched-out with ROM)
 * - @c $C000-$CFFF  Page 192-207  Free machine language program storage area
 * - @c $D000-$D3FF  Page 208-211
 * - @c $D400-$D7FF  Page 212-215  SID address space
 * - @c $D800-$DBFF  Page 216-219
 * - @c $DC00-$DCFF  Page 220      CIA1 page
 * - @c $DD00-$DDFF  Page 221      CIA2 page
 * - @c $DE00-$DEFF  Page 222-223  I/O Area 1 Reserved for interface extensions e.g. SID2 or SID3
 * - @c $DF00-$DFFF  Page 222-223  I/O Area 2 Reserved for interface extensions e.g. SID2 or SID3
 * - @c $E000-$FFFF  Page 224-255  Free machine language program storage area (when switched-out with ROM)
 */
class Memory
{
  private:
    /* ROM & RAM */
    uint8_t *mem_ram_;
    uint8_t *mem_rom_;
    uint8_t *mem_rom_cia1_;
    uint8_t *mem_rom_cia2_;

    /* Main pointer */
    C64 * c64_;

    /* Debug logging */
    bool logmemrw  = false;
    bool logcia1rw = false;
    bool logcia2rw = false;
    bool logiorw   = false;
    bool logplarw  = false;

  public:
    Memory(C64 * c64);
    ~Memory();

    /* Memory pointer */
    uint8_t *mem_ram(void) {return mem_ram_;};

    /* read/write memory */
    uint8_t read_byte(uint16_t addr);
    uint8_t read_byte_no_io(uint16_t addr);
    void write_byte(uint16_t addr, uint8_t v);
    void write_byte_no_io(uint16_t addr, uint8_t v);
    uint16_t read_word(uint16_t addr);
    uint16_t read_word_no_io(uint16_t);
    void write_word(uint16_t addr, uint16_t v);
    void write_word_no_io(uint16_t addr, uint16_t v);

    /* vic memory access */
    uint8_t vic_read_byte(uint16_t addr);
    uint8_t read_byte_rom(uint16_t addr);

    /* load external binaries */
    void load_rom(const std::string &f, uint16_t baseaddr);
    void load_ram(const std::string &f, uint16_t baseaddr);

    /* debug */
    void dump();
    void dump(uint16_t start, uint16_t end);
    void print_screen_text();
    void setlogrw(char logid) { switch(logid)
      { case 0: logmemrw  = true; break;
        case 1: logcia1rw = true; break;
        case 2: logcia2rw = true; break;
        case 3: logiorw   = true; break;
        case 4: logplarw  = true; break;
        default: break; } };

    /* constants */
    static const size_t kMemSize = 0x10000;
    static const size_t kPageSize = 0x100;

    /* memory addresses  */
    static const uint16_t kBaseAddrBasic       = 0xa000; /* -> 0xbfff */
    static const uint16_t kBaseAddrKernal      = 0xe000; /* -> 0xffff */
    static const uint16_t kBaseAddrStack       = 0x0100;
    static const uint16_t kBaseAddrScreen      = 0x0400;
    static const uint16_t kBaseAddrChars       = 0xd000; /* -> 0xdfff */
    static const uint16_t kBaseAddrBitmap      = 0x0000;
    static const uint16_t kBaseAddrColorRAM    = 0xd800;
    static const uint16_t kAddrResetVector     = 0xfffc;
    static const uint16_t kAddrIRQVector       = 0xfffe;
    static const uint16_t kAddrNMIVector       = 0xfffa;
    static const uint16_t kAddrDataDirection   = 0x0000;
    static const uint16_t kAddrMemoryLayout    = 0x0001;
    static const uint16_t kAddrColorRAM        = 0xd800;

    /* RAM, cannot be changed */
    /* ZeroPage */
    static const uint16_t kAddrZeroPage        = 0x0000;

    /* RAM or Cartridge ROM */
    static const uint16_t kAddrCartLoFirstPage = 0x8000;
    static const uint16_t kAddrCartLoLastPage  = 0x9f00;
    static const uint16_t kAddrCartH1FirstPage = 0xa000;
    static const uint16_t kAddrCartH1LastPage  = 0xbf00;
    static const uint16_t kAddrCartH2FirstPage = 0xe000;
    static const uint16_t kAddrCartH2LastPage  = 0xff00;

    /* RAM, I/O registers and Color RAM */
    /* VIC */
    static const uint16_t kAddrVicFirstPage    = 0xd000;
    static const uint16_t kAddrVicLastPage     = 0xd300;
    /* SID */
    static const uint16_t kAddrSIDFirstPage    = 0xd400;
    static const uint16_t kAddrSIDLastPage     = 0xd700;
    /* Color RAM */
    static const uint16_t kAddrColorFirstPage  = 0xd800;
    static const uint16_t kAddrColorLastPage   = 0xdb00;
    /* CIA */
    static const uint16_t kAddrCIA1Page        = 0xdc00;
    static const uint16_t kAddrCIA2Page        = 0xdd00;
    /* IO */
    static const uint16_t kAddrIO1Page         = 0xde00;
    static const uint16_t kAddrIO2Page         = 0xdf00;

    /* ROM pages */
    /* BASIC */
    static const uint16_t kAddrBasicFirstPage  = 0xa000;
    static const uint16_t kAddrBasicLastPage   = 0xbf00;
    /* CHAR */
    static const uint16_t kAddrCharsFirstPage  = 0xd000;
    static const uint16_t kAddrCharsLastPage   = 0xdf00;
    /* KERNAL */
    static const uint16_t kAddrKernalFirstPage = 0xe000;
    static const uint16_t kAddrKernalLastPage  = 0xff00;

    /* Number of SID vs memory layout configuration */
    static const uint8_t kMaxSIDs = 2;
    uint8_t kSIDNum = 0; /* TODO: use USBSID-Pico SID amount */
    uint16_t kAddrSIDOne = 0xd400;
    uint16_t kAddrSIDTwo = 0xd420;

    /* Public memory pointers set by Memory class */
    /* CIA1 */
    uint8_t *kCIA1MemWr; /* $dc00 */
    uint8_t *kCIA1MemRd; /* $(dc)00 */
    /* CIA2 */
    uint8_t *kCIA2MemWr; /* $dd00 */
    uint8_t *kCIA2MemRd; /* $(dd)00 */

};


#endif /* EMUDORE_MEMORY_H */
