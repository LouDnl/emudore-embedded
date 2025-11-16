/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
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

#include <bitset>
#include <iomanip>
#include <string.h>

#include <loader.h>
#include <sidfile.h>

#include "reloc65.h"

const char *chiptype_s[4] = {"Unknown", "MOS6581", "MOS8580", "MOS6581 and MOS8580"};
const char *clockspeed_s[5] = {"Unknown", "PAL", "NTSC", "PAL and NTSC", "DREAN"};

/**
 * @brief Initialize Loader object
 */
Loader::Loader()
{
  booted_up_ = false;
  format_ = kNone;

  filename = NULL;

  sidfile_ = NULL;
  pl_refresh_rate = 0;
  pl_frame_cycles = 0;
  pl_rasterrow_cycles = 0;
  pl_raster_lines = 0;
  pl_clock_speed = 0;
  pl_sidversion = 0;
  pl_clockspeed = 0;
  pl_chiptype = 0;
  pl_curr_sidspeed = 0;
  pl_sidflags = 0;
  pl_song_number = 0;
  pl_songs = 0;
  pl_sidspeed = 0;
  pl_databuffer = 0;
  pl_datalength = 0;
  pl_playaddr = 0;
  pl_loadaddr = 0;
  pl_initaddr = 0;

  subtune = -1;
}

/**
 * @brief called to create pointer object to Machine */
void Loader::C64ctr(C64 *c64)
{
  c64_ = c64;
}

// common ///////////////////////////////////////////////////////////////////

uint16_t Loader::read_short_le()
{
  char b;
  uint16_t v = 0;
  is_.get(b);
  v |= (b);
  is_.get(b);
  v |= b << 8;
  return v;
}

// BASIC listings ///////////////////////////////////////////////////////////

void Loader::bas(const std::string &f)
{
  format_ = kBasic;
  is_.open(f,std::ios::in);
}

void Loader::load_basic()
{
  char c;
  if(is_.is_open())
  {
    while(is_.good())
    {
      is_.get(c);
      c64_->io_->IO::type_character(c);
    }
  }
}

// BIN //////////////////////////////////////////////////////////////////////

void Loader::bin(const std::string &f)
{
  format_ = kBIN;
  is_.open(f,std::ios::in|std::ios::binary);
}

void Loader::load_bin()
{
  char b;
  uint16_t bbuf, addr;
  bbuf = addr = 0x8000; /* Fix binary address at $400 */
  if (init_addr != 0) {addr = init_addr;};
  // if (bank_setup != 0) {}; // TODO: Add custom bank setup
  /* Remap C64 ROMs */
  // TODO: Since we do not load a rom but a binary into RAM we cannot change the banks
  if (iscart) {c64_->pla_->switch_banks(PLA::m15);};

  if (iscart) {
    c64_->mem_->write_byte(0xD020,0); /* Bordor color to black */
    c64_->mem_->write_byte(0xD021,0); /* Background color to black */
  }

  if(!iscart && is_.is_open()) {
    while(is_.good()) /* BUG: THIS WORKS */
    {
      is_.get(b);
      c64_->mem_->write_byte(bbuf++,b);
    }
    /* BUG: This does not!? */
    // is_.seekg (0, is_.end);
    // std::streamoff length = is_.tellg();
    // is_.seekg (0, is_.beg);
    // is_.read ((char *) &mem_->mem_ram()[addr],length);
    c64_->cpu_->pc(addr);
  }
}

// PRG //////////////////////////////////////////////////////////////////////

void Loader::prg(const std::string &f)
{
  format_ = kPRG;
  is_.open(f,std::ios::in|std::ios::binary);
}

