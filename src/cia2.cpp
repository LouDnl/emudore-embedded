/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * cia2.cpp
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

#include <c64.h> /* All classes are loaded through c64.h */


enum CIA2Registers
{
  PRA     = 0x0, /* Keyboard (R/W), Joystick, Lightpen, Paddles */
  PRB     = 0x1, /* Keyboard (R/W), Joystick, Timer A, Timer B */
  DDRA    = 0x2, /* Datadirection Port A */
  DDRB    = 0x3, /* Datadirection Port B */
  TAL     = 0x4, /* Timer A Low Byte */
  TAH     = 0x5, /* Timer A High Byte */
  TBL     = 0x6, /* Timer B Low Byte */
  TBH     = 0x7, /* Timer A High Byte */
  TOD_TEN = 0x8, /* RTC 1/10s */
  TOD_SEC = 0x9, /* RTC sec */
  TOD_MIN = 0xA, /* RTC min */
  TOD_HR  = 0xB, /* RTC hr */
  SDR     = 0xC, /* Serial shift register */
  ICR     = 0xD, /* Interrupt control register */
  CRA     = 0xE, /* Control Timer A */
  CRB     = 0xF, /* Control Timer B */
};

enum InterruptBitVal
{ /* (Read or Write operation determines which one:) */
  INTERRUPT_HAPPENED = 0x80,
  SET_OR_CLEAR_FLAGS = 0x80,
  /* flags/masks of interrupt-sources */
  FLAGn      = 0x10,
  SERIALPORT = 0x08,
  ALARM      = 0x04,
  TIMERB     = 0x02,
  TIMERA     = 0x01
};

 enum ControlAbitVal
 {
  ENABLE_TIMERA        = 0x01,
  PORTB6_TIMERA        = 0x02,
  TOGGLED_PORTB6       = 0x04,
  ONESHOT_TIMERA       = 0x08,
  FORCELOADA_STROBE    = 0x10,
  TIMERA_FROM_CNT      = 0x20,
  SERIALPORT_IS_OUTPUT = 0x40,
  TIMEOFDAY_50Hz       = 0x80
};

enum ControlBbitVal
{
  ENABLE_TIMERB              = 0x01,
  PORTB7_TIMERB              = 0x02,
  TOGGLED_PORTB7             = 0x04,
  ONESHOT_TIMERB             = 0x08,
  FORCELOADB_STROBE          = 0x10,
  TIMERB_FROM_CPUCLK         = 0x00,
  TIMERB_FROM_CNT            = 0x20,
  TIMERB_FROM_TIMERA         = 0x40,
  TIMERB_FROM_TIMERA_AND_CNT = 0x60,
  TIMEOFDAY_WRITE_SETS_ALARM = 0x80
};

// ctor  /////////////////////////////////////////////////////////////////////

Cia2::Cia2(C64 *c64) :
  c64_(c64)
{ /* 0xDD00 */
  /* Base */
  prev_cpu_cycles_ = 0;
}

void Cia2::reset()
{
  prev_cpu_cycles_ = 0;

  for (uint i = 2; i < 0x10; i++) { /* Make sure register 0->16 are zeroed out */
    c64_->mem_->write_byte_no_io(c64_->mem_->kCIA2MemRd[i],0x00);
    c64_->mem_->write_byte_no_io(c64_->mem_->kCIA2MemWr[i],0x00);
  }
  c64_->mem_->write_byte_no_io(c64_->mem_->kCIA2MemRd[PRA],0xFF); /* Rd */
  c64_->mem_->write_byte_no_io(c64_->mem_->kCIA2MemWr[PRA],0xFF); /* Wr */
  c64_->mem_->write_byte_no_io(c64_->mem_->kCIA2MemRd[PRB],0xFF); /* Rd */
  c64_->mem_->write_byte_no_io(c64_->mem_->kCIA2MemWr[PRB],0xFF); /* Wr */
}

// DMA register access  //////////////////////////////////////////////////////

