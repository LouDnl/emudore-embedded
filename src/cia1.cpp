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

#include "cia1.h"

enum CIA1Registers
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

Cia1::Cia1() /* 0xDC00 */
{
  /* Base */
  prev_cpu_cycles_ = 0;
}

void Cia1::reset()
{
  prev_cpu_cycles_ = 0;

  for (uint i = 2; i < 0x10; i++) { /* Make sure register 0->16 are zeroed out */
    mem_->write_byte_no_io(mem_->kCIA1MemRd[i],0x00);
    mem_->write_byte_no_io(mem_->kCIA1MemWr[i],0x00);
  }
  /* PRA and PRB are 0xFF at boot */
  mem_->write_byte_no_io(mem_->kCIA1MemRd[PRA],0xFF);
  mem_->write_byte_no_io(mem_->kCIA1MemWr[PRA],0xFF);
  mem_->write_byte_no_io(mem_->kCIA1MemRd[PRB],0xFF);
  mem_->write_byte_no_io(mem_->kCIA1MemWr[PRB],0xFF);
  /* OLD */
  timer_a_latch_ = timer_b_latch_ = timer_a_counter_ = timer_b_counter_ = 0;
  timer_a_enabled_ = timer_b_enabled_ = timer_a_irq_enabled_ = timer_b_irq_enabled_ = false;
  timer_a_irq_triggered_ = timer_b_irq_triggered_ = false;
  timer_a_input_mode_ = timer_b_input_mode_ = kModePHI2;
  timer_a_run_mode_ = timer_b_run_mode_ = kModeRestart;

  /* NEW */
  for (uint i = 2; i < 0x10; i++)
    mem_->write_byte_no_io(mem_->kCIA1Mem[i],0x00);
  mem_->write_byte_no_io(mem_->kCIA1Mem[PRA],0xFF);
  mem_->write_byte_no_io(mem_->kCIA1Mem[PRB],0xFF);
}

// DMA register access  //////////////////////////////////////////////////////

void Cia1::write_register(uint8_t r, uint8_t v)
{
  switch(r)
  {
  /* data port a (0x0), keyboard matrix cols and joystick #2 */
  case PRA:
    mem_->kCIA1MemWr[PRA] = mem_->kCIA1MemRd[PRA] = v;
    break;
  /* data port b (0x1), keyboard matrix rows and joystick #1 */
  case PRB:
    mem_->kCIA1MemWr[PRB] = mem_->kCIA1MemRd[PRB] = v;
    break;
  /* data direction port a (0x2) */
  case DDRA:
    mem_->kCIA1MemWr[DDRA] = mem_->kCIA1MemRd[DDRA] = v;
    break;
  /* data direction port b (0x3) */
  case DDRB:
    mem_->kCIA1MemWr[DDRB] = mem_->kCIA1MemRd[DDRB] = v;
    break;
  /* timer a low byte (0x4) */
  case TAL: /* latch */
    mem_->kCIA1MemWr[TAL] = v; /* Latch low byte */
    break;
  /* timer a high byte (0x5) */
  case TAH: /* latch */
    mem_->kCIA1MemWr[TAH] = v; /* Latch high byte */
    break;
  /* timer b low byte (0x6) */
  case TBL: /* latch */
    mem_->kCIA1MemWr[TBL] = v; /* Latch low byte */
    break;
  /* timer b high byte (0x7) */
  case TBH: /* latch */
    mem_->kCIA1MemWr[TBH] = v; /* Latch high byte */
    break;
  /* RTC 1/10s (0x8) */
  case TOD_TEN:
    mem_->kCIA1MemWr[TOD_TEN] = mem_->kCIA1MemRd[TOD_TEN] = v;
    break;
  /* RTC seconds (0x9) */
  case TOD_SEC:
    mem_->kCIA1MemWr[TOD_SEC] = mem_->kCIA1MemRd[TOD_SEC] = v;
    break;
  /* RTC minutes (0xA) */
  case TOD_MIN:
    mem_->kCIA1MemWr[TOD_MIN] = mem_->kCIA1MemRd[TOD_MIN] = v;
    break;
  /* RTC hours (0xB) */
  case TOD_HR:
    mem_->kCIA1MemWr[TOD_HR] = mem_->kCIA1MemRd[TOD_HR] = v;
    break;
  /* shift serial (0xC) */
  case SDR:
    mem_->kCIA1MemWr[SDR] = mem_->kCIA1MemRd[SDR] = v;
    break;
  /* interrupt control and status (0xD) */
  case ICR:
    /**
     * if bit 7 is set, enable selected mask of
     * interrupts, else disable them
     */
    if (v&0x80)
      mem_->kCIA1MemWr[ICR] |= (v&0x1F);
    else
      mem_->kCIA1MemWr[ICR] &= ~(v&0x1F);
    mem_->kCIA1MemRd[ICR] = (mem_->kCIA1MemWr[ICR]&0x1F);
    break;
  /* control timer a (0xE) */
  case CRA:
    mem_->kCIA1MemWr[CRA] = mem_->kCIA1MemRd[CRA] = v; /* Write the data to the register */
    break;
  /* control timer b (0xF) */
  case CRB:
    mem_->kCIA1MemWr[CRB] = mem_->kCIA1MemRd[CRB] = v;
    break;
  }
}

