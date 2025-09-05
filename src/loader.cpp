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

  if(is_.is_open())
  { /* BUG: */
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
  }
  c64_->cpu_->pc(addr);
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
}

void Loader::load_sid()
{
  pl_songs = sidfile_->GetNumOfSongs();
  pl_song_number = sidfile_->GetFirstSong(); /* TODO: FIX N FINISH ;) */
  pl_sidflags = sidfile_->GetSidFlags();
  pl_sidspeed = sidfile_->GetSongSpeed(pl_song_number); // + 1);
  pl_curr_sidspeed = pl_sidspeed & (1 << pl_song_number); // ? 1 : 0;  // 1 ~ 60Hz, 2 ~ 50Hz
  pl_chiptype = sidfile_->GetChipType(1);
  pl_clockspeed = sidfile_->GetClockSpeed();
  pl_sidversion = sidfile_->GetSidVersion();
  pl_clock_speed = clockSpeed[pl_clockspeed];
  pl_raster_lines = scanLines[pl_clockspeed];
  pl_rasterrow_cycles = scanlinesCycles[pl_clockspeed];
  pl_frame_cycles = pl_raster_lines * pl_rasterrow_cycles;
  pl_refresh_rate = refreshRate[pl_clockspeed];
  pl_loadaddr = sidfile_->GetLoadAddress();
  pl_datalength = sidfile_->GetDataLength();
  pl_databuffer = sidfile_->GetDataPtr();
  pl_playaddr = sidfile_->GetPlayAddress();
  pl_initaddr = sidfile_->GetInitAddress();

  print_sid_info();
  c64_->mem_->write_byte(0xD020,2); /* Bordor color to red */
  c64_->mem_->write_byte(0xD021,0); /* Background color to black */

  /* Load SID data into memory */
  for (unsigned int i = 0; i < pl_datalength; i++) {
    c64_->mem_->write_byte((pl_loadaddr+i),pl_databuffer[i]);
  }
  printf("load: $%04X play: $%04X init: $%04X\n",
    pl_loadaddr, pl_playaddr, pl_initaddr);

  // if (sidfile_->GetSidType() == "PSID")
  load_sidplayer(pl_playaddr, pl_initaddr, pl_song_number);
  // load_sidplayerA(pl_playaddr, pl_initaddr, pl_song_number);
  // load_sidplayerB(pl_playaddr, pl_initaddr, pl_song_number);
  // load_sidplayerC(pl_playaddr, pl_initaddr, pl_song_number);

  c64_->sid_->sid_flush();
  c64_->sid_->set_playing(true);
  c64_->cpu_->pc(c64_->mem_->read_word(Memory::kAddrResetVector));
}

void Loader::load_sidplayer(uint16_t play, uint16_t init, int songno)
{
  // install reset vector for microplayer (0x0202)
  c64_->mem_->write_byte(0xFFFC, 0x02);
  c64_->mem_->write_byte(0xFFFD, 0x02);

  // install IRQ vector for play routine launcher (0x020C)
  c64_->mem_->write_byte(0xFFFE, 0x0C);
  c64_->mem_->write_byte(0xFFFF, 0x02);

  // clear kernel and basic rom from ram
  c64_->mem_->write_byte(0x0001, 0x1C); /* 35 */

  // install the micro player, 6502 assembly code
  c64_->mem_->write_byte(0x0202, 0xA9);               // 0xA9 LDA imm load A with the song number
  c64_->mem_->write_byte(0x0203, songno);             // 0xNN #NN song number

  c64_->mem_->write_byte(0x0204, 0x20);               // 0x20 JSR abs jump sub to INIT routine
  c64_->mem_->write_byte(0x0205, init & 0xFF);        // 0xxxNN address lo
  c64_->mem_->write_byte(0x0206, (init >> 8) & 0xFF); // 0xNNxx address hi

  c64_->mem_->write_byte(0x0207, 0x58);               // 0x58 CLI enable interrupt
  c64_->mem_->write_byte(0x0208, 0xEA);               // 0xEA NOP impl
  c64_->mem_->write_byte(0x0209, 0x4C);               // JMP jump to 0x0006
  c64_->mem_->write_byte(0x020A, 0x08);               // 0xxxNN address lo
  c64_->mem_->write_byte(0x020B, 0x02);               // 0xNNxx address hi

  c64_->mem_->write_byte(0x020C, 0xEA);               // 0xEA NOP // 0xA9 LDA imm // A = 1
  c64_->mem_->write_byte(0x020D, 0xEA);               // 0xEA NOP // 0x01 #NN
  c64_->mem_->write_byte(0x020E, 0xEA);               // 0xEA NOP // 0x78 SEI disable interrupt
  c64_->mem_->write_byte(0x020F, 0x20);               // 0x20 JSR jump sub to play routine
  c64_->mem_->write_byte(0x0210, play & 0xFF);        // playaddress lo
  c64_->mem_->write_byte(0x0211, (play >> 8) & 0xFF); // playaddress hi
  c64_->mem_->write_byte(0x0212, 0xEA);               // 0xEA NOP // 0x58 CLI enable interrupt
  c64_->mem_->write_byte(0x0213, 0x40);               // 0x40 RTI return from interrupt
}