void Loader::load_prg()
{
  char b;
  uint16_t pbuf;
  pbuf = load_addr = read_short_le();
  if(is_.is_open())
  {
    while(is_.good())
    {
      is_.get(b);
      c64_->mem_->write_byte(pbuf++,b);
    }

    /* basic-tokenized prg */
    if(load_addr == kBasicPrgStart)
    {
      /* make BASIC happy */
      c64_->mem_->write_word(kBasicTxtTab,load_addr);
      c64_->mem_->write_word(kBasicVarTab,pbuf);
      c64_->mem_->write_word(kBasicAryTab,pbuf);
      c64_->mem_->write_word(kBasicStrEnd,pbuf);
      if(autorun) {
        if (basic_run) {
          /* type and exec RUN */
          for(char &c: std::string("RUN\n")) {
            c64_->io_->IO::type_character(c);
          }
        } else if (init_addr == 0x0) { /* BUG: Not always correct */
          init_addr = ((c64_->mem_->read_byte(load_addr)|c64_->mem_->read_byte(load_addr+0x1)<<8)+0x2);
          D("load_addr %04X start_addr %04X, calculated init_addr %04X\n", load_addr, start_addr, init_addr);
          c64_->cpu_->pc(init_addr);
        } else {
          D("load_addr %04X start_addr %04X, provided init_addr %04X\n", load_addr, start_addr, init_addr);
          c64_->cpu_->pc(init_addr); /* Example: Cynthcart start/init address is 0x80D */
        }
      }
    }
    else {
      init_addr = ((c64_->mem_->read_byte(load_addr)|c64_->mem_->read_byte(load_addr+0x1)<<8)+0x2);
      D("load_addr %04X start_addr %04X, calculated init_addr %04X\n", load_addr, start_addr, init_addr);
      c64_->cpu_->pc(init_addr);
    }

  }
}

// D64 //////////////////////////////////////////////////////////////////////

void Loader::d64(const std::string &f)
{
  format_ = kD64;
  is_.open(f,std::ios::in|std::ios::binary);
}

void Loader::load_d64()
{
  D("Disk loading not implemented yet!\n");
  return;
  char b;
  int n = 0;
  //uint16_t pbuf, addr;
  //pbuf = addr = read_short_le();
  if(is_.is_open()) {
    while (is_.good()) {
      is_.get(b);
      c64_->pla_->DISKptr[n++] = b;
    }
    c64_->pla_->disk_size = n;
    c64_->io_->set_disk_loaded(true);
  }
  //  {
  //    while(is_.good())
  //    {
  //      is_.get(b);
  //      mem_->write_byte(pbuf++,b);
  //    }
  //
  //    /* basic-tokenized prg */
  //    if(addr == kBasicPrgStart)
  //    {
  //      /* make BASIC happy */
  //      mem_->write_word(kBasicTxtTab,addr);
  //      mem_->write_word(kBasicVarTab,pbuf);
  //      mem_->write_word(kBasicAryTab,pbuf);
  //      mem_->write_word(kBasicStrEnd,pbuf);
  //      if(autorun) {
  //        /* exec RUN */
  //        for(char &c: std::string("RUN\n"))
  //          io_->IO::type_character(c);
  //      }
  //    }
  //    /* ML */
  //    else
  //      cpu_->pc(addr); /* Else always start, or prg start is broken */
  //  }
}

// CRT //////////////////////////////////////////////////////////////////////

bool Loader::crt()
{
  format_ = kCRT;
  return true;
}

// SID //////////////////////////////////////////////////////////////////////

void Loader::sid(const std::string &f)
{
  format_ = kSID;
  sidfile_ = new SidFile();
  sidfile_->Parse(f);
  pre_load_sid();
}

