/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * memory.cppb
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

#include <fstream>
#include <iomanip>

#include <c64.h> /* All classes are loaded through c64.h */


Memory::Memory(C64 * c64) :
  c64_(c64)
{
  /**
   * 64 kB memory buffers, zeroed.
   *
   * We use two buffers to handle special circumstances, for instance,
   * any write to a ROM-mapped location will in turn store data on the
   * hidden RAM, this trickery is used in certain graphic modes.
   */
  mem_ram_ = new uint8_t[kMemSize]();
  mem_rom_ = new uint8_t[kMemSize]();

  /**
   * 1 KB _Read only_ page buffers, zeroed.
   */
  mem_rom_cia1_ = new uint8_t[kPageSize]();
  mem_rom_cia2_ = new uint8_t[kPageSize]();

  /* configure pointers */
  kCIA1MemWr = &mem_ram_[kAddrCIA1Page];
  kCIA1MemRd = &mem_rom_cia1_[0];
  kCIA2MemWr = &mem_ram_[kAddrCIA2Page];
  kCIA2MemRd = &mem_rom_cia2_[0];
}

Memory::~Memory()
{
  delete [] mem_ram_;
  delete [] mem_rom_;
  delete [] mem_rom_cia1_;
  delete [] mem_rom_cia2_;
}


/**
 * @brief writes a byte to RAM without performing I/O
 */
void Memory::write_byte_no_io(uint16_t addr, uint8_t v)
{
  mem_ram_[addr] = v;
}

/**
 * @brief writes a byte to RAM handling I/O
 */
void Memory::write_byte(uint16_t addr, uint8_t v)
{ /* TODO: Account for new bank modes! */
  if(logmemrw){D("[MEM  W] $%04X:%02X\n",addr,v);};
  uint16_t page = addr&0xff00;
  /* ZeroPage ~ $0000/$00ff */
  if (page == kAddrZeroPage)
  {
    /* bank switching $01 */
    if (addr == kAddrMemoryLayout) {
      c64_->pla_->runtime_bank_switching(v); /* Setup (new) runtime bank config */
    } else {
      // if (addr == kAddrDataDirection) {
      //   D("Datadirection change from %02X to %02X\n", mem_ram_[kAddrDataDirection], v);
      // }
      mem_ram_[addr] = v; /* Write to RAM */
    }
  }
  /* VIC-II DMA or Character ROM ~ $d000/$d3ff */
  else if (page >= kAddrVicFirstPage
        && page <= kAddrVicLastPage)
  {
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kIO) {
      c64_->vic_->write_register(addr&0x7f,v); /* VIC-II write */
    } else {
      mem_ram_[addr] = v; /* Write to RAM */
    }
  }
  /* CIA1 ~ $dc00/$dcff */
  else if (page == kAddrCIA1Page)
  {
    if(logcia1rw){D("[CIA1 W] $%04X:%02X\n",addr,v);};
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kIO) {
      c64_->cia1_->write_register(addr&0x0f,v);
    } else {
      mem_ram_[addr] = v; /* Write to RAM */
    }
  }
  /* CIA2 ~ $dd00/$ddff */
  else if (page == kAddrCIA2Page)
  {
    if(logcia2rw){D("[CIA2 W] $%04X:%02X\n",addr,v);};
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kIO) {
      c64_->cia2_->write_register(addr&0x0f,v);
    } else {
      mem_ram_[addr] = v; /* Write to RAM */
    }
  }
  /* SID ~ $d400/$d7ff */
  else if (page >= kAddrSIDFirstPage
        && page <= kAddrSIDLastPage)
  {
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kIO) {
      if (page == 0xd400) { /* TODO: Check for SID number */
        mem_ram_[addr] = v; /* Always write to RAM */
        c64_->sid_->write_register((uint8_t)(addr&0x1F), v, 0); /* TODO: This should not be fixed */
      } else {
        mem_ram_[addr] = v; /* Write to RAM */
      }
    }
    else {
      mem_ram_[addr] = v; /* Write to RAM */
    }
  }
  /* IO1 ~ $de00 */
  else if (page == kAddrIO1Page)
  {
    /* TODO: CHECK FOR ENABLED IO / CART DEVICES */
    if(logiorw){D("[IO1  W] $%04X:%02X\n",addr,v);};
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kIO) { /* TODO: CHECK IF THIS IS THE RIGHT BANK CHECK */
      c64_->cart_->write_register((addr&0xFF),v);
    } else {
      mem_ram_[addr] = v; /* Write to RAM */
    }
  }
  /* IO2 ~ $df00 */
  else if (page == kAddrIO1Page)
  {
    /* TODO: CHECK FOR ENABLED IO / CART DEVICES */
    if(logiorw){D("[IO2  W] $%04X:%02X\n",addr,v);};
    mem_ram_[addr] = v; /* Write to RAM */
  }
  /* default */
  else
  {
    mem_ram_[addr] = v; /* Write to RAM */
  }
}

/**
 * @brief reads a byte from RAM or ROM (depending on bank config)
 */
