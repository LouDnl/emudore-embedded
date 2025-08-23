/*
 * Motorola 68B50 ACIA emulation
 * (for Midi and or Uart communication)
 * Copyright (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * MC68B50.h
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

#ifndef EMUDORE_65B80_H
#define EMUDORE_65B80_H

#include <iostream>
#include <cstdint>


/* Fake data for testing */
/* NoteOn/Channel1, Note, Touchspeed */
static uint8_t midi_keydown[3] = { 0x90, 0x1F, 0x3F };
/* NoteOff/Channel1, Note, Touchspeed */
static uint8_t midi_keyup[3] = { 0x80, 0x1F, 0x1F };
static bool _keydown = false, _keyup = true;
static int n_read = 3;

/**
 * @brief Parent class containing register addresses,
 * bitmasks and bits
 */
class MC6850BitMasks
{
  protected:

    enum ACIARegisters
    { /* DATEL/SIEL/JMS/C-LAB (Kerberos) */
      CONTROL  = 0x04, /* control register       ~ write only */
      STATUS   = 0x06, /* status register        ~ read only */
      TXDR     = 0x05, /* transmit data register ~ write only */
      RXDR     = 0x07, /* receive data register  ~ read  only */
    };

    enum ControlBitMasks
    {
      CR0CR1SEL = 0b00000011, /* Counter divide select bits */
      WORDSEL   = 0b00011100, /* Word select bits */
      TCCTR     = 0b01100000, /* Transmit control bits */
      INTEN     = 0b10000000, /* Receive interrupt enable bit */
    };

    enum CounterDivideBits
    {
      R1  = 0b00, /* set divide ratio +1 */
      R16 = 0b01, /* set divide ratio +16 */
      R64 = 0b10, /* set divide ratio +64 */
      RES = 0b11, /* reset midi device */
    };

    enum WordSelectBits
    {
      w7e2 = 0b000, /* 7 Bits + Even Parity + 2 Stop Bits */
      w7o2 = 0b001, /* 7 Bits + Odd Parity + 2 Stop Bits */
      w7e1 = 0b010, /* 7 Bits + Even Parity + 1 Stop Bit */
      w7o1 = 0b011, /* 7 Bits + Odd Parity + 1 Stop Bit */
      w8n2 = 0b100, /* 8 Bits + 2 Stop Bits */
      w8n1 = 0b101, /* 8 Bits + 1 Stop Bit */
      w8e1 = 0b110, /* 8 Bits + Even Parity + 1 Stop Bit */
      w8o1 = 0b111, /* 8 Bits + Odd Parity + 1 Stop Bit */
    };

    enum TransmitControlBits
    {
      RTSloTID = 0b00, /* RTS=low, Transmitting Interrupt Disabled. */
      RTSloTIE = 0b01, /* RTS=low, Transmitting Interrupt Enabled. */
      RTShiTID = 0b10, /* RTS=high, Transmitting Interrupt Disabled */
      RTSloTRB = 0b11, /* RTS=low, Transmits a Break level on the Transmit Data Output. Transmitting Interrupt Disabled. */
    };

    enum StatusRegisterBits
    {
      RDRF = (1<<0), /* Receive Data Register Full */
      TDRE = (1<<1), /* Transmit Data Register Empty */
      DCD  = (1<<2), /* Data Carrier Detect */
      CTS  = (1<<3), /* Clear-to-Send */
      FE   = (1<<4), /* Framing Error */
      RO   = (1<<5), /* Receiver Overrun */
      PE   = (1<<6), /* Parity Error */
      IRQ  = (1<<7), /* Interrupt Request */
    };

    /* Interface registers per Midi device type */
    enum ACIARegisters_all_unused
    {
      /* SEQUENTIAL CIRCUITS INC. */
      CONTROL_1  = 0x00, /* control register       ~ write only */
      STATUS_1   = 0x02, /* status register        ~ read only */
      TXDR_1     = 0x01, /* transmit data register ~ write only */
      RXDR_1     = 0x03, /* receive data register  ~ read  only */
      /* PASSPORT & SENTECH */
      CONTROL_2  = 0x08, /* control register       ~ write only */
      STATUS_2   = 0x08, /* status register        ~ read only */
      TXDR_2     = 0x09, /* transmit data register ~ write only */
      RXDR_2     = 0x09, /* receive data register  ~ read  only */
      /* DATEL/SIEL/JMS/C-LAB (Kerberos) */
      CONTROL_3  = 0x04, /* control register       ~ write only */
      STATUS_3   = 0x06, /* status register        ~ read only */
      TXDR_3     = 0x05, /* transmit data register ~ write only */
      RXDR_3     = 0x07, /* receive data register  ~ read  only */
      /* NAMESOFT ~ SAME REGISTERS AS SEQUENTIAL SO NOT USED! */
      CONTROL_4  = 0x00, /* control register       ~ write only */
      STATUS_4   = 0x02, /* status register        ~ read only */
      TXDR_4     = 0x01, /* transmit data register ~ write only */
      RXDR_4     = 0x03, /* receive data register  ~ read  only */
    };

  public:
    MC6850BitMasks(){};
    ~MC6850BitMasks(){};
};

/**
 * @brief Motorola 68B50 Asynchronous Communications Interface Adapter
 *
 * Emulates the Motorola 68B50 ACIA (for Midi and or Uart communication)
 *
 */
class MC68B50 : protected MC6850BitMasks
{
  private:

    /* Main pointer */
    C64 *c64_;

  public:
    MC68B50(C64 * c64);
    ~MC68B50(){};

    void write_register(uint8_t r, uint8_t v);
    uint8_t read_register(uint8_t r);

    /* Handle midi keyboard keydown and keyup */
    void fake_keydown(void);
    void fake_keyup(void);

    void reset();
    void emulate();

};


#endif /* EMUDORE_65B80_H */
