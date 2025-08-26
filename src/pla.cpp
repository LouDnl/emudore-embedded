/*
 * MOS Programmable Logic Array (PLA)
 * Copyright (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * pla.cpp
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

#include <c64.h>


/**
 * @brief: Setup must happen _after_ memory
 */
PLA::PLA(C64 * c64) :
  c64_(c64)
{
  havecart = c64_->havecart;
  logplabank = c64_->bankswlog;

  /* configure memory layout */
  if (!havecart) { /* default mode 31, all bits high */
    setup_memory_banks(kLORAM|kHIRAM|kCHARGEN|kGAME|kEXROM);
  } else { /* cart depends on cart type */
    if (c64_->cart_->cartactive) {
      setup_memory_banks(c64_->cart_->banksetup);
    } else {
      /* Fallback settings */
      setup_memory_banks(kLORAM|kHIRAM|kCHARGEN);
    }
  }
  /* configure data directional bits to default boot setting */
  c64_->memory()->write_byte_no_io(Memory::kAddrDataDirection, data_direction_default);

  D("[EMU] PLA initialized.\n");
}

PLA::~PLA()
{

}

/**
  * Example:
  * PLA latch bits: 11111 ~ 54321
  * kLORAM   ~ 1
  * kHIRAM   ~ 2
  * kCHARGEN ~ 3
  * kGAME    ~ 4
  * kEXROM   ~ 5
  *
  * banks_[] variable gets set up with the values
  * from kBankCfg referenced by they bit location
  *
  * In mode 31 (default) all bits are high (1)
  * and setting this mode equals:
  * kEXROM,kGAME,kCHARGEN,kHIRAM,kLORAM
  *      1,    1,       1,     1,     1
  *
  * In mode 20 bits are 10100 this corresponds to:
  * kEXROM,      kCHARGEN
  *      1,    0,       1,     0,     0
  *
  * So banks_[]
  * [kBankRam0,kBankRam1,kBankCart,kBankBasic,kBankRam2,kBankChargen,kBankKernal]
  * will look like this for mode 32:
  * [1,1,1,0,1,2,0]
  * and for mode 20:
  * [1,-1,3,-1,-1,2,4]
  */