void Loader::pre_load_sid()
{
  pl_songs = sidfile_->GetNumOfSongs();
  pl_song_number = sidfile_->GetFirstSong(); /* TODO: FIX N FINISH ;) */
  pl_sidflags = sidfile_->GetSidFlags();
  pl_sidspeed = sidfile_->GetSongSpeed(pl_song_number); // + 1);
  pl_curr_sidspeed = (pl_sidspeed & (1 << pl_song_number)); // ? 1 : 0;  // 1 ~ 60Hz, 2 ~ 50Hz
  pl_chiptype = sidfile_->GetChipType(1);
  pl_clockspeed = sidfile_->GetClockSpeed();
  pl_sidversion = sidfile_->GetSidVersion();
  pl_clock_speed = clockSpeed[pl_clockspeed];
  pl_raster_lines = scanLines[pl_clockspeed];
  pl_rasterrow_cycles = scanlinesCycles[pl_clockspeed];
  pl_frame_cycles = (pl_raster_lines * pl_rasterrow_cycles);
  pl_refresh_rate = refreshRate[pl_clockspeed];
  pl_loadaddr = sidfile_->GetLoadAddress();
  pl_lastloadaddr = ((sidfile_->GetLoadAddress() - 1) + sidfile_->GetDataLength());
  pl_datalength = sidfile_->GetDataLength();
  pl_databuffer = sidfile_->GetDataPtr();
  pl_playaddr = sidfile_->GetPlayAddress();
  pl_initaddr = sidfile_->GetInitAddress();
  pl_dataoffset = sidfile_->GetDataOffset();
  pl_start_page = sidfile_->GetStartPage();
  pl_max_pages = sidfile_->GetMaxPages();
  pl_isrsid = (sidfile_->GetSidType() == "RSID");
  C64::is_rsid = pl_isrsid;
  printf("RSID? %d %d\n",pl_isrsid, C64::is_rsid);
}

uint_least8_t Loader::findDriverSpace(const bool* pages, uint_least8_t scr,
                        uint_least8_t chars, uint_least8_t size)
{
  uint_least8_t firstPage = 0;
  for (unsigned int i = 0; i < MAX_PAGES; ++i)
  {
    if (pages[i] || (scr && (scr <= i) && (i < (scr + NUM_SCREEN_PAGES)))
      || (chars && (chars <= i) && (i < (chars + NUM_CHAR_PAGES))))
    {
      if ((i - firstPage) >= size)
      {
        return firstPage;
      }
      firstPage = i + 1;
    }
  }
  return 0;
}