void Cia2::write_register(uint8_t r, uint8_t v)
{
  // D("CIA2 WR $%02X: %02X\n",r,v);
  // if(r==0xD) D("[CIA2 W] $0xDC0D:%02X\n",v);
  switch(r)
  {
  /* data port a (0x0) */
  case PRA:
    c64_->mem_->kCIA2MemWr[PRA] = c64_->mem_->kCIA2MemRd[PRA] = v;
    break;
  /* data port b (0x1) */
  case PRB:
    c64_->mem_->kCIA2MemWr[PRB] = c64_->mem_->kCIA2MemRd[PRB] = v;
    break;
  /* data direction port a (0x2) */
  case DDRA:
    c64_->mem_->kCIA2MemWr[DDRA] = c64_->mem_->kCIA2MemRd[DDRA] = v;
    break;
  /* data direction port b (0x3) */
  case DDRB:
    c64_->mem_->kCIA2MemWr[DDRB] = c64_->mem_->kCIA2MemRd[DDRB] = v;
    break;
  /* timer a low byte (0x4) */
  case TAL:
    c64_->mem_->kCIA2MemWr[TAL] = v; /* Latch low byte */
    break;
  /* timer a high byte (0x5) */
  case TAH:
    c64_->mem_->kCIA2MemWr[TAH] = v; /* Latch high byte */
    break;
  /* timer b low byte (0x6) */
  case TBL:
    c64_->mem_->kCIA2MemWr[TBL] = v; /* Latch low byte */
    break;
  /* timer b high byte (0x7) */
  case TBH:
    c64_->mem_->kCIA2MemWr[TBH] = v; /* Latch high byte */
    break;
  /* RTC 1/10s (0x8) */
  case TOD_TEN:
    c64_->mem_->kCIA2MemWr[TOD_TEN] = c64_->mem_->kCIA2MemRd[TOD_TEN] = v;
    break;
  /* RTC seconds (0x9) */
  case TOD_SEC:
    c64_->mem_->kCIA2MemWr[TOD_SEC] = c64_->mem_->kCIA2MemRd[TOD_SEC] = v;
    break;
  /* RTC minutes (0xA) */
  case TOD_MIN:
    c64_->mem_->kCIA2MemWr[TOD_MIN] = c64_->mem_->kCIA2MemRd[TOD_MIN] = v;
    break;
  /* RTC hours (0xB) */
  case TOD_HR:
    c64_->mem_->kCIA2MemWr[TOD_HR] = c64_->mem_->kCIA2MemRd[TOD_HR] = v;
    break;
  /* shift serial (0xC) */
  case SDR:
    c64_->mem_->kCIA2MemWr[SDR] = c64_->mem_->kCIA2MemRd[SDR] = v;
    /* D("SDR WR: %X\n",v); */
    break;
  /* interrupt control and status (0xD) */
  case ICR:
    /**
     * if bit 7 is set, enable selected mask of
     * interrupts, else disable them
     */
    if (v&0x80)
      c64_->mem_->kCIA2MemWr[ICR] |= (v&0x1F);
    else
      c64_->mem_->kCIA2MemWr[ICR] &= ~(v&0x1F);
    // ISSUE: QUESTION: Should we set all bits in the read register high?!
    // Doesnt't this imply there is an IRQ?
    // c64_->mem_->kCIA2MemRd[ICR] = (c64_->mem_->kCIA2MemWr[ICR]&0x1F);
    break;
  /* control timer a (0xE) */
  /* control timer a (0xE) */
  case CRA:
    c64_->mem_->kCIA2MemWr[CRA] = c64_->mem_->kCIA2MemRd[CRA] = v; /* Write the data to the register */
    break;
  /* control timer b (0xF) */
  case CRB:
    c64_->mem_->kCIA2MemWr[CRB] = c64_->mem_->kCIA2MemRd[CRB] = v;
    break;
  }
}