uint8_t Cia1::read_register(uint8_t r)
{
  uint8_t retval = 0;

  switch(r)
  {
  /* data port a (0x0), keyboard matrix cols and joystick #2 */
  case PRA:
  /* data port b (0x1), keyboard matrix rows and joystick #1 */
  case PRB:
    if (mem_->kCIA1MemWr[PRA] == 0xFF)
      retval = mem_->kCIA1MemWr[PRA];
    else if(mem_->kCIA1MemWr[PRA]) {
      int col = 0;
      uint8_t v = ~mem_->kCIA1MemWr[PRA];
      while (v >>= 1)col++;
      retval = io_->keyboard_matrix_row(col);
    }
    break;
  /* data direction port a (0x2) */
  case DDRA:
    break;
  /* data direction port b (0x3) */
  case DDRB:
    break;
  /* timer a low byte (0x4) */
  case TAL:
    retval = mem_->kCIA1MemRd[TAL];
    break;
  /* timer a high byte (0x5) */
  case TAH:
    retval = mem_->kCIA1MemRd[TAH];
    break;
  /* timer b low byte (0x6) */
  case TBL:
    retval = mem_->kCIA1MemRd[TBL];
    break;
  /* timer b high byte (0x7) */
  case TBH:
    retval = mem_->kCIA1MemRd[TBH];
    break;
  /* RTC 1/10s (0x8) */
  case TOD_TEN:
    retval = mem_->kCIA1MemRd[TOD_TEN];
    break;
  /* RTC seconds (0x9) */
  case TOD_SEC:
    retval = mem_->kCIA1MemRd[TOD_SEC];
    break;
  /* RTC minutes (0xA) */
  case TOD_MIN:
    retval = mem_->kCIA1MemRd[TOD_MIN];
    break;
  /* RTC hours (0xB) */
  case TOD_HR:
    retval = mem_->kCIA1MemRd[TOD_HR];
    break;
  /* shift serial (0xC) */
  case SDR:
    retval = mem_->kCIA1MemRd[SDR];
    break;
  /* interrupt control and status (0xD) */
  case ICR:
    /* Reading from ICR clears the read register after reading */
    if (mem_->kCIA1MemRd[ICR] & INTERRUPT_HAPPENED) {
      retval |= (1<<7); /* IRQ triggered */
      if(mem_->kCIA1MemRd[ICR] & 1) retval |= 1; /* For timer A */
      if(mem_->kCIA1MemRd[ICR] & 2) retval |= 2; /* And/or timer B */
    }
    mem_->kCIA1MemRd[ICR] = 0; /* Clear read register */
    break;
  /* control timer a (0xE) */
  case CRA:
    retval = (mem_->kCIA1MemRd[CRA] & 0xee) | (mem_->kCIA1MemWr[CRA]&0x1);
    break;
  /* control timer b (0xF) */
  case CRB:
    retval = (mem_->kCIA1MemRd[CRB] & 0xee) | (mem_->kCIA1MemWr[CRB]&0x1);
    break;
  }
  return retval;
}

// emulation  ////////////////////////////////////////////////////////////////

