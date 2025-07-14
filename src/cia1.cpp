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

Cia1::Cia1()
{
  /* Base */
  prev_cpu_cycles_ = 0;
}

void Cia1::reset()
{
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
  // if(r==0x4||r==0xD||r==0xE)D("CIA1 WR $%02X: %02X\n",r,v);
  switch(r)
  {
  /* data port a (0x0), keyboard matrix cols and joystick #2 */
  case PRA:
    mem_->kCIA1Mem[PRA] = v;
    break;
  /* data port b (0x1), keyboard matrix rows and joystick #1 */
  case PRB:
    mem_->kCIA1Mem[PRB] = v;
    break;
  /* data direction port a (0x2) */
  case DDRA:
    mem_->kCIA1Mem[DDRA] = v;
    break;
  /* data direction port b (0x3) */
  case DDRB:
    mem_->kCIA1Mem[DDRB] = v;
    break;
  /* timer a low byte (0x4) */
  case TAL:
    // timer_a_latch_ &= 0xff00; /* Keep high byte */
    // timer_a_latch_ |= v; /* Add new low byte */
    mem_->kCIA1Mem[TAL] = v; /* Latch low byte */
    break;
  /* timer a high byte (0x5) */
  case TAH:
    // timer_a_latch_ &= 0x00ff; /* Keep low byte */
    // timer_a_latch_ |= v << 8; /* Add new high byte */
    mem_->kCIA1Mem[TAH] = v; /* Latch high byte */
    break;
  /* timer b low byte (0x6) */
  case TBL:
    // timer_b_latch_ &= 0xff00;
    // timer_b_latch_ |= v;
    mem_->kCIA1Mem[TBL] = v; /* Latch low byte */
    break;
  /* timer b high byte (0x7) */
  case TBH:
    // timer_b_latch_ &= 0x00ff;
    // timer_b_latch_ |= v << 8;
    mem_->kCIA1Mem[TBH] = v; /* Latch high byte */
    break;
  /* RTC 1/10s (0x8) */
  case TOD_TEN:
    break;
  /* RTC seconds (0x9) */
  case TOD_SEC:
    break;
  /* RTC minutes (0xA) */
  case TOD_MIN:
    break;
  /* RTC hours (0xB) */
  case TOD_HR:
    break;
  /* shift serial (0xC) */
  case SDR:
    break;
  /* interrupt control and status (0xD) */
  case ICR:
    /**
     * if bit 7 is set, enable selected mask of
     * interrupts, else disable them
     */
    // if(ISSET_BIT(v,0)) timer_a_irq_enabled_ = ISSET_BIT(v,7);
    // if(ISSET_BIT(v,1)) timer_b_irq_enabled_ = ISSET_BIT(v,7);

    // D("ICR BEFORE: %02X V: %02X\n",
    // mem_->kCIA1Mem[ICR],v);
    if ((v&0x80)!=0)
      mem_->kCIA1Mem[ICR] |= (v&0x1F);
    else
      mem_->kCIA1Mem[ICR] &= ~(v&0x1F);
    // D("ICR AFTER: %02X V: %02X\n",
    // mem_->kCIA1Mem[ICR],v);

    if (ISSET_BIT(v,7)) { /* SET */
    //   v &= 0x1F;
    //   mem_->kCIA1Mem[ICR] |= v;
      timer_a_irq_enabled_ = ISSET_BIT(mem_->kCIA1Mem[ICR],0);
      timer_b_irq_enabled_ = ISSET_BIT(mem_->kCIA1Mem[ICR],1);
    } else { /* CLEAR */
    //   v &= 0x1F;
    //   mem_->kCIA1Mem[ICR] ^= v;
      timer_a_irq_enabled_ = ISSET_BIT(mem_->kCIA1Mem[ICR],0);
      timer_b_irq_enabled_ = ISSET_BIT(mem_->kCIA1Mem[ICR],1);
    }

    break;
  /* control timer a (0xE) */
  case CRA:
    mem_->kCIA1Mem[CRA] = v;

    timer_a_enabled_ = ((mem_->kCIA1Mem[CRA]&(1<<0))!=0);
    timer_a_input_mode_ = (mem_->kCIA1Mem[CRA]&(1<<5)) >> 5; /* 0x20>>5 = 1*/
    /* load latch requested */
    if((mem_->kCIA1Mem[CRA]&(1<<4))!=0) /* 0x10 */
      timer_a_counter_ = (mem_->kCIA1Mem[TAH] << 8 | mem_->kCIA1Mem[TAL]);
      // timer_a_counter_ = timer_a_latch_;

    break;
  /* control timer b (0xF) */
  case CRB:
    mem_->kCIA1Mem[CRB] = v;
    // mem_->kCIA1Mem[CRB] ^= v;
    // mem_->kCIA1Mem[CRB] &= ~FORCELOADB_STROBE;
    timer_b_enabled_ = ((mem_->kCIA1Mem[CRB]&0x1)!=0);
    timer_b_input_mode_ = (mem_->kCIA1Mem[CRB]&(1<<5)) | (mem_->kCIA1Mem[CRB]&(1<<6)) >> 5;
    /* load latch requested */
    if((mem_->kCIA1Mem[CRB]&(1<<4))!=0)
      timer_b_counter_ = (mem_->kCIA1Mem[TBH] << 8 | mem_->kCIA1Mem[TBL]);
      // timer_b_counter_ = timer_b_latch_;
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
    // break;
  /* data port b (0x1), keyboard matrix rows and joystick #1 */
  case PRB:
    if (mem_->kCIA1Mem[PRA] == 0xFF)
      retval = mem_->kCIA1Mem[PRA];
    else if(mem_->kCIA1Mem[PRA]) {
      int col = 0;
      uint8_t v = ~mem_->kCIA1Mem[PRA];
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
    // retval = mem_->kCIA1Mem[TAL];
    retval = (uint8_t)(timer_a_counter_ & 0x00ff);
    break;
  /* timer a high byte (0x5) */
  case TAH:
    // retval = mem_->kCIA1Mem[TAH];
    retval = (uint8_t)((timer_a_counter_ & 0xff00) >> 8);
    break;
  /* timer b low byte (0x6) */
  case TBL:
    // retval = mem_->kCIA1Mem[TBL];
    retval = (uint8_t)(timer_b_counter_ & 0x00ff);
    break;
  /* timer b high byte (0x7) */
  case TBH:
    // retval = mem_->kCIA1Mem[TBH];
    retval = (uint8_t)((timer_b_counter_ & 0xff00) >> 8);
    break;
  /* RTC 1/10s (0x8) */
  case TOD_TEN:
    break;
  /* RTC seconds (0x9) */
  case TOD_SEC:
    break;
  /* RTC minutes (0xA) */
  case TOD_MIN:
    break;
  /* RTC hours (0xB) */
  case TOD_HR:
    break;
  /* shift serial (0xC) */
  case SDR:
    break;
  /* interrupt control and status (0xD) */
  case ICR:
    /* ??? Reading from ICR should always return 0 ??? */
    // if(timer_a_irq_triggered_ ||
    //    timer_b_irq_triggered_)
    // {
    //   retval |= (1 << 7); // IRQ occured
    //   if(timer_a_irq_triggered_) retval |= (1 << 0);
    //   if(timer_b_irq_triggered_) retval |= (1 << 1);
    // }
    retval = mem_->kCIA1Mem[ICR] = 0;
    break;
  /* control timer a (0xE) */
  case CRA:
    retval = (mem_->kCIA1Mem[CRA] & 0xee) | timer_a_enabled_;
    break;
  /* control timer b (0xF) */
  case CRB:
    retval = (mem_->kCIA1Mem[CRB] & 0xee) | timer_b_enabled_;
    break;
  }
  return retval;
}

// timer reset ///////////////////////////////////////////////////////////////

void Cia1::reset_timer_a()
{
  switch(timer_a_run_mode_)
  {
  case kModeRestart:
    timer_a_counter_ = (mem_->kCIA1Mem[TAH] << 8 | mem_->kCIA1Mem[TAL]);
    // timer_a_counter_ = timer_a_latch_;
    break;
  case kModeOneTime:
    timer_a_enabled_ = false;
    break;
  }
}

void Cia1::reset_timer_b()
{
  switch(timer_b_run_mode_)
  {
  case kModeRestart:
    timer_b_counter_ = (mem_->kCIA1Mem[TBH] << 8 | mem_->kCIA1Mem[TBL]);
    // timer_b_counter_ = timer_b_latch_;
    break;
  case kModeOneTime:
    timer_b_enabled_ = false;
    break;
  }
}

// emulation  ////////////////////////////////////////////////////////////////

bool Cia1::emulate()
{
  /* timer a */
  if(timer_a_enabled_)
  {
    switch(timer_a_input_mode_)
    {
    case kModePHI2:
      timer_a_counter_ -= cpu_->cycles() - prev_cpu_cycles_;
      if (timer_a_counter_ <= 0)
      {
        if(timer_a_irq_enabled_)
        {
          timer_a_irq_triggered_ = true;
          cpu_->irq();
        }
        reset_timer_a();
      }
      break;
    case kModeCNT:
      break;
    }
  }
  /* timer b */
  if(timer_b_enabled_)
  {
    switch(timer_b_input_mode_)
    {
    case kModePHI2:
      timer_b_counter_ -= cpu_->cycles() - prev_cpu_cycles_;
      if (timer_b_counter_ <= 0)
      {
        if(timer_b_irq_enabled_)
        {
          timer_b_irq_triggered_ = true;
          cpu_->irq();
        }
        reset_timer_b();
      }
      break;
    case kModeCNT:
      break;
    case kModeTimerA:
      break;
    case kModeTimerACNT:
      break;
    }
  }
  prev_cpu_cycles_ = cpu_->cycles();
  return true;
}