uint8_t Cia2::read_register(uint8_t r)
{
  uint8_t retval = 0;
  static int sector = 1;
  static int bytesrd = 0;

  int track = 18; /* Directory listing at sector 18 */

  switch(r)
  {
  /* data port a (0x0), serial pla, RS232, NMI control */
  case PRA:
    retval = c64_->mem_->kCIA2MemRd[PRA] | ~(c64_->mem_->kCIA2MemRd[DDRA] & 0x3f);
    break;
  /* data port b (0x1), serial pla, RS232, NMI control */
  case PRB:
    retval = c64_->mem_->kCIA2MemRd[PRB] & c64_->mem_->kCIA2MemRd[DDRB];
    break;
  /* data direction port a (0x2) */
  case DDRA:
    break;
  /* data direction port b (0x3) */
  case DDRB:
    break;
  /* timer a low byte (0x4) */
  case TAL:
    retval = c64_->mem_->kCIA2MemRd[TAL];
    break;
  /* timer a high byte (0x5) */
  case TAH:
    retval = c64_->mem_->kCIA2MemRd[TAH];
    break;
  /* timer b low byte (0x6) */
  case TBL:
    retval = c64_->mem_->kCIA2MemRd[TBL];
    break;
  /* timer b high byte (0x7) */
  case TBH:
    retval = c64_->mem_->kCIA2MemRd[TBH];
    break;
  /* RTC 1/10s (0x8) */
  case TOD_TEN:
    retval = c64_->mem_->kCIA2MemRd[TOD_TEN];
    break;
  /* RTC seconds (0x9) */
  case TOD_SEC:
    retval = c64_->mem_->kCIA2MemRd[TOD_SEC];
    break;
  /* RTC minutes (0xA) */
  case TOD_MIN:
    retval = c64_->mem_->kCIA2MemRd[TOD_MIN];
    break;
  /* RTC hours (0xB) */
  case TOD_HR:
    retval = c64_->mem_->kCIA2MemRd[TOD_HR];
    break;
  /* shift serial (0xC) */
  case SDR:
    retval = c64_->mem_->kCIA2MemRd[SDR];
    // D("SDR RD: %X\n",retval);
    break;
  /* interrupt control and status (0xD) */
  case ICR:
    /* Reading from ICR clears the read register after reading */
    if (c64_->mem_->kCIA2MemRd[ICR] & INTERRUPT_HAPPENED) {
      retval |= (1<<7); /* IRQ triggered */
      if(c64_->mem_->kCIA2MemRd[ICR] & 1) retval |= 1; /* For timer A */
      if(c64_->mem_->kCIA2MemRd[ICR] & 2) retval |= 2; /* And/or timer B */
    }
    c64_->mem_->kCIA2MemRd[ICR] = 0; /* Clear read register */
    break;
  /* control timer a (0xE) */
  case CRA:
    retval = (c64_->mem_->kCIA2MemRd[CRA] & 0xee) | (c64_->mem_->kCIA2MemWr[CRA]&0x1);
    break;
  /* control timer b (0xF) */
  case CRB:
    retval = (c64_->mem_->kCIA2MemRd[CRB] & 0xee) | (c64_->mem_->kCIA2MemWr[CRB]&0x1);
    break;
  }
  return retval;
}

// VIC banking ///////////////////////////////////////////////////////////////

/**
 * @brief retrieves vic base address
 *
 * PRA bits (0..1)
 *
 *  %00, 0: Bank 3: $C000-$FFFF, 49152-65535
 *  %01, 1: Bank 2: $8000-$BFFF, 32768-49151
 *  %10, 2: Bank 1: $4000-$7FFF, 16384-32767
 *  %11, 3: Bank 0: $0000-$3FFF, 0-16383 (standard)
 */
uint16_t Cia2::vic_base_address()
{
  return ((~c64_->mem_->kCIA2MemRd[PRA]&0x3) << 14);
}

// emulation  ////////////////////////////////////////////////////////////////