void Loader::find_free_page()
{
  /* Start and end pages. */

  bool pages[256];
  unsigned int startp = pl_start_page;
  unsigned int maxp = pl_max_pages;
  unsigned int i;
  unsigned int j;
  unsigned int k;
  uint_least8_t scr;
  uint_least8_t chars;
  uint_least8_t driver;
  m_screenPage = (uint_least8_t) (0x00);
  m_driverPage = (uint_least8_t) (0x00);
  m_charPage = (uint_least8_t) (0x00);
  m_stilPage = (uint_least8_t) (0x00);
  m_songlengthsPage = (uint_least8_t) (0x00);

  if (pl_start_page == 0x00) {
    fprintf(stdout, "No PSID freepages set, recalculating\n");
  } else {
    fprintf(stdout, "Calculating first free page\n");
  }

  if (startp == 0x00) {

    /* Used memory ranges. */
    unsigned int used[] = {
      0x00, 0x03,
      0xa0, 0xbf,
      0xd0, 0xff,
      0x00, 0x00
    }; /* calculated below */

    /* finish initialization */
    used[6] = (pl_loadaddr >> 8);
    used[7] = (pl_lastloadaddr >> 8);

    /* Mark used pages in table. */
    for (i = 0; i < MAX_PAGES; ++i) {
      pages[i] = false;
    }
    for (i = 0; i < sizeof(used) / sizeof(*used); i += 2) {
      for (j = used[i]; j <= used[i + 1]; ++j) {
        pages[j] = true;
      }
    }
  } else if ((startp != 0xff) && (maxp != 0)) {
    /* the available pages have been specified in the PSID file */
    unsigned int endp = min((startp + maxp), MAX_PAGES);

    /* check that the relocation information does not use the following */
    /* memory areas: 0x0000-0x03FF, 0xA000-0xBFFF and 0xD000-0xFFFF */
    if ((startp < 0x04)
      || ((0xa0 <= startp) && (startp <= 0xbf))
      || (startp >= 0xd0)
      || ((endp - 1) < 0x04)
      || ((0xa0 <= (endp - 1)) && ((endp - 1) <= 0xbf))
      || ((endp - 1) >= 0xd0))
    {
      fprintf(stderr, "No free pages!\n");
      exit(1);
    }

    for (i = 0; i < MAX_PAGES; ++i) {
      pages[i] = ((startp <= i) && (i < endp)) ? false : true;
    }
  }
  driver = 0;
  for (i = 0; i < 4; ++i) {
    /* Calculate the VIC bank offset. Screens located inside banks 1 and 3
    require a copy the character rom in ram. The code below uses a
    little trick to swap bank 1 and 2 so that bank 0 and 2 are checked
    before 1 and 3. */
    uint_least8_t bank = (((i & 1) ^ (i >> 1)) ? i ^ 3 : i) << 6;

    for (j = 0; j < 0x40; j += 4) {
        /* screen may not reside within the char rom mirror areas */
        if (!(bank & 0x40) && (0x10 <= j) && (j < 0x20))
            continue;

        // check if screen area is available
        scr = bank + j;
        if (pages[scr] || pages[scr + 1] || pages[scr + 2]
          || pages[scr + 3]) {
            continue;
        }

        if (bank & 0x40) {
          /* The char rom needs to be copied to RAM so let's try to find
          a suitable location. */
          for (k = 0; k < 0x40; k += 8) {
            /* char rom area may not overlap with screen area */
            if (k == (j & 0x38)) {
              continue;
            }

            // check if character rom area is available
            chars = bank + k;
            if (pages[chars] || pages[chars + 1]
              || pages[chars + 2] || pages[chars + 3]) {
              continue;
            }

            driver = findDriverSpace (pages, scr, chars, NUM_EXTDRV_PAGES);
            if (driver)
            {
              m_driverPage = driver;
              m_screenPage = scr;
              m_charPage = chars;
              break;
            }
          }
        } else {
          driver = findDriverSpace(pages, scr, 0, NUM_EXTDRV_PAGES);
          if (driver)
          {
            m_driverPage = driver;
            m_screenPage = scr;
            break;
          }
      }
    }
  }

  if (!driver)
  {
    driver = findDriverSpace(pages, 0, 0, NUM_MINDRV_PAGES);
    m_driverPage = driver;
  }
  printf("m_driverPage %02X driver %02x\n", m_driverPage, driver);
}

void Loader::load_sid()
{
  print_sid_info();
  c64_->mem_->write_byte(0xD020,2); /* Bordor color to red */
  c64_->mem_->write_byte(0xD021,0); /* Background color to black */
  printf("load: $%04X play: $%04X init: $%04X length: $%X\n", pl_loadaddr, pl_playaddr, pl_initaddr, pl_datalength);

  psid_init_driver();
  // else if (!pl_isrsid) {
  //   /* Load SID data into memory */
  //   for (unsigned int i = 0; i < pl_datalength; i++) {
  //     c64_->mem_->write_byte((pl_loadaddr+i),pl_databuffer[i]);
  //     if (i == pl_datalength-1) printf("end: $%04X\n",(pl_loadaddr+i));
  //   }
  //   load_Psidplayer(pl_playaddr, pl_initaddr, pl_loadaddr, pl_datalength, pl_song_number);
  // } else {
  //   psid_init_driver();
  // }

  c64_->sid_->sid_flush();
  c64_->sid_->set_playing(true);
  // if (!pl_isrsid) c64_->cpu_->pc(c64_->mem_->read_word(Memory::kAddrResetVector));
  // if (!pl_isrsid) c64_->cpu_->irq();
}