void PLA::switch_banks(uint8_t v)
{
  /* Setup bank mode on boot */
  switch ((v&0x1F)) { /* Masked to check only available bits */
    case m31:
      banks_[kBankRam0]    = kRAM; /* RAM     ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM     ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM     ~ 0x8000 */
      banks_[kBankBasic]   = kROM; /* BASIC   ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM     ~ 0xc000 */
      banks_[kBankChargen] = kIO;  /* IO      ~ 0xd000 */
      banks_[kBankKernal]  = kROM; /* KERNAL  ~ 0xe000 */
      break;
    case m30: /* fallthrough ~ same settings */
    case m14:
      banks_[kBankRam0]    = kRAM; /* RAM     ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM     ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM     ~ 0x8000 */
      banks_[kBankBasic]   = kRAM; /* RAM     ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM     ~ 0xc000 */
      banks_[kBankChargen] = kIO;  /* IO      ~ 0xd000 */
      banks_[kBankKernal]  = kROM; /* KERNAL  ~ 0xe000 */
      break;
    case m29: /* fallthrough ~ same settings */
    case m13:
      banks_[kBankRam0]    = kRAM; /* RAM     ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM     ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM     ~ 0x8000 */
      banks_[kBankBasic]   = kRAM; /* RAM     ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM     ~ 0xc000 */
      banks_[kBankChargen] = kIO;  /* IO      ~ 0xd000 */
      banks_[kBankKernal]  = kRAM; /* RAM     ~ 0xe000 */
      break;
    case m28: /* fallthrough ~ same settings */
    case m24:
      banks_[kBankRam0]    = kRAM; /* RAM     ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM     ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM     ~ 0x8000 */
      banks_[kBankBasic]   = kRAM; /* RAM     ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM     ~ 0xc000 */
      banks_[kBankChargen] = kRAM; /* RAM     ~ 0xd000 */
      banks_[kBankKernal]  = kRAM; /* RAM     ~ 0xe000 */
      break;
    case m27:
      banks_[kBankRam0]    = kRAM; /* RAM     ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM     ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM     ~ 0x8000 */
      banks_[kBankBasic]   = kROM; /* BASIC   ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM     ~ 0xc000 */
      banks_[kBankChargen] = kROM; /* CHARGEN ~ 0xd000 */
      banks_[kBankKernal]  = kROM; /* KERNAL  ~ 0xe000 */
      break;
    case m26: /* fallthrough ~ same settings */
    case m10:
      banks_[kBankRam0]    = kRAM; /* RAM     ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM     ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM     ~ 0x8000 */
      banks_[kBankBasic]   = kRAM; /* RAM     ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM     ~ 0xc000 */
      banks_[kBankChargen] = kROM; /* CHARGEN ~ 0xd000 */
      banks_[kBankKernal]  = kROM; /* KERNAL  ~ 0xe000 */
      break;
    case m25: /* fallthrough ~ same settings */
    case m09:
      banks_[kBankRam0]    = kRAM; /* RAM     ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM     ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM     ~ 0x8000 */
      banks_[kBankBasic]   = kRAM; /* RAM     ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM     ~ 0xc000 */
      banks_[kBankChargen] = kROM; /* CHARGEN ~ 0xd000 */
      banks_[kBankKernal]  = kRAM; /* KERNAL  ~ 0xe000 */
      break;
    case m23: /* fallthrough ~ same settings */
    case m22:
    case m21:
    case m20:
    case m19:
    case m18:
    case m17:
    case m16:
      banks_[kBankRam0]    = kRAM; /* RAM      ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kUNM; /* UNMAPPED ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kCLO; /* CART LO  ~ 0x8000 */
      banks_[kBankBasic]   = kUNM; /* UNMAPPED ~ 0xa000 */
      banks_[kBankRam2]    = kUNM; /* UNMAPPED ~ 0xc000 */
      banks_[kBankChargen] = kIO;  /* IO       ~ 0xd000 */
      banks_[kBankKernal]  = kCHI; /* CART HI  ~ 0xe000 */
      break;
    case m15:
      banks_[kBankRam0]    = kRAM; /* RAM      ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM      ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kCLO; /* CART LO  ~ 0x8000 */
      banks_[kBankBasic]   = kROM; /* BASIC    ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM      ~ 0xc000 */
      banks_[kBankChargen] = kIO;  /* IO       ~ 0xd000 */
      banks_[kBankKernal]  = kROM; /* KERNAL   ~ 0xe000 */
      break;
    case m12: /* fallthrough ~ same settings */
    case m08:
    case m04:
    case m00:
      banks_[kBankRam0]    = kRAM; /* RAM      ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM      ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM      ~ 0x8000 */
      banks_[kBankBasic]   = kRAM; /* RAM      ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM      ~ 0xc000 */
      banks_[kBankChargen] = kRAM; /* RAM      ~ 0xd000 */
      banks_[kBankKernal]  = kRAM; /* RAM      ~ 0xe000 */
      break;
    case m11:
      banks_[kBankRam0]    = kRAM; /* RAM      ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM      ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kCLO; /* CART LO  ~ 0x8000 */
      banks_[kBankBasic]   = kROM; /* BASIC    ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM      ~ 0xc000 */
      banks_[kBankChargen] = kROM; /* CHARGEN  ~ 0xd000 */
      banks_[kBankKernal]  = kROM; /* KERNAL   ~ 0xe000 */
      break;
    case m07:
      banks_[kBankRam0]    = kRAM; /* RAM      ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM      ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kCLO; /* CART LO  ~ 0x8000 */
      banks_[kBankBasic]   = kCHI; /* CART HI  ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM      ~ 0xc000 */
      banks_[kBankChargen] = kIO;  /* IO       ~ 0xd000 */
      banks_[kBankKernal]  = kROM; /* KERNAL   ~ 0xe000 */
      break;
    case m06:
      banks_[kBankRam0]    = kRAM; /* RAM      ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM      ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM      ~ 0x8000 */
      banks_[kBankBasic]   = kCHI; /* CART HI  ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM      ~ 0xc000 */
      banks_[kBankChargen] = kIO;  /* IO       ~ 0xd000 */
      banks_[kBankKernal]  = kROM; /* KERNAL   ~ 0xe000 */
      break;
    case m05:
      banks_[kBankRam0]    = kRAM; /* RAM      ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM      ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM      ~ 0x8000 */
      banks_[kBankBasic]   = kRAM; /* RAM      ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM      ~ 0xc000 */
      banks_[kBankChargen] = kIO;  /* IO       ~ 0xd000 */
      banks_[kBankKernal]  = kRAM; /* RAM      ~ 0xe000 */
      break;
    case m03:
      banks_[kBankRam0]    = kRAM; /* RAM      ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM      ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kCLO; /* CART LO  ~ 0x8000 */
      banks_[kBankBasic]   = kCHI; /* CART HI  ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM      ~ 0xc000 */
      banks_[kBankChargen] = kROM; /* CHARGEN  ~ 0xd000 */
      banks_[kBankKernal]  = kROM; /* KERNAL   ~ 0xe000 */
      break;
    case m02:
      banks_[kBankRam0]    = kRAM; /* RAM      ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM      ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM      ~ 0x8000 */
      banks_[kBankBasic]   = kCHI; /* CART HI  ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM      ~ 0xc000 */
      banks_[kBankChargen] = kROM; /* CHARGEN  ~ 0xd000 */
      banks_[kBankKernal]  = kROM; /* KERNAL   ~ 0xe000 */
      break;
    case m01:
      banks_[kBankRam0]    = kRAM; /* RAM      ~ 0x0000 - Unchangeable */
      banks_[kBankRam1]    = kRAM; /* RAM      ~ 0x1000 - Gets unmapped if in Ultimax Mode */
      banks_[kBankCart]    = kRAM; /* RAM      ~ 0x8000 */
      banks_[kBankBasic]   = kRAM; /* RAM      ~ 0xa000 */
      banks_[kBankRam2]    = kRAM; /* RAM      ~ 0xc000 */
      banks_[kBankChargen] = kRAM; /* RAM      ~ 0xd000 */
      banks_[kBankKernal]  = kRAM; /* RAM      ~ 0xe000 */
      break;
    default:
      D("[PLA] Unmapped bank switch mode from %02X to %02X %02X requested\n",
        c64_->mem_->read_byte_no_io(Memory::kAddrMemoryLayout),
        v, (v&0x1f));
      break;
  }
}

