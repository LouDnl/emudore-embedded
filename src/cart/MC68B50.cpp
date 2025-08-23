/*
 * Motorola 68B50 ACIA emulation
 * (for Midi and or Uart communication)
 * Copyright (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * MC68B50.cpp
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
#include <MC68B50.h>
#include <util.h>


/**
 * @brief Constructor setup must happen _after_ memory
 * @param C64 * c64 a pointer to the main object
 * @return MC68B50 object
 */
MC68B50::MC68B50(C64 * c64)
  : c64_(c64)
{
  /* TODO: Need to set TDRE to STATUS here
   * but after memory pointer initalization */
}

/**
 * @brief resets the MC6850 STATUS register
 */
void MC68B50::reset()
{
  /* ISSUE: Calling this from anywhere else then
   * from `write_register` seems to break rules
   * and causes a segmentation fault */
  c64_->cart_->k6850MemRd[STATUS] = TDRE; /* Set TDRE default to empty */
}

/**
 * @brief Handle Midi keyboard key down event
 */
void MC68B50::fake_keydown(void)
{
  if (!_keydown && _keyup/*  && n_read >= 3 */) {
    D("[MC68B50] insert->keydown\n"); /* Temporary workaround hack using insert */
    _keydown = true;
    _keyup = false;

    c64_->cart_->k6850MemRd[STATUS] |= (IRQ|RDRF); /* Set IRQ and RDRF */
    // uint8_t r = read_register(STATUS);
    // write_register(STATUS, (r |= (IRQ|RDRF)));
    // uint8_t r = c64_->cart_->read_byte(0xde00|STATUS);
    // c64_->cart_->write_byte((0xde00|STATUS), (r |= (IRQ|RDRF)));

    if (n_read >= 3) {
      if (midi_keydown[1] < 127) midi_keydown[1] += 2; /* Two notes up */
      else midi_keydown[1] = 31; /* Start again at 0x1F */
    }
    n_read = 0;
  }
}

/**
 * @brief Handle Midi keyboard key up event
 */
void MC68B50::fake_keyup(void)
{
  if (!_keyup && _keydown/*  && n_read >= 3 */) {
    D("[MC68B50] end->keyup\n"); /* Temporary workaround hack using end */
    _keyup = true;
    _keydown = false;

    c64_->cart_->k6850MemRd[STATUS] |= (IRQ|RDRF); /* Set IRQ and RDRF */
    if (n_read >= 3) {
      if (midi_keyup[1] < 127) midi_keyup[1] += 2; /* Two notes up */
      else midi_keyup[1] = 31; /* Start again at 0x1F */
    }
    n_read = 0;
  }
}

/**
 * @brief reads a byte from MC6850 readable registers
 * @param uint8_t r the register to read
 * @return uint8_t the registers value
 * configured for Datel/Kerberos
 * Registers for other brands can be at different
 * addresses depending on the type of Cart
 */
uint8_t MC68B50::read_register(uint8_t r)
{
  uint8_t retval = 0;
  switch (r) {
    case CONTROL: /* $de04 ~ control register ~ write only */
      break;
    case STATUS:  /* $de06 ~ status register  ~ read only */
      retval = c64_->cart_->k6850MemRd[r];
      break;
    case TXDR:    /* $de05 ~ TX register      ~ write only */
      break;;
    /* NOTE: Data in the RXDR should come from a ringbuffer that shifts after reading */
    case RXDR:    /* $de07 ~ RX register      ~ read only */
      c64_->cart_->k6850MemRd[STATUS] &= ~(IRQ|RDRF); /* Clear IRQ and RDRF on read (if set) */
      if (n_read < 3) { /* NOTE: This is fake to emulate Midi input */
        if (_keydown) c64_->cart_->k6850MemRd[RXDR] = midi_keydown[n_read++];
        if (_keyup) c64_->cart_->k6850MemRd[RXDR] = midi_keyup[n_read++];
        c64_->cart_->k6850MemRd[STATUS] |= (IRQ|RDRF); /* Set IRQ and RDRF for data available */
        retval = c64_->cart_->k6850MemRd[RXDR];
      }
      if (n_read >= 3)
      {
        c64_->cart_->k6850MemRd[STATUS] &= ~(IRQ|RDRF); /* Clear IRQ and RDRF on read (if set) */
      }
      break;
    default:      /* default always return read */
      retval = c64_->cart_->k6850MemRd[r];
      break;
  }

  return retval;
}

/**
 * @brief Write a byte to MC6850 writeable registers
 * @param uint8_t r the register to write to
 * @param uint8_t v the value to write
 * configured for Datel/Kerberos
 * Registers for other brands can be at different
 * addresses depending on the type of Cart
 */