void Loader::load_Psidplayer(uint16_t play, uint16_t init, uint16_t load, uint16_t length, int songno)
{
  D("Starting PSID player\n");
  find_free_page();
  playerstart = (m_driverPage<<8);
  // /* uint16_t */ playerstart = (load <= 0x1000) ? (init+length+1) : (load-28);
  // uint16_t playerstart = (load-28);
  /* uint8_t */ p_hi = ((playerstart >> 8) & 0xFF);
  /* uint8_t */ p_lo = (playerstart & 0xFF);
  printf("playerstart: $%04X p_lo: $%02X p_hi: $%02X\n", playerstart, p_lo, p_hi);
  printf("playerend  : $%04X\n", (playerstart+0x1A));

  /* install reset vector for microplayer */
  c64_->mem_->write_byte(0xFFFC, p_lo); // lo
  c64_->mem_->write_byte(0xFFFD, p_hi); // hi

  /* install IRQ vector for play routine launcher */
  c64_->mem_->write_byte(0xFFFE, (p_lo+0x13)); // lo
  c64_->mem_->write_byte(0xFFFF, p_hi); // hi

  /* clear everything from from ram except IO 'm13' */
  c64_->pla_->switch_banks(c64_->pla_->m13);  /* WORKS */
  // c64_->mem_->write_byte(0x0001, c64_->pla_->m13); /* 13 */  /* ISSUE: BREAKS PLAY, DOESNT EVEN START TO PLAY! */

  /* install the micro player, 6502 assembly code */
  c64_->mem_->write_byte(playerstart, 0xA9);                     // 0xA9 LDA imm load A with the song number
  c64_->mem_->write_byte(playerstart+1, songno);                 // 0xNN #NN song number

  /* jump to init address */
  /* ISSUE: 0x4C WORKS BUT CREATES TURBO PLAY */
  // c64_->mem_->write_byte(playerstart+2, 0x4C);                   // 0x20 JSR abs jump sub to INIT routine
  /* WORKS, ONLY IF C64 IS FULLY BOOTED */
  c64_->mem_->write_byte(playerstart+2, 0x20);                   // 0x20 JSR abs jump sub to INIT routine
  c64_->mem_->write_byte(playerstart+3, (init & 0xFF));          // 0xxxNN address lo
  c64_->mem_->write_byte(playerstart+4, (init >> 8) & 0xFF);     // 0xNNxx address hi

  c64_->mem_->write_byte(playerstart+5, 0x58);                   // 0x58 CLI enable interrupt
  c64_->mem_->write_byte(playerstart+6, 0xEA);                   // 0xEA NOP impl
  c64_->mem_->write_byte(playerstart+7, 0x4C);                   // JMP jump to 0x0006
  c64_->mem_->write_byte(playerstart+8, (p_lo+6));               // 0xxxNN address lo
  c64_->mem_->write_byte(playerstart+9, p_hi);                   // 0xNNxx address hi

  /* play routine launcher */
  c64_->mem_->write_byte(playerstart+0x13, 0xEA);                // 0xEA NOP
  c64_->mem_->write_byte(playerstart+0x14, 0xEA);                // 0xEA NOP
  c64_->mem_->write_byte(playerstart+0x15, 0xEA);                // 0xEA NOP
  c64_->mem_->write_byte(playerstart+0x16, 0x20);                // 0x20 JSR jump sub to play routine
  c64_->mem_->write_byte(playerstart+0x17, (play & 0xFF));       // playaddress lo
  c64_->mem_->write_byte(playerstart+0x18, (play >> 8) & 0xFF);  // playaddress hi
  c64_->mem_->write_byte(playerstart+0x19, 0xEA);                // 0xEA NOP
  c64_->mem_->write_byte(playerstart+0x1A, 0x40);                // 0x40 RTI return from interrupt

  // for (uint16_t i = playerstart ; i <= playerstart+0x1A ; i++) {
  //   printf("addr: $%04X data: $%02X\n",
  //     i, c64_->mem_->read_byte_no_io(i));
  // }
}

//-------------------------------------------------------------------------
// Temporary hack till real bank switching code added

//  Input: A 16-bit effective address
// Output: A default bank-select value for $01.
uint8_t Loader::iomap(uint_least16_t addr)
{
  // Force Real C64 Compatibility
  if (pl_isrsid)
    return 0;     // Special case, converted to 0x37 later

  if (addr == 0)
    return 0;     // Special case, converted to 0x37 later
  if (addr < 0xa000)
    return 0x37;  // Basic-ROM, Kernal-ROM, I/O
  if (addr  < 0xd000)
    return 0x36;  // Kernal-ROM, I/O
  if (addr >= 0xe000)
    return 0x35;  // I/O only
  return 0x34;  // RAM only
}