uint8_t Memory::read_byte(uint16_t addr)
{ /* TODO: Account for new bank modes! */
  uint8_t  retval = 0;
  uint16_t page   = addr&0xff00;
  /* RAM ~ $1000/$1fff */
  if (page >= kAddrRAM1FirstPage
        && page <= kAddrRAM1LastPage)
  {
    if(c64_->pla_->memory_banks(PLA::kBankCart) == PLA::kUNM) {
      retval = 0xFF; /* TODO: What to return if unmapped!? */
    } else {
      retval = mem_ram_[addr];
    }
  }
  /* CART LORAM ~ $8000/$9fff */
  else if (page >= kAddrCartLoFirstPage
        && page <= kAddrCartLoLastPage)
  {
    if(c64_->pla_->memory_banks(PLA::kBankCart) == PLA::kCLO) {
      retval = kCARTRomLo[(addr-0x8000)]; /* TODO: Move to Cart? */
      if (logcrtrw) {D("[CART R] $%04X:%02X\n",addr,retval);};
    } else {
      retval = mem_ram_[addr];
    }
  }
  /* BASIC or RAM ~ $a000/$bfff */
  else if (page >= kAddrBasicFirstPage
   && page <= kAddrBasicLastPage)
  {
    if (c64_->pla_->memory_banks(PLA::kBankBasic) == PLA::kROM) {
      retval = mem_rom_[addr];
    } else if (c64_->pla_->memory_banks(PLA::kBankBasic) == PLA::kCHI) {
      retval = kCARTRomHi1[(addr-0xa000)]; /* TODO: Move to Cart? */
    } else {
      retval = mem_ram_[addr];
    }
  }
  /* RAM ~ $c000/$cfff */
  else if (page == kAddrRAM2Page)
  {
    if(c64_->pla_->memory_banks(PLA::kBankCart) == PLA::kUNM) {
      retval = 0xFF; /* TODO: What to return if unmapped!? */
    } else {
      retval = mem_ram_[addr];
    }
  }
  /* VIC-II DMA or Character ROM ~ $d000/$d3ff */
  else if (page >= kAddrVicFirstPage
   && page <= kAddrVicLastPage)
  {
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kIO) {
      retval = c64_->vic_->read_register(addr&0x7f);
    } else if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kROM) {
      retval = mem_rom_[addr]; /* Read from ROM */
    } else {
      retval = mem_ram_[addr]; /* Read from RAM */
    }
  }
  /* SID ~ $d400/$d7ff */
  else if (page >= kAddrSIDFirstPage
        && page <= kAddrSIDLastPage)
  {
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kIO) {
      retval = mem_ram_[addr]; /* Read from RAM */
    } else if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kROM) {
      retval = mem_rom_[addr]; /* Read from ROM */
    } else {
      retval = mem_ram_[addr]; /* Read from RAM */
    }
  }
  /* Color RAM ~ $d800/$dbff */
  else if (page >= kAddrColorFirstPage
        && page <= kAddrColorLastPage)
  {
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kROM) {
      retval = mem_rom_[addr]; /* Read from ROM */
    } else {
      retval = mem_ram_[addr]; /* Read from RAM */
    }
  }
  /* CIA1 ~ $dc00/$dcff */
  else if (page == kAddrCIA1Page)
  {
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kIO) {
      retval = c64_->cia1_->read_register(addr&0x0f);
    } else if (c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kROM) {
      retval = mem_rom_[addr]; /* Read from ROM */
    } else {
      retval = mem_ram_[addr]; /* Read from RAM */
    }
    if(logcia1rw){D("[CIA1 R] $%04X:%02X\n",addr,retval);};
  }
  /* CIA2 ~ $dd00/$ddff */
  else if (page == kAddrCIA2Page)
  {
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kIO) {
      retval = c64_->cia2_->read_register(addr&0x0f);
    } else if (c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kROM) {
      retval = mem_rom_[addr]; /* Read from ROM */
    } else {
      retval = mem_ram_[addr];
    }
    if(logcia2rw){D("[CIA2 R] $%04X:%02X\n",addr,retval);};
  }
  /* IO1 ~ $de00/$deff */
  else if (page == kAddrIO1Page)
  {
    /* TODO: Add check for what is enabled! */
    // if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kROM) {
    //   retval = mem_rom_[addr]; /* Read from ROM */
    // } else {
    //   retval = mem_ram_[addr]; /* Read from RAM */
    // }

    /* TODO: CHECK FOR ENABLED IO / CART DEVICES */
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kIO) { /* TODO: CHECK IF THIS IS THE RIGHT BANK CHECK */
      retval = c64_->cart_->read_register((addr&0xFF));
    } else {
      retval = mem_ram_[addr]; /* Read from RAM */
    }
    if(logiorw){D("[IO1  R] $%04X:%02X\n",addr,retval);};
  }
  /* IO2 ~ $df00/$dfff */
  else if (page == kAddrIO2Page)
  {
    if(c64_->pla_->memory_banks(PLA::kBankChargen) == PLA::kROM) {
      retval = mem_rom_[addr]; /* Read from ROM */
    } else {
      retval = mem_ram_[addr]; /* Read from RAM */
    }
    if(logiorw){D("[IO2  R] $%04X:%02X\n",addr,retval);};
  }
  /* KERNAL ~ $e000/$ffff */
  else if (page >= kAddrKernalFirstPage
        && page <= kAddrKernalLastPage)
  {
    if (c64_->pla_->memory_banks(PLA::kBankKernal) == PLA::kROM) {
      retval = mem_rom_[addr];
    } else if (c64_->pla_->memory_banks(PLA::kBankBasic) == PLA::kCHI) {
      retval = kCARTRomHi2[(addr-0xe000)]; /* TODO: Move to Cart? */
    } else {
      retval = mem_ram_[addr];
    }
  }
  /* default ~ any other address */
  else
  {
    retval = mem_ram_[addr];
  }
  if(logmemrw){D("[MEM  R] $%04X:%02X\n",addr,retval);};
  return retval;
}

