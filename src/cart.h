/*
 * Cartridge expansion port
 * Copyright (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * cart.h
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

#ifndef EMUDORE_C64CART_H
#define EMUDORE_C64CART_H


/* Pre declarations */
class C64;
class MC68B50;

class Cart
{
  private:
    uint16_t read_short_le(void);
    uint16_t read_short_be(void);
    void load_crt(const std::string &f);

    /* Main pointer */
    C64 * c64_;

    /* Cart uses mc6850 for midi yes or no? */
    bool midi = false;

  public:
    Cart(C64 * c64);
    ~Cart();

    void deinit_cart(void);
    void reset(void);
    bool emulate(void);

    void write_register(uint16_t addr, uint8_t v);
    uint8_t read_register(uint16_t addr);

    /* Emulated cart ic's */
    MC68B50 *mc6850_;

    /* MC68B50 ACIA */
    bool acia_active = false;

    /* Cart stuff */
    uint8_t banksetup; /* Used by PLA if havecart == true */
    bool cartactive = false;

    /* For reference only */
    enum crt_hardware_type {
      NORMAL_CARTRIDGE,    // 0
      ACTION_REPLAY,       // 1
      KCS_POWER_CARTRIDGE, // 2
      FINAL_CARTRIDGE_III, // 3
      SIMONS_BASIC,        // 4
      OCEAN_TYPE_1,        // 5
      EXPERT_CARTRIDGE,    // 6
      FUN_PLAY_POWER_PLAY, // 7
      SUPER_GAMES,         // 8
      ATOMIC_POWER,        // 9
      EPYX_FASTLOAD,       // 10
      WESTERMANN_LEARNING, // 11
      REX_UTILITY,         // 12
      FINAL_CARTRIDGE_I,   // 13
      MAGIC_FORMEL,        // 14
      C64_GAME_SYSTEM_3,   // 15
      WARPSPEED,           // 16
      DINAMIC,             // 17
      SUPER_ZAXXON_SEGA,   // 18
      MAGIC_DESK,          // 19
      SUPER_SNAPSHOT_5,    // 20
      COMAL_80,            // 21
      STRUCTURED_BASIC,    // 22
      ROSS,                // 23
      DELA_EP64,           // 24
      DELA_EP7X8,          // 25
      DELA_EP256,          // 26
      REX_EP256,           // 27
    };

    typedef struct crt_header_s {
      uint8_t signature_id_s[16] =
        {0x43,0x36,0x34, /* C64 */
         0x20, /* space */
         0x43,0x41,0x52,0x54,0x52,0x49,0x44,0x47,0x45, /* CARTRIDGE */
         0x20,0x20}; /* space space */
      uint8_t signature          = 0x00;
      uint8_t signature_l        = 0x0F;
      uint8_t headerlength       = 0x10;
      uint8_t headerlength_l     = 0x04;
      uint8_t version            = 0x14;
      uint8_t version_l          = 0x02;
      uint8_t hardware_type      = 0x16;
      uint8_t hardware_type_l    = 0x02;
      uint8_t exrom_line_status  = 0x18;
      uint8_t game_line_status   = 0x19;
      uint8_t unused             = 0x1a;
      uint8_t unused_l           = 0x05;
      uint8_t cartname           = 0x20;
      uint8_t cartname_l         = 0x1f;
      uint8_t cart_data          = 0x40;
    } crt_header_t;

    /* For reference only */
    enum crt_chip_type {
      ROM,      /* 0 */
      RAMnoROM, /* 1 */
      FlashROM, /* 2 */
    };

    typedef struct crt_chip_s {
      uint8_t signature_id_s[4] =
        {0x43,0x48,0x49,0x50}; /* CHIP */
      uint8_t signature         = 0x00;
      uint8_t signature_l       = 0x04;
      uint8_t packetlength      = 0x04;
      uint8_t packetlength_l    = 0x04;
      uint8_t chiptype          = 0x08;
      uint8_t chiptype_l        = 0x02;
      uint8_t banknumber        = 0x0a;
      uint8_t banknumber_l      = 0x02;
      uint8_t start_load_addr   = 0x0c;
      uint8_t start_load_addr_l = 0x02;
      uint8_t rom_img_size      = 0x0e;
      uint8_t rom_img_size_l    = 0x02;
      uint8_t rom_data          = 0x10;
    } crt_chip_t;

    typedef struct cart_chip_s {
      uint32_t signature     = 0;
      uint32_t packetlength  = 0;
      uint16_t chiptype      = 0;
      uint16_t banknumber    = 0;
      uint16_t addr          = 0;
      uint16_t romsize       = 0;
      uint8_t  *rom;
    } cart_chip_t;
    cart_chip_t * chips;

};


#endif /* EMUDORE_C64CART_H */