void MC68B50::write_register(uint8_t r, uint8_t v)
{
  uint8_t cr = 0; /* control register masked value */
  switch (r) {
    case CONTROL: /* $de04 ~ control register ~ write only */
      /* Save value to write ram */
      c64_->cart_->k6850MemWr[r] = v;
      cr = (v & CR0CR1SEL); /* Counter divide select bits */
      switch (cr) {
        case R1: /* set divide ratio +1 */
          D("[MC68B50] divide ratio +1\n");
          break;
        case R16: /* set divide ratio +16 */
          D("[MC68B50] divide ratio +16\n");
          break;
        case R64: /* set divide ratio +64 */
          D("[MC68B50] divide ratio +64\n");
          break;
        case RES: /* reset midi device */
          D("[MC68B50] Master reset!\n");
          this->reset();
          break;
        default: break;
      }
      if (cr == RES) {return;}; /* Return here if Master reset */
      cr = ((v & WORDSEL) >> 2); /* Word select bits */
      switch (cr) {
        case w7e2:
          D("[MC68B50] 7 Bits + Even Parity + 2 Stop Bits\n");
          break; /* Unused for CynthCart */
        case w7o2:
          D("[MC68B50] 7 Bits + Odd Parity + 2 Stop Bits\n");
          break; /* Unused for CynthCart */
        case w7e1:
          D("[MC68B50] 7 Bits + Even Parity + 1 Stop Bit\n");
          break; /* Unused for CynthCart */
        case w7o1:
          D("[MC68B50] 7 Bits + Odd Parity + 1 Stop Bit\n");
          break; /* Unused for CynthCart */
        case w8n2:
          D("[MC68B50] 8 Bits + 2 Stop Bits\n");
          break; /* Unused for CynthCart */
        case w8n1: /* 8 Bits+1 Stop Bit */
          D("[MC68B50] 8 Bits+1 Stop Bit\n");
          break;
        case w8e1:
          D("[MC68B50] 8 Bits + Even Parity + 1 Stop Bit\n");
          break; /* Unused for CynthCart */
        case w8o1:
          D("[MC68B50] 8 Bits + Odd Parity + 1 Stop Bit\n");
          break; /* Unused for CynthCart */
        default: break;
      }
      cr = ((v & TCCTR) >> 5);   /* Transmit control bits */
      switch (cr) {
        case RTSloTID: /* RTS=low, Transmitting Interrupt Disabled. */
          D("[MC68B50] RTS=low, Transmitting Interrupt Disabled\n");
          break;
        case RTSloTIE: /* RTS=low, Transmitting Interrupt Enabled. */
          D("[MC68B50] RTS=low, Transmitting Interrupt Enabled\n");
          break;
        case RTShiTID: /* RTS=high, Transmitting Interrupt Disabled */
          D("[MC68B50] RTS=high, Transmitting Interrupt Disabled\n");
          break;
        case RTSloTRB: /* RTS=low, Transmits a Break level on the Transmit Data Output. Transmitting Interrupt Disabled. */
          D("[MC68B50] RTS=low, Transmits a Break level on the Transmit Data Output. Transmitting Interrupt Disabled\n");
          break;
        default: break;
      }
      cr = ((v & INTEN) >> 7);   /* Receive interrupt enable bit */
      switch (cr) {
        case 0b0: /* Receive interrupt disabled */
          D("[MC68B50] Receive interrupt disabled\n");
          break;
        case 0b1: /* Receive interrupt enabled */
          D("[MC68B50] Receive interrupt enabled\n");
          break;
        default: break;
      }
      return;
    case STATUS:  /* $de06 ~ status register  ~ read only */
      return;
    case TXDR:    /* $de05 ~ TX register      ~ write only */
      c64_->cart_->k6850MemWr[r] = v; /* Save value to ram */
      return;
    case RXDR:    /* $de07 ~ RX register      ~ read only */
      return;
    default:      /* default always write to read and write */
      c64_->cart_->k6850MemWr[r] = c64_->cart_->k6850MemRd[r] = v;
      return;
  }
}

/**
 * @brief emulate triggers a CPU IRQ if no CPU IRW
 * is presently active
 * Other brands then Datel/Kerberos might require
 * a NMI to trigger, this is not supported yet
 */
void MC68B50::emulate()
{
  if(!c64_->cpu_->idf()) { /* If no CPU IRQ already in place */
    if (c64_->cart_->k6850MemRd[STATUS] & IRQ) { /* If IRQ is set */
      c64_->cpu_->irq(); /* Trigger CPU IRQ */
    }
  }
}