void Loader::psid_init_driver(void) /* RSID ONLY */
{
  int start_song = (subtune < 0 ? (pl_song_number+1) : (subtune+1));
  // PSID_LOAD_FILE
  {
    if (pl_sidversion >= 2) {
      pl_sidflags = pl_sidflags;
      pl_start_page = sidfile_->GetStartPage();
      pl_max_pages = sidfile_->GetMaxPages();
    } else {
      pl_sidflags = 0;
      pl_start_page = 0;
      pl_max_pages = 0;
    }
    if ((start_song < 0) || (start_song > pl_songs)) {
      fprintf(stdout, "Default tune out of range (%d of %d ?), using 1 instead.\n", start_song, pl_songs);
      start_song = 1;
    } else printf("start_song: %d\n",start_song);

    /* Relocation setup. */
    find_free_page();

    if (m_driverPage == 0x00) { //|| pl_max_pages < 2) {
      fprintf(stdout, "No space for driver.\n");
      exit(1);
    }
  }

  // PSID_INIT_DRIVER
  {
    uint8_t psid_driver[] = {
#include "psiddrv.h"
    };
    static const uint_least8_t psid_extdriver[] = {
#include "psidextdrv.h"
    };
    // char *psid_reloc = (char *)psid_driver;
    const uint_least8_t* driver;
    uint_least8_t* psid_reloc;
    int psid_size;
    uint_least16_t reloc_addr;
    uint_least16_t addr;
    int i;

    /* select driver */
    printf("m_screenPage: %02X\n",m_screenPage);
    if (m_screenPage == 0x00)
    {
      printf("using psid_driver\n");
      psid_size = sizeof(psid_driver);
      driver = psid_driver;
    }
    else /* Uses SCREEN, NOT NEEDED */
    {
      printf("using psid_extdriver\n");
      psid_size = sizeof(psid_extdriver);
      driver = psid_extdriver;
    }

    // Relocation of C64 PSID driver code.
    psid_reloc = new uint_least8_t[psid_size];
    if (psid_reloc == NULL)
    {
      return;
    }
    memcpy(psid_reloc, driver, psid_size);
    reloc_addr = m_driverPage << 8;

    fprintf(stdout, "PSID free pages: $%04x-$%04x\n",
            reloc_addr, (reloc_addr + (pl_max_pages << 8)) - 1U);
    globals_t globals;
    globals["playnum"] = (start_song - 1) & 0xff;
    uint_least16_t jmpAddr = m_driverPage << 8;
    // start address of driver
    globals["player"] = jmpAddr;
    // address of new stop vector for tunes that call $a7ae during init
    globals["stopvec"] = jmpAddr+3;
    const uint_least16_t load_addr = 0x0801;
    int screen = m_screenPage << 8;
    globals["screen"] = screen;
    // globals["barsprptr"] = ((screen + BAR_SPRITE_SCREEN_OFFSET) & 0x3fc0) >> 6;
    globals["dd00"] = ((((m_screenPage & 0xc0) >> 6) ^ 3) | 0x04);
    uint_least8_t vsa;  // video screen address
    uint_least8_t cba;  // character memory base address
    vsa = (uint_least8_t) ((m_screenPage & 0x3c) << 2);
    cba = (uint_least8_t) (m_charPage ? (m_charPage >> 2) & 0x0e : 0x06);
    globals["d018"] = vsa | cba;
    globals["sid2base"] = 0xd420; //sid2base;
    globals["sid3base"] = 0xd440; //sid3base;
    if (!reloc65(reinterpret_cast<char **>(&psid_reloc), &psid_size, reloc_addr, &globals)) {
        fprintf(stderr, "Relocation.\n");
        return;
    }

    // Skip JMP table
    addr = 6;

    // Store parameters for PSID player.
    psid_reloc[addr++] = (uint_least8_t) (pl_initaddr ? 0x4c : 0x60);
    psid_reloc[addr++] = (uint_least8_t) (pl_initaddr & 0xff);
    psid_reloc[addr++] = (uint_least8_t) (pl_initaddr >> 8);
    psid_reloc[addr++] = (uint_least8_t) (pl_playaddr ? 0x4c : 0x60);
    psid_reloc[addr++] = (uint_least8_t) (pl_playaddr & 0xff);
    psid_reloc[addr++] = (uint_least8_t) (pl_playaddr >> 8);
    psid_reloc[addr++] = (uint_least8_t) (pl_songs);

    // get the speed bits (the driver only has space for the first 32 songs)
    uint_least32_t speed = 0;
    unsigned int songs = min(pl_songs, 32);
    for (unsigned int i = 0; i < songs; ++i)
    {
      if ((pl_sidspeed&i) != 0) /* SIDTUNE_SPEED_VBI VerticalBlankInterrupt == 0 */
      {
        speed |= (1 << i);
      }
    }
    psid_reloc[addr++] = (uint_least8_t) (speed & 0xff);
    psid_reloc[addr++] = (uint_least8_t) ((speed >> 8) & 0xff);
    psid_reloc[addr++] = (uint_least8_t) ((speed >> 16) & 0xff);
    psid_reloc[addr++] = (uint_least8_t) (speed >> 24);

    psid_reloc[addr++] = (uint_least8_t) ((pl_loadaddr < 0x31a) ? 0xff : 0x05);
    psid_reloc[addr++] = iomap (pl_initaddr);
    psid_reloc[addr++] = iomap (pl_playaddr);
    psid_reloc[addr++] = ((start_song - 1) & 0xff); /* DOESN'T WORK */
    psid_reloc[addr++] = 1; /* PAL */

    printf("ADDR: $%04X D:$%02X\n",reloc_addr+addr,psid_reloc[addr]);

    /* Write driver to memory */
    for (i = 0; i < psid_size; i++) {
      c64_->mem_->write_byte((uint16_t)(reloc_addr + i), psid_reloc[i]);
    }

    /* Load SID data into memory */
    for (unsigned int i = 0; i < pl_datalength; i++) {
      c64_->mem_->write_byte((pl_loadaddr+i),pl_databuffer[i]);
    }

    fprintf(stdout, "Driver=$%04X, Image=$%04X-$%04X, Init=$%04X, Play=$%04X\n",
      m_driverPage<<8, pl_loadaddr,
      (unsigned int)(pl_loadaddr + pl_datalength - 1),
      pl_initaddr, pl_playaddr);

    if (this->init_addr != 0x0) {
      printf("Set program counter to $%04X (init argument)\n",this->init_addr);
      c64_->cpu_->pc(this->init_addr);
    } else {
      printf("Set program counter to $%04X\n",reloc_addr);
      c64_->cpu_->pc(reloc_addr);
    }
  }
}

