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

#include "loader.h"
#include "sidfile.h"
#include <bitset>
#include <iomanip>

// const char *sidtype_s[5] = {"Unknown", "N/A", "MOS8580", "MOS6581", "FMopl" };  /* 0 = unknown, 1 = N/A, 2 = MOS8085, 3 = MOS6581, 4 = FMopl */
const char *chiptype_s[4] = {"Unknown", "MOS6581", "MOS8580", "MOS6581 and MOS8580"};
const char *clockspeed_s[5] = {"Unknown", "PAL", "NTSC", "PAL and NTSC", "DREAN"};

Loader::Loader(C64 *c64)
{
  c64_ = c64;
  io_  = c64_->io();
  cpu_ = c64_->cpu();
  mem_ = c64_->memory();
  vic_ = c64_->vic();
  sid_ = c64_->sid();
  booted_up_ = false;
  format_ = kNone;
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
      io_->IO::type_character(c);
    }
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
  uint16_t pbuf, addr;
  pbuf = addr = read_short_le();
  if(is_.is_open())
  {
    while(is_.good())
    {
      is_.get(b);
      mem_->write_byte_no_io(pbuf++,b);
    }
    /* basic-tokenized prg */
    if(addr == kBasicPrgStart)
    {
      /* make BASIC happy */
      mem_->write_word_no_io(kBasicTxtTab,kBasicPrgStart);
      mem_->write_word_no_io(kBasicVarTab,pbuf);
      mem_->write_word_no_io(kBasicAryTab,pbuf);
      mem_->write_word_no_io(kBasicStrEnd,pbuf);
      /* exec RUN */
      for(char &c: std::string("RUN\n"))
        io_->IO::type_character(c);
    }
    /* ML */
    else cpu_->pc(addr);
  }
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
  print_sid_info();
  mem_->write_byte(0xD020,2); /* Bordor color to red */
  mem_->write_byte(0xD021,0); /* Background color to black */
  /* Turn off screen by wrapping the border color over the whole screen */
  // mem_->write_byte(0xD011,(mem_->read_byte(0xD011) & 0b11101111));
  uint16_t load = sidfile_->GetLoadAddress();
  uint16_t len = sidfile_->GetDataLength();
  uint8_t *buffer = sidfile_->GetDataPtr();

  for (unsigned int i = 0; i < len; i++) {
    mem_->write_byte((load+i),buffer[i]);
  }
  uint16_t play = sidfile_->GetPlayAddress();
  uint16_t init = sidfile_->GetInitAddress();

  int songno = sidfile_->GetFirstSong();
  // cpu_->reset();

  // install reset vector for microplayer (0x0000)
  mem_->write_byte(0xFFFD, 0x00);
  mem_->write_byte(0xFFFC, 0x00);

  // install IRQ vector for play routine launcher (0x0013)
  mem_->write_byte(0xFFFF, 0x00);
  mem_->write_byte(0xFFFE, 0x13);

  mem_->write_byte(0x0001, 0x35); // clear kernel and basic rom from ram

  // install the micro player, 6502 assembly code
  mem_->write_byte(0x0000, 0xA9);               // 0xA9 LDA imm load A with the song number
  mem_->write_byte(0x0001, songno);             // 0xNN #NN song number

  mem_->write_byte(0x0002, 0x20);               // 0x20 JSR abs jump sub to INIT routine
  mem_->write_byte(0x0003, init & 0xFF);        // 0xxxNN address lo
  mem_->write_byte(0x0004, (init >> 8) & 0xFF); // 0xNNxx address hi

  mem_->write_byte(0x0005, 0x58);               // 0x58 CLI enable interrupt
  mem_->write_byte(0x0006, 0xEA);               // 0xEA NOP impl
  mem_->write_byte(0x0007, 0x4C);               // JMP jump to 0x0006
  mem_->write_byte(0x0008, 0x06);               // 0xxxNN address lo
  mem_->write_byte(0x0009, 0x00);               // 0xNNxx address hi

  mem_->write_byte(0x0013, 0xEA);               // 0xEA NOP // 0xA9 LDA imm // A = 1
  mem_->write_byte(0x0014, 0xEA);               // 0xEA NOP // 0x01 #NN
  mem_->write_byte(0x0015, 0xEA);               // 0xEA NOP // 0x78 SEI disable interrupt
  mem_->write_byte(0x0016, 0x20);               // 0x20 JSR jump sub to play routine
  mem_->write_byte(0x0017, play & 0xFF);        // playaddress lo
  mem_->write_byte(0x0018, (play >> 8) & 0xFF); // playaddress hi
  mem_->write_byte(0x0019, 0xEA);               // 0xEA NOP // 0x58 CLI enable interrupt
  mem_->write_byte(0x001A, 0x40);               // 0x40 RTI return from interrupt
  // vic_->disable(true); /* Disable screen output */    // TODO: FINISH, IS BROKEN NOW
  sid_->reset_cycles();
  // cpu_->reset();
  cpu_->pc(mem_->read_word(Memory::kAddrResetVector));

  // mem_->write_byte(0xD020,0); /* Bordor color to black */
  // mem_->write_byte(0xD021,0); /* Background color to black */
  // mem_->write_byte(0xD011,(mem_->read_byte(0xD011) & 0b11101111)); /* Turn off screen by wrapping it with border */

}