void Loader::load_sidplayerA(uint16_t play, uint16_t init, int songno)
{
  // install reset vector for microplayer (0x0000)
  c64_->mem_->write_byte(0xFFFD, 0x00);
  c64_->mem_->write_byte(0xFFFC, 0x02);

  // install IRQ vector for play routine launcher (0x000C)
  c64_->mem_->write_byte(0xFFFF, 0x00);
  c64_->mem_->write_byte(0xFFFE, 0x0C);

  // clear kernel and basic rom from ram
  c64_->mem_->write_byte(0x0001, 0x35);

  // install the micro player, 6502 assembly code
  c64_->mem_->write_byte(0x0002, 0xA9);               // 0xA9 LDA imm load A with the song number
  c64_->mem_->write_byte(0x0003, songno);             // 0xNN #NN song number

  c64_->mem_->write_byte(0x0004, 0x20);               // 0x20 JSR abs jump sub to INIT routine
  c64_->mem_->write_byte(0x0005, init & 0xFF);        // 0xxxNN address lo
  c64_->mem_->write_byte(0x0006, (init >> 8) & 0xFF); // 0xNNxx address hi

  c64_->mem_->write_byte(0x0007, 0x58);               // 0x58 CLI enable interrupt
  c64_->mem_->write_byte(0x0008, 0xEA);               // 0xEA NOP impl
  c64_->mem_->write_byte(0x0009, 0x4C);               // JMP jump to 0x0006
  c64_->mem_->write_byte(0x000A, 0x08);               // 0xxxNN address lo
  c64_->mem_->write_byte(0x000B, 0x00);               // 0xNNxx address hi

  c64_->mem_->write_byte(0x000C, 0xEA);               // 0xEA NOP // 0xA9 LDA imm // A = 1
  c64_->mem_->write_byte(0x000D, 0xEA);               // 0xEA NOP // 0x01 #NN
  c64_->mem_->write_byte(0x000E, 0xEA);               // 0xEA NOP // 0x78 SEI disable interrupt
  c64_->mem_->write_byte(0x000F, 0x20);               // 0x20 JSR jump sub to play routine
  c64_->mem_->write_byte(0x0010, play & 0xFF);        // playaddress lo
  c64_->mem_->write_byte(0x0011, (play >> 8) & 0xFF); // playaddress hi
  c64_->mem_->write_byte(0x0012, 0xEA);               // 0xEA NOP // 0x58 CLI enable interrupt
  c64_->mem_->write_byte(0x0013, 0x40);               // 0x40 RTI return from interrupt
}

void Loader::load_sidplayerB(uint16_t play, uint16_t init, int songno)
{ /* BUG: Doesn't work at all! */
  // install reset vector for microplayer (0x0000)
  c64_->mem_->write_byte(0xFFFD, 0x00);
  c64_->mem_->write_byte(0xFFFC, 0x02);

  // install IRQ vector for play routine launcher (0x0813)
  c64_->mem_->write_byte(0xFFFF, 0xD7);
  c64_->mem_->write_byte(0xFFFE, 0xE0);

  // clear kernel and basic rom from ram
  // c64_->mem_->write_byte(0x0001, 0x35);

  // install the micro pl`ayer, 6502 assembly code
  c64_->mem_->write_byte(0xD7E0, 0xA9);               // 0xA9 LDA imm load A with the song number
  c64_->mem_->write_byte(0xD7E1, songno);             // 0xNN #NN song number

  c64_->mem_->write_byte(0xD7E2, 0x20);               // 0x20 JSR abs jump sub to INIT routine
  c64_->mem_->write_byte(0xD7E3, init & 0xFF);        // 0xxxNN address lo
  c64_->mem_->write_byte(0x0004, (init >> 8) & 0xFF); // 0xNNxx address hi

  // infinite loop?
  c64_->mem_->write_byte(0xD7E5, 0x58);               // 0x58 CLI enable interrupt
  c64_->mem_->write_byte(0xD7E6, 0xEA);               // 0xEA NOP impl
  c64_->mem_->write_byte(0xD7E7, 0x4C);               // JMP jump to 0x0006
  c64_->mem_->write_byte(0xD7E8, 0x08);               // 0xxxNN address lo
  c64_->mem_->write_byte(0xD7E9, 0x00);               // 0xNNxx address hi

  // install the play routine launcher
  c64_->mem_->write_byte(0xD7EA, 0xEA);               // 0xEA NOP // 0xA9 LDA imm // A = 1
  c64_->mem_->write_byte(0xD7EB, 0xEA);               // 0xEA NOP // 0x01 #NN
  c64_->mem_->write_byte(0xD7EC, 0xEA);               // 0xEA NOP // 0x78 SEI disable interrupt
  c64_->mem_->write_byte(0xD7ED, 0x20);               // 0x20 JSR jump sub to play routine
  c64_->mem_->write_byte(0xD7EE, play & 0xFF);        // playaddress lo
  c64_->mem_->write_byte(0xD7EF, (play >> 8) & 0xFF); // playaddress hi
  c64_->mem_->write_byte(0xD7F0, 0xEA);               // 0xEA NOP // 0x58 CLI enable interrupt
  c64_->mem_->write_byte(0xD7F1, 0x40);               // 0x40 RTI return from interrupt
}