void Loader::print_sid_info() /* TODO: THIS MUST GO TO C64 SCREEN */
{
  cout << "\n< Sid Info >" << endl;
  cout << "---------------------------------------------" << endl;
  cout << "SID Title          : " << sidfile_->GetModuleName() << endl;
  cout << "Author Name        : " << sidfile_->GetAuthorName() << endl;
  cout << "Release & (C)      : " << sidfile_->GetCopyrightInfo() << endl;
  cout << "---------------------------------------------" << endl;
  cout << "SID Type           : " << sidfile_->GetSidType() << endl;
  cout << "SID Format version : " << dec << pl_sidversion << endl;
  cout << "---------------------------------------------" << endl;
  // cout << "SID Flags          : 0x" << hex << sidflags << " 0b" << bitset<8>{sidflags} << endl;
  cout << "Chip Type          : " << chiptype_s[pl_chiptype] << endl;
  if (pl_sidversion == 3 || pl_sidversion == 4)
      cout << "Chip Type 2        : " << chiptype_s[sidfile_->GetChipType(2)] << endl;
  if (pl_sidversion == 4)
      cout << "Chip Type 3        : " << chiptype_s[sidfile_->GetChipType(3)] << endl;
  cout << "Clock Type         : " << hex << clockspeed_s[pl_clockspeed] << endl;
  cout << "Clock Speed        : " << dec << pl_clock_speed << endl;
  cout << "Raster Lines       : " << dec << pl_raster_lines << endl;
  cout << "Rasterrow Cycles   : " << dec << pl_rasterrow_cycles << endl;
  cout << "Frame Cycles       : " << dec << pl_frame_cycles << endl;
  cout << "Refresh Rate       : " << dec << pl_refresh_rate << endl;
  if (pl_sidversion == 3 || pl_sidversion == 4 || pl_sidversion == 78) {
      cout << "---------------------------------------------" << endl;
      cout << "SID 2 $addr        : $d" << hex << sidfile_->GetSIDaddr(2) << "0" << endl;
      if (pl_sidversion == 4 || pl_sidversion == 78)
          cout << "SID 3 $addr        : $d" << hex << sidfile_->GetSIDaddr(3) << "0" << endl;
      if (pl_sidversion == 78)
          cout << "SID 4 $addr        : $d" << hex << sidfile_->GetSIDaddr(4) << "0" << endl;
  }
  cout << "---------------------------------------------" << endl;
  cout << "Data Offset        : $" << setfill('0') << setw(4) << hex << sidfile_->GetDataOffset() << endl;
  cout << "Image length       : $" << hex << sidfile_->GetInitAddress() << " - $" << hex << pl_lastloadaddr << endl;
  cout << "Load Address       : $" << hex << sidfile_->GetLoadAddress() << endl;
  cout << "Init Address       : $" << hex << sidfile_->GetInitAddress() << endl;
  cout << "Play Address       : $" << hex << sidfile_->GetPlayAddress() << endl;
  cout << "Start Page         : $" << hex << sidfile_->GetStartPage() << endl;
  cout << "Max Pages          : $" << hex << sidfile_->GetMaxPages() << endl;
  cout << "---------------------------------------------" << endl;
  cout << "Song Speed(s)      : $" << hex << pl_curr_sidspeed << " $0x" << hex << pl_sidspeed << " 0b" << bitset<32>{pl_sidspeed} << endl;
  cout << "Timer              : " << (pl_curr_sidspeed == 1 ? "CIA1" : "Clock") << endl;
  cout << "Selected Sub-Song  : " << dec << pl_song_number + 1 << " / " << dec << sidfile_->GetNumOfSongs() << endl;
}