bool Cia1::emulate()
{
  static int temp; /* Timer */

  /* Check for force load latch requested */
  if(mem_->kCIA1MemWr[CRA] & FORCELOADA_STROBE) {
    mem_->kCIA1MemRd[TAH] = mem_->kCIA1MemWr[TAH];
    mem_->kCIA1MemRd[TAL] = mem_->kCIA1MemWr[TAL];
  }
  else /* Check for timer A enabled, counts Phi2 */
  if ((mem_->kCIA1MemWr[CRA] & (ENABLE_TIMERA|TIMERA_FROM_CNT)) == ENABLE_TIMERA) { /* Check for count Phi2 enabled */
    temp = ((mem_->kCIA1MemRd[TAH]<<8) + mem_->kCIA1MemRd[TAL]) - (cpu_->cycles() - prev_cpu_cycles_); /* Update counter */
    if (temp <= 0) {  /* counter underflow */
      temp = (mem_->kCIA1MemWr[TAH]<<8) + mem_->kCIA1MemWr[TAL]; /* reload timer */
      if (mem_->kCIA1MemWr[CRA] & ONESHOT_TIMERA) { /* If one-shot is enabled */
        mem_->kCIA1MemWr[CRA] &= ~ENABLE_TIMERA; /* Disable timer A */
      }
      mem_->kCIA1MemRd[ICR] |= TIMERA; /* Set timer A in read ICR */
      if (mem_->kCIA1MemWr[ICR] & TIMERA) { /* Generate interrupt if write mask allows */
        mem_->kCIA1MemRd[ICR] |= INTERRUPT_HAPPENED; /* Set interrupt bit in read ICR */
        cpu_->irq(); /* Trigger interrupt */
      }
    }
    /* Set new timer A value */
    mem_->kCIA1MemRd[TAH] = (temp >> 8);
    mem_->kCIA1MemRd[TAL] = (temp & 0xFF);
  }
  mem_->kCIA1MemWr[CRA] &= ~FORCELOADA_STROBE;   /* Reset timer A strobe bit */
  mem_->kCIA1MemRd[CRA] = mem_->kCIA1MemWr[CRA]; /* Copy CRA write to read register */

  /* Check for force load latch requested */
  if (mem_->kCIA1MemWr[CRB] & FORCELOADB_STROBE) {
    mem_->kCIA1MemRd[TBH] = mem_->kCIA1MemWr[TBH];
    mem_->kCIA1MemRd[TBL] = mem_->kCIA1MemWr[TBL];
  }
  else /* Check for timer B enabled */
  if ((mem_->kCIA1MemWr[CRB] & (ENABLE_TIMERB|TIMERB_FROM_TIMERA)) == ENABLE_TIMERB) { /* Check for count Phi2 enabled */
    temp = ((mem_->kCIA1MemRd[TBH]<<8) + mem_->kCIA1MemRd[TBL] ) - (cpu_->cycles() - prev_cpu_cycles_); /* Update counter */
    if (temp <= 0) { //Timer counted down
      temp += (mem_->kCIA1MemWr[TBH]<<8) + mem_->kCIA1MemWr[TBL]; /* reload timer */
      if (mem_->kCIA1MemWr[CRB] & ONESHOT_TIMERB) { /* If one-shot is enabled */
         mem_->kCIA1MemWr[CRB] &= ~ENABLE_TIMERB; /* Disable timer A */
      }
      mem_->kCIA1MemRd[ICR] |= TIMERB; /* Set timer B in read ICR */
      if (mem_->kCIA1MemWr[ICR] & TIMERB) { /* Generate interrupt if write mask allows */
        mem_->kCIA1MemRd[ICR] |= INTERRUPT_HAPPENED; /* Set interrupt bit in read ICR */
        cpu_->irq(); /* Trigger interrupt */
      }
    }
    /* Set new timer B value */
    mem_->kCIA1MemRd[TBH] = (temp >> 8);
    mem_->kCIA1MemRd[TBL] = temp & 0xFF;
  }
  mem_->kCIA1MemWr[CRB] &= ~FORCELOADB_STROBE;   /* Reset timer A strobe bit */
  mem_->kCIA1MemRd[CRB] = mem_->kCIA1MemWr[CRB]; /* Copy CRB write to read register */

  /* Fake time of day timer */
  --_cia_tenthsecondcount;
  if(_cia_tenthsecondcount<=0) {
    _cia_tenthsecondcount = (_fake_samplerate/10);
    ++(mem_->kCIA1MemRd[TOD_TEN]);
    if(mem_->kCIA1MemRd[TOD_TEN]==9) {
      mem_->kCIA1MemRd[TOD_TEN] = 0;
      ++(mem_->kCIA1MemRd[TOD_SEC]);
      if(mem_->kCIA1MemRd[TOD_SEC]==59) {
        mem_->kCIA1MemRd[TOD_SEC] = 0;
        ++(mem_->kCIA1MemRd[TOD_MIN]);
        if(mem_->kCIA1MemRd[TOD_MIN]==59) {
          mem_->kCIA1MemRd[TOD_MIN] = 0;
          ++(mem_->kCIA1MemRd[TOD_HR]);
          if((mem_->kCIA1MemRd[TOD_HR]&0x1F)==11) {
            mem_->kCIA1MemRd[TOD_HR] = 0;
            if((mem_->kCIA1MemRd[TOD_HR]&0x80)==0) {
              mem_->kCIA1MemRd[TOD_HR] |= (1<<7);
            } else {
              mem_->kCIA1MemWr[TOD_HR] &= ~((1<<7)&0x7F);
            }
          }
        }
      }
    }
  }

  prev_cpu_cycles_ = cpu_->cycles();
  return true;
}
