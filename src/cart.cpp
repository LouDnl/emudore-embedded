/*
 * Cartridge expansion port
 * Copyright (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * cart.cpp
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

#include <cstring>
#include <fstream>

#include <c64.h>
#include <MC68B50.h>

char *cartfile;
std::ifstream is_;


/**
 * @brief: Setup must happen _after_ memory
 */
Cart::Cart(C64 * c64) :
  c64_(c64)
{
  cartactive = c64_->havecart;
  midi = c64_->acia;

  /* enable mc68B50 if requested */
  if (midi && !acia_active) {
    /* init_mc6850(); */
    mc6850_ = new MC68B50(c64_);
    acia_active = true;
  }

  if (cartactive) {
    load_crt(c64_->cartfile);
  }

  D("[EMU] Cart initialized.\n");
}

Cart::~Cart()
{
  if (acia_active) { delete mc6850_; }
}

void Cart::deinit_cart(void)
{ /* RUN/STOP+RESTORE resets cart contents! or maybe not? */
  if (acia_active) { delete mc6850_; }
  acia_active = false;
  midi = false;
  cartactive = false;
}

void Cart::reset(void)
{
  deinit_cart();
}

bool Cart::emulate(void)
{
  if (acia_active) {
    mc6850_->emulate();
  }
  return true;
}

void Cart::write_register(uint8_t r, uint8_t v)
{
  if (acia_active) {
    mc6850_->write_register(r,v);
  }
}

uint8_t Cart::read_register(uint8_t r)
{
  uint8_t retval = 0;
  if (acia_active) {
    retval = mc6850_->read_register(r);
  }
  return retval;
}


// CRT //////////////////////////////////////////////////////////////////////

uint16_t Cart::read_short_le()
{
  char b;
  uint16_t v = 0;
  is_.get(b);
  v |= (b);
  is_.get(b);
  v |= b << 8;
  return v;
}

uint16_t Cart::read_short_be()
{
  char b;
  uint16_t v = 0;
  is_.get(b);
  v |= (b << 8);
  is_.get(b);
  v |= (b);
  return v;
}