// emulate //////////////////////////////////////////////////////////////////

void Loader::handle_args()
{
  if (lowercase) {
    c64_->mem_->write_byte(0xD018,0x17); /* Enable lowercase mode */
  }
  if (memrwlog) {
    c64_->mem_->setlogrw(0);
  }
  if (cia1rwlog) {
    c64_->mem_->setlogrw(1);
  }
  if (cia2rwlog) {
    c64_->mem_->setlogrw(2);
  }
  if (iorwlog) {
    c64_->mem_->setlogrw(3);
  }
  if (plarwlog) {
    c64_->mem_->setlogrw(4);
  }
  if (cartrwlog) {
    c64_->mem_->setlogrw(5);
  }
  if (sidrwlog) {
    c64_->mem_->setlogrw(6);
  }
  /* 7 is sid something */
  if (vicrwlog) {
    c64_->mem_->setlogrw(8);
  }
}

bool Loader::handle_file()
{
  switch(format_)
  {
  case kBasic:
    load_basic();
    break;
  case kBIN:
    load_bin();
    break;
  case kPRG:
    load_prg();
    break;
  case kD64:
    load_d64();
    break;
  case kSID:
    load_sid();
    break;
  case kCRT: /* Skip, is done in Cart */
  default:
    break;
  }
  return false;
}

bool Loader::emulate()
{
  handle_args();
  handle_file();
  if (instrlog) Cpu::loginstructions = true;
  // if (instrlog) c64_->mem_->setlogrw(1);
  return false;
}