void Loader::load_sidplayerC(uint16_t play, uint16_t init, int songno)
{ /* BUG: Causes turbo play and other issues, do not use */
  // install reset vector for microplayer (0x0000)
  c64_->mem_->write_byte(0xFFFD, 0x00);
  c64_->mem_->write_byte(0xFFFC, 0x02);

  // install IRQ vector for play routine launcher (0x0813)
  c64_->mem_->write_byte(0xFFFF, 0x00);
  c64_->mem_->write_byte(0xFFFE, 0x13);

  // clear kernel and basic rom from ram
  // c64_->mem_->write_byte(0x0001, 0x35);

  // install the micro player, 6502 assembly code
  c64_->mem_->write_byte(0x0000, 0xA9);               // 0xA9 LDA imm load A with the song number
  c64_->mem_->write_byte(0x0001, songno);             // 0xNN #NN song number

  c64_->mem_->write_byte(0x0002, 0x20);               // 0x20 JSR abs jump sub to INIT routine
  c64_->mem_->write_byte(0x0003, init & 0xFF);        // 0xxxNN address lo
  c64_->mem_->write_byte(0x0004, (init >> 8) & 0xFF); // 0xNNxx address hi

  // infinite loop?
  c64_->mem_->write_byte(0x0005, 0x58);               // 0x58 CLI enable interrupt
  c64_->mem_->write_byte(0x0006, 0xEA);               // 0xEA NOP impl
  c64_->mem_->write_byte(0x0007, 0x4C);               // JMP jump to 0x0006
  c64_->mem_->write_byte(0x0008, 0x08);               // 0xxxNN address lo
  c64_->mem_->write_byte(0x0009, 0x00);               // 0xNNxx address hi

  // install the play routine launcher
  c64_->mem_->write_byte(0x0013, 0xEA);               // 0xEA NOP // 0xA9 LDA imm // A = 1
  c64_->mem_->write_byte(0x0014, 0xEA);               // 0xEA NOP // 0x01 #NN
  c64_->mem_->write_byte(0x0015, 0xEA);               // 0xEA NOP // 0x78 SEI disable interrupt
  c64_->mem_->write_byte(0x0016, 0x20);               // 0x20 JSR jump sub to play routine
  c64_->mem_->write_byte(0x0017, play & 0xFF);        // playaddress lo
  c64_->mem_->write_byte(0x0018, (play >> 8) & 0xFF); // playaddress hi
  c64_->mem_->write_byte(0x0019, 0xEA);               // 0xEA NOP // 0x58 CLI enable interrupt
  c64_->mem_->write_byte(0x001A, 0x40);               // 0x40 RTI return from interrupt
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
  cout << "Image length       : $" << hex << sidfile_->GetInitAddress() << " - $" << hex << (sidfile_->GetInitAddress() - 1) + sidfile_->GetDataLength() << endl;
  cout << "Load Address       : $" << hex << sidfile_->GetLoadAddress() << endl;
  cout << "Init Address       : $" << hex << sidfile_->GetInitAddress() << endl;
  cout << "Play Address       : $" << hex << sidfile_->GetPlayAddress() << endl;
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
  if(booted_up_)
  {
    handle_file();
    return false;
  }
  else
  {
    /* at this point BASIC is ready */
    if(c64_->cpu_->pc() == 0xa65c)
      booted_up_ = true;
  }
  return true;
}