void Cart::load_crt(const std::string &f)
{
  is_.open(f,std::ios::in|std::ios::binary);
  Cart::crt_header_t cart_header;
  Cart::crt_chip_t   chip_header;
  char b;
  uint16_t cbuf;
  if (is_.is_open()) {
    is_.seekg(0, is_.end);
    std::streamoff length = is_.tellg();
    is_.seekg(0, is_.beg);
    D("CART SIZE: %ld\n",length);
    uint8_t * cart_sig = new uint8_t[16]();
    is_.read((char *) &cart_sig[0],cart_header.signature_l);
    int n = memcmp(cart_sig, cart_header.signature_id_s, 16);
    D("Equal (0)? %d\n",n);
    delete [] cart_sig;
    is_.seekg(cart_header.version);
    uint16_t v = read_short_be();
    std::streamoff pos = is_.tellg();
    D("VERSION? %d POS: %ld\n",v,pos);
    is_.seekg(cart_header.hardware_type);
    uint16_t h = read_short_be();
    pos = is_.tellg();
    D("HARDWARE TYPE? %d IN SPEC? %d POS: %ld\n",h,(h<=27),pos);
    is_.get(b);
    uint8_t e = b;
    is_.get(b);
    uint8_t g = b;
    D("EXROM? %d GAME %d\n",e,g);
    // is_.seekg(0, is_.beg);
    /* Count number of Chip ROMS */
    size_t n_chips = 0;
    size_t offset = 0;
    is_.seekg(cart_header.cart_data);
    uint8_t * chip_sig = new uint8_t[4]();
CHIP:;
    is_.read((char *) &chip_sig[0],chip_header.signature_l);
    int c = memcmp(chip_sig, chip_header.signature_id_s, 4);
    pos = is_.tellg();
    is_.seekg(pos-4);
    pos = is_.tellg();
    /* D("POS IS: %ld C IS: %d\n",pos,c); */
NOCHIP:;
    if(c==0) { /* Chips = 1 */
      n_chips++;
      /* D("Chip (0)? %d No. chips: %lu\n",c,n_chips); */
      is_.seekg(cart_header.cart_data+(0x2010*n_chips));
      // is_.seekg(cart_header.cart_data+(n_chips*0x10));
      pos = is_.tellg();
      if (pos < length) {
        goto CHIP;
      } else {
        c = -1;
        goto NOCHIP;
      }
    } else {
      delete [] chip_sig;
      D("No. chips found: %lu\n",n_chips);
      std::streamoff romstart = is_.tellg();
      D("ROMS start at $%04X\n",(uint16_t)romstart);
    }
    /* Get chip data */
    is_.seekg(cart_header.cart_data);
    // cart_chip_t chips[n_chips]; /* Must be public to switch bank into */
    chips = new Cart::cart_chip_t[n_chips](); /* Must be public to switch bank into */
    for (size_t n = 0; n < n_chips; n++) {
      D("CHIP%lu\n",n+1);
      is_.get(b);
      chips[n].signature |= (b<<24);
      is_.get(b);
      chips[n].signature |= (b<<16);
      is_.get(b);
      chips[n].signature |= (b<<8);
      is_.get(b);
      chips[n].signature |= (b);
      D("SIGNATURE: 0x%X\n",chips[n].signature);
      is_.get(b);
      chips[n].packetlength |= (b<<24);
      is_.get(b);
      chips[n].packetlength |= (b<<16);
      is_.get(b);
      chips[n].packetlength |= (b<<8);
      is_.get(b);
      chips[n].packetlength |= (b);
      D("PACKETLENGTH: 0x%X %u\n",chips[n].packetlength,chips[n].packetlength);
      is_.get(b);
      chips[n].chiptype |= (b<<8);
      is_.get(b);
      chips[n].chiptype |= (b);
      D("CHIPTYPE: 0x%X %u\n",chips[n].chiptype,chips[n].chiptype);
      is_.get(b);
      chips[n].banknumber |= (b<<8);
      is_.get(b);
      chips[n].banknumber |= (b);
      D("BANKNUMBER: 0x%X %u\n",chips[n].banknumber,chips[n].banknumber);
      is_.get(b);
      chips[n].addr |= (b<<8);
      is_.get(b);
      chips[n].addr |= (b);
      D("START LOAD ADDRESS: 0x%X %u\n",chips[n].addr,chips[n].addr);
      is_.get(b);
      chips[n].romsize |= (b<<8);
      is_.get(b);
      chips[n].romsize |= (b);
      D("ROM SIZE 0x%X %u\n",chips[n].romsize,chips[n].romsize);
      /* Needed!? */
      chips[n].rom = new uint8_t[chips[n].romsize]();
      is_.read ((char *) &chips[n].rom[0],chips[n].romsize);
      // c64_->mem_->kCARTRomLo = &chips[n].rom[0];
      /* D("kCARTRomLo @ 0x%x\n",&c64_->mem_->kCARTRomLo); */
    }

    /* Start with default */
    banksetup = (PLA::kEXROM|PLA::kGAME|PLA::kCHARGEN|PLA::kHIRAM|PLA::kLORAM);
    /* Set game bit to 0 if linestate byte = 0 */
    if (g == 0) {banksetup &= ~(PLA::kGAME);};
    /* Set exrom bit to 0 if linestate byte = 0 */
    if (e == 0) {banksetup &= ~(PLA::kEXROM);};
    /* Banksetup will be set from PLA() now */

    // c64_->pla_->setup_memory_banks(banksetup);
    // uint8_t banksetup_curr = c64_->mem_->read_byte_no_io(Memory::kAddrMemoryLayout);
    // uint8_t banksetup = (e<<PLA::kCHI|g<<PLA::kCLO|)
    // c64_->pla_->setup_memory_banks();

    // TODO: Set rom locations based on chips on cart!
    // TODO: Move this to PLA for bank switching!?
    c64_->mem_->kCARTRomLo  = &chips[0].rom[0];
    D("4BYTES ROMLO: $%02X $%02X $%02X $%02X\n",
      c64_->mem_->kCARTRomLo[0],
      c64_->mem_->kCARTRomLo[1],
      c64_->mem_->kCARTRomLo[2],
      c64_->mem_->kCARTRomLo[3]
    );
    // if (n_chips > 1) {
    //   c64_->mem_->kCARTRomHi1 = &chips[1].rom[0];
    //   /* c64_->mem_->kCARTRomHi2 = &chips[1].rom[0]; */
    //   D("4BYTES ROMHI1: $%02X $%02X $%02X $%02X\n",
    //     c64_->mem_->kCARTRomHi1[0],
    //     c64_->mem_->kCARTRomHi1[1],
    //     c64_->mem_->kCARTRomHi1[2],
    //     c64_->mem_->kCARTRomHi1[3]
    //   );
    // }
    if (n_chips > 1) {
      c64_->mem_->kCARTRomHi2 = &chips[1].rom[0];
      D("4BYTES ROMHI2: $%02X $%02X $%02X $%02X\n",
        c64_->mem_->kCARTRomHi2[0],
        c64_->mem_->kCARTRomHi2[1],
        c64_->mem_->kCARTRomHi2[2],
        c64_->mem_->kCARTRomHi2[3]
      );
    }

    // D("4BYTES: $%02X $%02X $%02X $%02X\n",
    //   chips[0].rom[0],
    //   chips[0].rom[1],
    //   chips[0].rom[2],
    //   chips[0].rom[3]
    // );

    /* TODO: Fix jump address per cart */
    c64_->cpu_->pc(chips[0].addr);

    // D("4BYTES: $%02X $%02X $%02X $%02X\n",
    //   c64_->memory()->kCARTRomLo[0],
    //   c64_->memory()->kCARTRomLo[1],
    //   c64_->memory()->kCARTRomLo[2],
    //   c64_->memory()->kCARTRomLo[3]
    // );
    is_.close();
  }
}