void Loader::print_sid_info() /* TODO: THIS MUST GO TO C64 SCREEN */
{
  int songs = sidfile_->GetNumOfSongs();
  int song_number = sidfile_->GetFirstSong(); /* TODO: FIX N FINISH ;) */
  int sidflags = sidfile_->GetSidFlags();
  uint32_t sidspeed = sidfile_->GetSongSpeed(song_number); // + 1);
  int curr_sidspeed = sidspeed & (1 << song_number); // ? 1 : 0;  // 1 ~ 60Hz, 2 ~ 50Hz
  int ct = sidfile_->GetChipType(1);
  int cs = sidfile_->GetClockSpeed();
  int sv = sidfile_->GetSidVersion();
  // 2 2 3 ~ Genius
  // 2 1 3 ~ Jump
  // printf("%d %d %d\n", ct, cs, sv);
  int clock_speed = clockSpeed[cs];
  int raster_lines = scanLines[cs];
  int rasterrow_cycles = scanlinesCycles[cs];
  int frame_cycles = raster_lines * rasterrow_cycles;
  int refresh_rate = refreshRate[cs];

  cout << "\n< Sid Info >" << endl;
  cout << "---------------------------------------------" << endl;
  cout << "SID Title          : " << sidfile_->GetModuleName() << endl;
  cout << "Author Name        : " << sidfile_->GetAuthorName() << endl;
  cout << "Release & (C)      : " << sidfile_->GetCopyrightInfo() << endl;
  cout << "---------------------------------------------" << endl;
  cout << "SID Type           : " << sidfile_->GetSidType() << endl;
  cout << "SID Format version : " << dec << sv << endl;
  cout << "---------------------------------------------" << endl;
  // cout << "SID Flags          : 0x" << hex << sidflags << " 0b" << bitset<8>{sidflags} << endl;
  cout << "Chip Type          : " << chiptype_s[ct] << endl;
  if (sv == 3 || sv == 4)
      cout << "Chip Type 2        : " << chiptype_s[sidfile_->GetChipType(2)] << endl;
  if (sv == 4)
      cout << "Chip Type 3        : " << chiptype_s[sidfile_->GetChipType(3)] << endl;
  cout << "Clock Type         : " << hex << clockSpeed[cs] << endl;
  cout << "Clock Speed        : " << dec << clock_speed << endl;
  cout << "Raster Lines       : " << dec << raster_lines << endl;
  cout << "Rasterrow Cycles   : " << dec << rasterrow_cycles << endl;
  cout << "Frame Cycles       : " << dec << frame_cycles << endl;
  cout << "Refresh Rate       : " << dec << refresh_rate << endl;
  if (sv == 3 || sv == 4 || sv == 78) {
      cout << "---------------------------------------------" << endl;
      cout << "SID 2 $addr        : $d" << hex << sidfile_->GetSIDaddr(2) << "0" << endl;
      if (sv == 4 || sv == 78)
          cout << "SID 3 $addr        : $d" << hex << sidfile_->GetSIDaddr(3) << "0" << endl;
      if (sv == 78)
          cout << "SID 4 $addr        : $d" << hex << sidfile_->GetSIDaddr(4) << "0" << endl;
  }
  cout << "---------------------------------------------" << endl;
  cout << "Data Offset        : $" << setfill('0') << setw(4) << hex << sidfile_->GetDataOffset() << endl;
  cout << "Image length       : $" << hex << sidfile_->GetInitAddress() << " - $" << hex << (sidfile_->GetInitAddress() - 1) + sidfile_->GetDataLength() << endl;
  cout << "Load Address       : $" << hex << sidfile_->GetLoadAddress() << endl;
  cout << "Init Address       : $" << hex << sidfile_->GetInitAddress() << endl;
  cout << "Play Address       : $" << hex << sidfile_->GetPlayAddress() << endl;
  cout << "---------------------------------------------" << endl;
  cout << "Song Speed(s)      : $" << hex << curr_sidspeed << " $0x" << hex << sidspeed << " 0b" << bitset<32>{sidspeed} << endl;
  cout << "Timer              : " << (curr_sidspeed == 1 ? "CIA1" : "Clock") << endl;
  cout << "Selected Sub-Song  : " << dec << song_number + 1 << " / " << dec << sidfile_->GetNumOfSongs() << endl;
}


// emulate //////////////////////////////////////////////////////////////////

bool Loader::emulate()
{
  if(booted_up_)
  {
    switch(format_)
    {
    case kBasic:
      load_basic();
      break;
    case kPRG:
      load_prg();
      break;
    case kSID:
      load_sid();
      break;
    default:
      break;
    }
    return false;
  }
  else
  {
    /* at this point BASIC is ready */
    if(cpu_->pc() == 0xa65c)
      booted_up_ = true;
  }
  return true;
}
