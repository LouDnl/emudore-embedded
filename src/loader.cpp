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

  subtune = 0;
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
      c64_->mem_->write_byte_no_io(bbuf++,b);
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
      c64_->mem_->write_byte_no_io(pbuf++,b);
    }

    /* basic-tokenized prg */
    if(load_addr == kBasicPrgStart)
    {
      /* make BASIC happy */
      c64_->mem_->write_word_no_io(kBasicTxtTab,load_addr);
      c64_->mem_->write_word_no_io(kBasicVarTab,pbuf);
      c64_->mem_->write_word_no_io(kBasicAryTab,pbuf);
      c64_->mem_->write_word_no_io(kBasicStrEnd,pbuf);
      if(autorun) {
        if (basic_run) {
          /* type and exec RUN */
          for(char &c: std::string("RUN\n")) {
            c64_->io_->IO::type_character(c);
          }
        } else if (init_addr == 0x0) { /* BUG: Not always correct */
          init_addr = ((c64_->mem_->read_byte_no_io(load_addr)|c64_->mem_->read_byte_no_io(load_addr+0x1)<<8)+0x2);
          D("load_addr %04X start_addr %04X, calculated init_addr %04X\n", load_addr, start_addr, init_addr);
          c64_->cpu_->pc(init_addr);
        } else {
          D("load_addr %04X start_addr %04X, provided init_addr %04X\n", load_addr, start_addr, init_addr);
          c64_->cpu_->pc(init_addr); /* Example: Cynthcart start/init address is 0x80D */
        }
      }
    }
    else {
      init_addr = ((c64_->mem_->read_byte_no_io(load_addr)|c64_->mem_->read_byte_no_io(load_addr+0x1)<<8)+0x2);
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
  //      mem_->write_byte_no_io(pbuf++,b);
  //    }
  //
  //    /* basic-tokenized prg */
  //    if(addr == kBasicPrgStart)
  //    {
  //      /* make BASIC happy */
  //      mem_->write_word_no_io(kBasicTxtTab,addr);
  //      mem_->write_word_no_io(kBasicVarTab,pbuf);
  //      mem_->write_word_no_io(kBasicAryTab,pbuf);
  //      mem_->write_word_no_io(kBasicStrEnd,pbuf);
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
  pl_song_number = subtune;//sidfile_->GetFirstSong(); /* TODO: FIX N FINISH ;) */
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

uint16_t Loader::find_free_page()
{
  /* Start and end pages. */
  int startp = (pl_loadaddr >> 8);
  int endp = (pl_lastloadaddr >> 8);

  /* Used memory ranges. */
  unsigned int used[] = {
      0x00,
      0x03,
      0xa0,
      0xbf,
      0xd0,
      0xff,
      0x00,
      0x00
  }; /* calculated below */

  unsigned int pages[256];
  unsigned int last_page = 0;
  unsigned int i, page, tmp;

  if (pl_start_page == 0x00) {
    fprintf(stdout, "No PSID freepages set, recalculating\n");
  } else {
    fprintf(stdout, "Calculating first free page\n");
  }

  /* finish initialization */
  used[6] = startp; used[7] = endp;

  /* Mark used pages in table. */
  memset(pages, 0, sizeof(pages));
  for (i = 0; i < sizeof(used) / sizeof(*used); i += 2) {
      for (page = used[i]; page <= used[i + 1]; page++) {
          pages[page] = 1;
      }
  }

  /* Find largest free range. */
  pl_max_pages = 0x00;
  for (page = 0; page < sizeof(pages) / sizeof(*pages); page++) {
      if (!pages[page]) {
          continue;
      }
      tmp = page - last_page;
      if (tmp > pl_max_pages) {
          pl_start_page = last_page;
          pl_max_pages = tmp;
      }
      last_page = page + 1;
  }

  if (pl_max_pages == 0x00) {
      pl_start_page = 0xff;
  }
  return (pl_start_page << 8);
}

void Loader::load_sid()
{
  print_sid_info();
  c64_->mem_->write_byte(0xD020,2); /* Bordor color to red */
  c64_->mem_->write_byte(0xD021,0); /* Background color to black */
  printf("load: $%04X play: $%04X init: $%04X length: $%X\n", pl_loadaddr, pl_playaddr, pl_initaddr, pl_datalength);

  if (!pl_isrsid) {
    /* Load SID data into memory */
    for (unsigned int i = 0; i < pl_datalength; i++) {
      c64_->mem_->write_byte_no_io((pl_loadaddr+i),pl_databuffer[i]);
      if (i == pl_datalength-1) printf("end: $%04X\n",(pl_loadaddr+i));
    }
    load_Psidplayer(pl_playaddr, pl_initaddr, pl_loadaddr, pl_datalength, pl_song_number);
  } else {
    printf("RSID not implemented yet! Try as PRG :)\n");
    psid_init_driver();
  }

  c64_->sid_->sid_flush();
  c64_->sid_->set_playing(true);
  if (!pl_isrsid) c64_->cpu_->pc(c64_->mem_->read_word_no_io(Memory::kAddrResetVector));
  // c64_->cpu_->pc(c64_->mem_->read_word(Memory::kAddrResetVector));
  if (!pl_isrsid) c64_->cpu_->irq();
  // c64_->mem_->setlogrw(0); /* Enable memory logging (TEST) */
  // Cpu::loginstructions = true;
}

void Loader::load_Psidplayer(uint16_t play, uint16_t init, uint16_t load, uint16_t length, int songno)
{
  D("Starting PSID player\n");
  playerstart = find_free_page();
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

extern "C" int reloc65(char** buf, int* fsize, int addr);

inline int Loader::psid_set_cbm80(uint16_t vec, uint16_t addr)
{
    unsigned int i;
    uint8_t cbm80[] = { 0x00, 0x00, 0x00, 0x00, 0xc3, 0xc2, 0xcd, 0x38, 0x30 };

    cbm80[0] = vec & 0xff;
    cbm80[1] = vec >> 8;

    for (i = 0; i < sizeof(cbm80); i++) {
        /* make backup of original content at 0x8000 */
        c64_->mem_->write_byte_no_io((uint16_t)(addr + i), c64_->mem_->read_byte_no_io((uint16_t)(0x8000 + i)));
        /* copy header */
        c64_->mem_->write_byte_no_io((uint16_t)(0x8000 + i), cbm80[i]);
    }

    return i;
}

void Loader::psid_init_driver(void) /* RSID ONLY */
{
  c64_->pla_->switch_banks(c64_->pla_->m13);  /* TEST */

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
    if ((subtune < 0) || (subtune > pl_songs)) {
      fprintf(stdout, "Default tune out of range (%d of %d ?), using 1 instead.\n", subtune, pl_songs);
      subtune = 1;
    } else printf("subtune: %d\n",subtune);

    /* Relocation setup. */
    if (pl_start_page == 0x00) {
      find_free_page();
    }

    if (pl_start_page == 0xff || pl_max_pages < 2) {
      fprintf(stdout, "No space for driver.\n");
      exit(1);
    }

  }

  // PSID_INIT_DRIVER
  {
    uint8_t psid_driver[] = {
#include "psiddrv.h"
    };
    char *psid_reloc = (char *)psid_driver;
    int psid_size;

    uint16_t reloc_addr;
    uint16_t addr;
    int i;
    int sync; // FIXED TO PAL
    int sid2loc, sid3loc;

    for (addr = 0; addr < 0x0800; addr++) {
        c64_->mem_->write_byte_no_io(addr, (uint8_t)0x00);
    }

    reloc_addr = pl_start_page << 8;
    psid_size = sizeof(psid_driver);
    fprintf(stdout, "PSID free pages: $%04x-$%04x\n",
            reloc_addr, (reloc_addr + (pl_max_pages << 8)) - 1U);

    if (!reloc65((char **)&psid_reloc, &psid_size, reloc_addr)) {
        fprintf(stderr, "Relocation.\n");
        // psid_set_tune(-1);
        return;
    }

    for (i = 0; i < psid_size; i++) {
        c64_->mem_->write_byte_no_io((uint16_t)(reloc_addr + i), psid_reloc[i]);
    }

    /* Load SID data into memory */
    for (unsigned int i = 0; i < pl_datalength; i++) {
      c64_->mem_->write_byte_no_io((pl_loadaddr+i),pl_databuffer[i]);
      if (i == pl_datalength-1) printf("end: $%04X\n",(pl_loadaddr+i));
    }

    /* Skip JMP and CBM80 reset vector. */
    addr = reloc_addr + 3 + 9 + 9;

    /* Store parameters for PSID player. */
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(0));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_songs));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_loadaddr & 0xff));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_loadaddr >> 8));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_initaddr & 0xff));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_initaddr >> 8));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_playaddr & 0xff));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_playaddr >> 8));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_curr_sidspeed & 0xff));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)((pl_curr_sidspeed >> 8) & 0xff));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)((pl_curr_sidspeed >> 16) & 0xff));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_curr_sidspeed >> 24));
    // c64_->mem_->write_byte_no_io(addr++, (uint8_t)((int)sync == MACHINE_SYNC_PAL ? 1 : 0));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)1);  // SYNC PAL
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_lastloadaddr & 0xff));
    c64_->mem_->write_byte_no_io(addr++, (uint8_t)(pl_lastloadaddr >> 8));
  }

  // PSID_INIT_TUNE
  {
    int start_song = subtune;
    int sync, sid_model;
    int i;
    uint16_t reloc_addr;
    uint16_t addr;
    int speedbit;
    char* irq;
    char irq_str[20];
    const char csidflag[4][8] = { "UNKNOWN", "6581", "8580", "ANY"};
    reloc_addr = pl_start_page << 8;
    fprintf(stdout, "Driver=$%04X, Image=$%04X-$%04X, Init=$%04X, Play=$%04X\n",
            reloc_addr, pl_loadaddr,
            (unsigned int)(pl_loadaddr + pl_datalength - 1),
            pl_initaddr, pl_playaddr);

    if (start_song == 0) {
      start_song = subtune;
    } else if (start_song < (subtune - 1) || start_song > pl_songs) {
      fprintf(stdout, "Tune out of range.\n");
      start_song = subtune;
    }
    printf("start_song: %d\n",start_song);
    /* Check tune speed. */
    speedbit = 1;
    for (i = 1; i < start_song && i < 32; i++) {
        speedbit <<= 1;
    }

    /* Store parameters for PSID player. */
    if (1 /* install_driver_hook */) {
        /* Skip JMP. */
        addr = reloc_addr + 3 + 9;

        /* CBM80 reset vector. */
        addr += psid_set_cbm80((uint16_t)(reloc_addr + 9), addr);

        c64_->mem_->write_byte_no_io(addr, (uint8_t)(start_song));
    }
    /* put song number into address 780/1/2 (A/X/Y) for use by BASIC tunes */
    c64_->mem_->write_byte_no_io(780, (uint8_t)(start_song - 1));
    c64_->mem_->write_byte_no_io(781, (uint8_t)(start_song - 1));
    c64_->mem_->write_byte_no_io(782, (uint8_t)(start_song - 1));
    /* force flag in c64 memory, many sids reads it and must be set AFTER the sid flag is read */
    // c64_->mem_->write_byte_no_io((uint16_t)(0x02a6), (uint8_t)(sync == MACHINE_SYNC_NTSC ? 0 : 1));
    c64_->mem_->write_byte_no_io((uint16_t)(0x02a6), (uint8_t)1); // FORCE PAL
    // c64_->cpu_->pc(psid->load_addr);
    // c64_->cpu_->pc((unsigned int)(psid->load_addr + pl_datalength - 1));
    c64_->cpu_->pc(pl_initaddr);
    // c64_->cpu_->pc(pl_playaddr);
    // c64_->cpu_->pc(reloc_addr);
    // c64_->cpu_->reset();
    // c64_->cpu_->irq();
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
