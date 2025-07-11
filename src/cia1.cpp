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

enum
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

// ctor  /////////////////////////////////////////////////////////////////////

Cia1::Cia1()
{
  timer_a_latch_ = timer_b_latch_ = timer_a_counter_ = timer_b_counter_ = 0;
  timer_a_enabled_ = timer_b_enabled_ = timer_a_irq_enabled_ = timer_b_irq_enabled_ = false;
  timer_a_irq_triggered_ = timer_b_irq_triggered_ = false;
  timer_a_input_mode_ = timer_b_input_mode_ = kModeProcessor;
  timer_a_run_mode_ = timer_b_run_mode_ = kModeRestart;
  pra_ = prb_ = 0xff;
  prev_cpu_cycles_ = 0;
}

void Cia1::reset()
{
  timer_a_latch_ = timer_b_latch_ = timer_a_counter_ = timer_b_counter_ = 0;
  timer_a_enabled_ = timer_b_enabled_ = timer_a_irq_enabled_ = timer_b_irq_enabled_ = false;
  timer_a_irq_triggered_ = timer_b_irq_triggered_ = false;
  timer_a_input_mode_ = timer_b_input_mode_ = kModeProcessor;
  timer_a_run_mode_ = timer_b_run_mode_ = kModeRestart;
  pra_ = prb_ = 0xff;
  prev_cpu_cycles_ = 0;
}

// DMA register access  //////////////////////////////////////////////////////

void Cia1::write_register(uint8_t r, uint8_t v)
{
  switch(r)
  {
  /* data port a (0x0), keyboard matrix cols and joystick #2 */
  case PRA:
    pra_ = v;
    break;
  /* data port b (0x1), keyboard matrix rows and joystick #1 */
  case PRB:
    prb_ = v;
    break;
  /* data direction port a (0x2) */
  case DDRA:
    break;
  /* data direction port b (0x3) */
  case DDRB:
    break;
  /* timer a low byte (0x4) */
  case TAL:
    timer_a_latch_ &= 0xff00;
    timer_a_latch_ |= v;
    break;
  /* timer a high byte (0x5) */
  case TAH:
    timer_a_latch_ &= 0x00ff;
    timer_a_latch_ |= v << 8;
    break;
  /* timer b low byte (0x6) */
  case TBL:
    timer_b_latch_ &= 0xff00;
    timer_b_latch_ |= v;
    break;
  /* timer b high byte (0x7) */
  case TBH:
    timer_b_latch_ &= 0x00ff;
    timer_b_latch_ |= v << 8;
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
    if(ISSET_BIT(v,0)) timer_a_irq_enabled_ = ISSET_BIT(v,7);
    if(ISSET_BIT(v,1)) timer_b_irq_enabled_ = ISSET_BIT(v,7);
    break;
  /* control timer a (0xE) */
  case CRA:
    timer_a_enabled_ = ((v&(1<<0))!=0);
    timer_a_input_mode_ = (v&(1<<5)) >> 5;
    /* load latch requested */
    if((v&(1<<4))!=0)
      timer_a_counter_ = timer_a_latch_;
    break;
  /* control timer b (0xF) */
  case CRB:
    timer_b_enabled_ = ((v&0x1)!=0);
    timer_b_input_mode_ = (v&(1<<5)) | (v&(1<<6)) >> 5;
    /* load latch requested */
    if((v&(1<<4))!=0)
      timer_b_counter_ = timer_b_latch_;
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
    break;
  /* data port b (0x1), keyboard matrix rows and joystick #1 */
  case PRB:
    if (pra_ == 0xff) retval = 0xff;
    else if(pra_)
    {
      int col = 0;
      uint8_t v = ~pra_;
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
    retval = (uint8_t)(timer_a_counter_ & 0x00ff);
    break;
  /* timer a high byte (0x5) */
  case TAH:
    retval = (uint8_t)((timer_a_counter_ & 0xff00) >> 8);
    break;
  /* timer b low byte (0x6) */
  case TBL:
    retval = (uint8_t)(timer_b_counter_ & 0x00ff);
    break;
  /* timer b high byte (0x7) */
  case TBH:
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
    if(timer_a_irq_triggered_ ||
       timer_b_irq_triggered_)
    {
      retval |= (1 << 7); // IRQ occured
      if(timer_a_irq_triggered_) retval |= (1 << 0);
      if(timer_b_irq_triggered_) retval |= (1 << 1);
    }
    break;
  /* control timer a (0xE) */
  case CRA:
    break;
  /* control timer b (0xF) */
  case CRB:
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
    timer_a_counter_ = timer_a_latch_;
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
    timer_b_counter_ = timer_b_latch_;
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
    case kModeProcessor:
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
    case kModeProcessor:
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