bool Cia2::emulate()
{
  static int temp; /* Timer */

  /* Check for force load latch requested */
  if(c64_->mem_->kCIA2MemWr[CRA] & FORCELOADA_STROBE) {
    c64_->mem_->kCIA2MemRd[TAH] = c64_->mem_->kCIA2MemWr[TAH];
    c64_->mem_->kCIA2MemRd[TAL] = c64_->mem_->kCIA2MemWr[TAL];
  }
  else /* Check for timer A enabled, counts Phi2 */
  if ((c64_->mem_->kCIA2MemWr[CRA] & (ENABLE_TIMERA|TIMERA_FROM_CNT)) == ENABLE_TIMERA) { /* Check for count Phi2 enabled */
    temp = ((c64_->mem_->kCIA2MemRd[TAH]<<8) + c64_->mem_->kCIA2MemRd[TAL]) - (c64_->cpu_->cycles() - prev_cpu_cycles_); /* Update counter */
    if (temp <= 0) {  /* counter underflow */
      temp = (c64_->mem_->kCIA2MemWr[TAH]<<8) + c64_->mem_->kCIA2MemWr[TAL]; /* reload timer */
      if (c64_->mem_->kCIA2MemWr[CRA] & ONESHOT_TIMERA) { /* If one-shot is enabled */
        c64_->mem_->kCIA2MemWr[CRA] &= ~ENABLE_TIMERA; /* Disable timer A */
      }
      c64_->mem_->kCIA2MemRd[ICR] |= TIMERA; /* Set timer A in read ICR */
      if (c64_->mem_->kCIA2MemWr[ICR] & TIMERA) { /* Generate interrupt if write mask allows */
        c64_->mem_->kCIA2MemRd[ICR] |= INTERRUPT_HAPPENED; /* Set interrupt bit in read ICR */
        c64_->cpu_->irq(); /* Trigger interrupt */
      }
    }
    /* Set new timer A value */
    c64_->mem_->kCIA2MemRd[TAH] = (temp >> 8);
    c64_->mem_->kCIA2MemRd[TAL] = (temp & 0xFF);
  }
  c64_->mem_->kCIA2MemWr[CRA] &= ~FORCELOADA_STROBE;   /* Reset timer A strobe bit */
  c64_->mem_->kCIA2MemRd[CRA] = c64_->mem_->kCIA2MemWr[CRA]; /* Copy CRA write to read register */

  /* Check for force load latch requested */
  if (c64_->mem_->kCIA2MemWr[CRB] & FORCELOADB_STROBE) {
    c64_->mem_->kCIA2MemRd[TBH] = c64_->mem_->kCIA2MemWr[TBH];
    c64_->mem_->kCIA2MemRd[TBL] = c64_->mem_->kCIA2MemWr[TBL];
  }
  else /* Check for timer B enabled */
  if ((c64_->mem_->kCIA2MemWr[CRB] & (ENABLE_TIMERB|TIMERB_FROM_TIMERA)) == ENABLE_TIMERB) { /* Check for count Phi2 enabled */
    temp = ((c64_->mem_->kCIA2MemRd[TBH]<<8) + c64_->mem_->kCIA2MemRd[TBL] ) - (c64_->cpu_->cycles() - prev_cpu_cycles_); /* Update counter */
    if (temp <= 0) { //Timer counted down
      temp += (c64_->mem_->kCIA2MemWr[TBH]<<8) + c64_->mem_->kCIA2MemWr[TBL]; /* reload timer */
      if (c64_->mem_->kCIA2MemWr[CRB] & ONESHOT_TIMERB) { /* If one-shot is enabled */
         c64_->mem_->kCIA2MemWr[CRB] &= ~ENABLE_TIMERB; /* Disable timer A */
      }
      c64_->mem_->kCIA2MemRd[ICR] |= TIMERB; /* Set timer B in read ICR */
      if (c64_->mem_->kCIA2MemWr[ICR] & TIMERB) { /* Generate interrupt if write mask allows */
        c64_->mem_->kCIA2MemRd[ICR] |= INTERRUPT_HAPPENED; /* Set interrupt bit in read ICR */
        c64_->cpu_->irq(); /* Trigger interrupt */
      }
    }
    /* Set new timer B value */
    c64_->mem_->kCIA2MemRd[TBH] = (temp >> 8);
    c64_->mem_->kCIA2MemRd[TBL] = temp & 0xFF;
  }
  c64_->mem_->kCIA2MemWr[CRB] &= ~FORCELOADB_STROBE;   /* Reset timer A strobe bit */
  c64_->mem_->kCIA2MemRd[CRB] = c64_->mem_->kCIA2MemWr[CRB]; /* Copy CRB write to read register */

  /* Fake time of day timer */
  --_cia_tenthsecondcount;
  if(_cia_tenthsecondcount<=0) {
    _cia_tenthsecondcount = (_fake_samplerate/10);
    ++(c64_->mem_->kCIA2MemRd[TOD_TEN]);
    if(c64_->mem_->kCIA2MemRd[TOD_TEN]==9) {
      c64_->mem_->kCIA2MemRd[TOD_TEN] = 0;
      ++(c64_->mem_->kCIA2MemRd[TOD_SEC]);
      if(c64_->mem_->kCIA2MemRd[TOD_SEC]==59) {
        c64_->mem_->kCIA2MemRd[TOD_SEC] = 0;
        ++(c64_->mem_->kCIA2MemRd[TOD_MIN]);
        if(c64_->mem_->kCIA2MemRd[TOD_MIN]==59) {
          c64_->mem_->kCIA2MemRd[TOD_MIN] = 0;
          ++(c64_->mem_->kCIA2MemRd[TOD_HR]);
          if((c64_->mem_->kCIA2MemRd[TOD_HR]&0x1F)==11) {
            c64_->mem_->kCIA2MemRd[TOD_HR] = 0;
            if((c64_->mem_->kCIA2MemRd[TOD_HR]&0x80)==0) {
              c64_->mem_->kCIA2MemRd[TOD_HR] |= (1<<7);
            } else {
              c64_->mem_->kCIA2MemWr[TOD_HR] &= ~((1<<7)&0x7F);
            }
          }
        }
      }
    }
  }
  prev_cpu_cycles_ = c64_->cpu_->cycles();
  return true;
}