/**
 * @brief configure memory banks on boot and PLA reset
 *
 * There are five latch bits that control the configuration allowing
 * for a total of 32 different memory layouts, for now we only take
 * in count three bits : HIRAM/LORAM/CHAREN
 */
void PLA::setup_memory_banks(uint8_t v) // TODO: If havecart enable cartrom etc!
{
  /* load ROMs */
  if (!l_basic) { /* Only load if not already loaded */
    if (c64_->mem_->load_rom("roms/basic.901226-01.bin",Memory::kBaseAddrBasic)) {
    l_basic = true; /* True for success / loaded */
    } else { l_basic = false; } /* False for failed / not loaded */
  }
  if (!l_chargen) { /* Only load if not already loaded */
    if (c64_->mem_->load_rom("roms/characters.901225-01.bin",Memory::kBaseAddrChars)) {
      l_chargen = true; /* True for success / loaded */
    } else { l_chargen = false; } /* False for failed / not loaded */
  }
  if (!l_kernal) { /* Only load if not already loaded */
    if (c64_->mem_->load_rom("roms/kernal.901227-03.bin",Memory::kBaseAddrKernal)) {
      l_kernal = true; /* True for success / loaded */
    } else { l_kernal = false; } /* False for failed / not loaded */
  }

  /* start with mode 0 and set everything to ram */
  for(size_t i=0 ; i < sizeof(banks_) ; i++) {
    banks_[i] = kRAM;
  }
  /* Switch banks on boot */
  banks_at_boot = v;
  switch_banks(banks_at_boot);
  if(logplabank) {
    D("Bank setup @ boot to: %02X\n",banks_at_boot);
    logbanksetup();
  }
  /* write the raw value to the zero page (doesn't influence the cart bits) */
  c64_->memory()->write_byte_no_io(Memory::kAddrMemoryLayout, v);
}

/**
 * @brief configure memory banks during runtime, limited to 3 cpu latches
 *
 * During runtime the system can only change the cpu latches at bit 0,1 and 2
 */
void PLA::runtime_bank_switching(uint8_t v) /* TODO: UPDATE TO NEW MODE SWITCHING WAY */
{
  /* Setup bank mode during runtime */

  uint8_t b = banks_at_boot; /* Use boot time state as preset */
  b &= (0x18|(v&0x7)); /* Preserve _cart bits_ and only set cpu latches */
  if(logplabank){
    D("[PLA] Bank switch @ runtime from %02X to: %02X\n",banks_at_boot,b);
    logbanksetup();
  }
  switch_banks(b);

  /* write the raw value to the zero page (doesn't influence the cart bits) */
  c64_->mem_->write_byte_no_io(Memory::kAddrMemoryLayout, v);
}

/* Not working, broken and unfinished */
void PLA::c1541(void) /* TODO: Move elsewhere, e.g. iec.cpp? */
{
  return; /* Disabled */
  disk = new uint8_t[disksize]();
  DISKptr = &disk[0];

  // memset(mem_ram_, 127, sizeof(mem_ram_)/sizeof(mem_ram_[0]));
  int chunk_size = 64;

  // for (int i = 0; i < kMemSize; i += chunk_size) {
  //   memset(mem_ram_ + i, 0x00, chunk_size);
  //   D("%04X 00\n", i);
  //   if (i + chunk_size < kMemSize) {
  //     memset(mem_ram_ + i + chunk_size, 0xFF, chunk_size);
  //     D("%04X FF\n", i+chunk_size);
  //   }
  // }
}

void PLA::write_pla(uint16_t addr, uint8_t v)
{
   /* Unused */
}

uint8_t PLA::read_pla(uint16_t addr)
{ /* Unused */
  uint8_t retval = 0;
  return retval;
}

/**
 * @brief When used will reset the bank configuration to default
 */
void PLA::reset(void)
{
  setup_memory_banks(kLORAM|kHIRAM|kCHARGEN|kGAME|kEXROM);
}

void PLA::emulate(void)
{
  /* Unused */
}

/* Debug functions */
void PLA::logbanksetup(void)
{
  D("Rm Rm Ct Bc Rm Cn Kl\n%2X %2X %2X %2X %2X %2X %2X\n",
    memory_banks(kBankRam0),
    memory_banks(kBankRam1),
    memory_banks(kBankCart),
    memory_banks(kBankBasic),
    memory_banks(kBankRam2),
    memory_banks(kBankChargen),
    memory_banks(kBankKernal)
  );
}