/**
 * @brief reads a byte without performing I/O
 */

uint8_t Memory::read_byte_no_io(uint16_t addr)
{
  return mem_ram_[addr];
}

/**
 * @brief reads a word performing I/O
 */
uint16_t Memory::read_word(uint16_t addr)
{
  return read_byte(addr) | (read_byte(addr+1) << 8);
}

/**
 * @brief reads a word withouth performing I/O
 */
uint16_t Memory::read_word_no_io(uint16_t addr)
{
  return read_byte_no_io(addr) | (read_byte_no_io(addr+1) << 8);
}

/**
 * @brief writes a word performing I/O
 */
void Memory::write_word(uint16_t addr, uint16_t v)
{
  write_byte(addr, (uint8_t)(v));
  write_byte(addr+1, (uint8_t)(v>>8));
}

/**
 * @brief writes a word without performing I/O
 */
void Memory::write_word_no_io(uint16_t addr, uint16_t v)
{
  write_byte_no_io(addr, (uint8_t)(v));
  write_byte_no_io(addr+1, (uint8_t)(v>>8));
}

/**
 * @brief read byte (from VIC's perspective)
 *
 * The VIC has only 14 address lines so it can only access
 * 16kB of memory at once, the two missing address bits are
 * provided by CIA2.
 *
 * The VIC always reads from RAM ignoring the memory configuration,
 * there's one exception: the character generator ROM. Unless the
 * Ultimax mode is selected, VIC sees the character generator ROM
 * in the memory areas:
 *
 *  1000-1FFF
 *  9000-9FFF
 */
uint8_t Memory::vic_read_byte(uint16_t addr)
{
  uint8_t v;
  uint16_t vic_addr = c64_->cia2_->vic_base_address() + (addr & 0x3fff);
  if((vic_addr >= 0x1000 && vic_addr <  0x2000) ||
     (vic_addr >= 0x9000 && vic_addr <  0xa000))
    v = mem_rom_[kBaseAddrChars + (vic_addr & 0xfff)];
  else
    v = read_byte_no_io(vic_addr);
  return v;
}

/**
 * @brief loads a external binary into ROM
 */
bool Memory::load_rom(const std::string &f, uint16_t baseaddr)
{
  // std::string path = "./assets/roms/" + f;
  std::string path = "./assets/" + f;
  std::ifstream is(path, std::ios::in | std::ios::binary);
  if(is)
  {
    is.seekg (0, is.end);
    std::streamoff length = is.tellg();
    is.seekg (0, is.beg);
    is.read ((char *) &mem_rom_[baseaddr],length);
    return true;
  }
  return false;
}

/**
 * @brief loads a external binary into RAM
 */
bool Memory::load_ram(const std::string &f, uint16_t baseaddr)
{
  std::string path = "./assets/" + f;
  std::ifstream is(path, std::ios::in | std::ios::binary);
  if(is)
  {
    is.seekg (0, is.end);
    std::streamoff length = is.tellg();
    is.seekg (0, is.beg);
    is.read ((char *) &mem_ram_[baseaddr],length);
    return true;
  }
  return false;
}

// debug ////////////////////////////////////////////////////////////////////

/**
 * @brief dumps memory as seen by the CPU to stdout
 */
void Memory::dump()
{
  for(unsigned int p=0 ; p < kMemSize ; p++)
  {
    if (p % 15 == 0 || p == 0) std::cout << std::setw(5) << std::setfill('0') << (p == 0 ? p : p - 16) << " ";
    std::cout << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << int(read_byte_no_io(p)) << " ";
    if (p % 15 == 0 && p != 0) std::cout << std::endl;
  }
}
/**
 * @brief dumps memory range as seen by the CPU to stdout
 */
void Memory::dump(uint16_t start, uint16_t end)
{
  for(unsigned int p=start ; p < (end<=kMemSize?end:kMemSize) ; p++)
  {
    if (p % 15 == 0 || p == 0) std::cout << std::setw(5) << std::setfill('0') << (p == 0 ? p : p - 16) << " ";
    std::cout << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << int(read_byte_no_io(p)) << " ";
    if (p % 15 == 0 && p != 0) std::cout << std::endl;
  }
}
