#include <c64.h>
#include <MC68B50.h>
#include <util.h>


enum ACIARegisters
{
  CONTROL = 0x04, /* control register       ~ write only */
  STATUS  = 0x06, /* status register        ~ read only */
  TXDR    = 0x05, /* transmit data register ~ write only */
  RXDR    = 0x07, /* receive data register  ~ read  only */
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

enum StatusRegister
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


/**
 * @brief: Setup must happen _after_ memory
 */
MC68B50::MC68B50(C64 * c64) :
  c64_(c64)
{
  // c64_->cart_->init_mc6850();
}

MC68B50::~MC68B50()
{

}

void MC68B50::reset()
{
  c64_->cart_->k6850MemRd[STATUS] = TDRE; /* Set TDRE default to empty */
}

/**
 * @brief Handle Midi keyboard key down event
 *
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
 *
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
 * @brief reads a byte from a Motorola MC68B50 ACIA register @ $denn
 */
uint8_t MC68B50::read_register(uint8_t r)
{
  uint8_t retval = 0;
  switch (r) {
    case CONTROL: /* $de04 ~ control register ~ write only */
      retval = c64_->cart_->k6850MemRd[r]; // Return Wr register?
      break;
    case STATUS:  /* $de06 ~ status register  ~ read only */
      retval = c64_->cart_->k6850MemRd[r];
      break;
    case TXDR:    /* $de05 ~ TX register      ~ write only */
      break;
    /* NOTE: Data in the RXDR should come from a ringbuffer that shifts after reading */
    case RXDR:    /* $de07 ~ RX register      ~ read only */
      c64_->cart_->k6850MemRd[STATUS] &= ~(IRQ|RDRF); /* Clear IRQ and RDRF on read (if set) */
      if (n_read < 3) {
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
    default:
      retval = c64_->cart_->k6850MemRd[r];
      break;
  }

  return retval;
}

void MC68B50::write_register(uint8_t r, uint8_t v)
{
  uint8_t cr = 0; /* control register masked value */
  switch (r) {
    case CONTROL: /* $de04 ~ control register ~ write only */
      c64_->cart_->k6850MemWr[r] = c64_->cart_->k6850MemRd[r] = v; /* Save value to ram */
      cr = (v & CR0CR1SEL);      /* Counter divide select bits */
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
          // c64_->cart_->k6850MemRd[STATUS] = TDRE;
          reset();
          break;
        default: break;
      }
      cr = ((v & WORDSEL) >> 2); /* Word select bits */
      switch (cr) {
        case w7e2: break; /* Unused for CynthCart */
        case w7o2: break; /* Unused for CynthCart */
        case w7e1: break; /* Unused for CynthCart */
        case w7o1: break; /* Unused for CynthCart */
        case w8n2: break; /* Unused for CynthCart */
        case w8n1: /* 8 Bits+1 Stop Bit */
          D("[MC68B50] 8 Bits+1 Stop Bit\n");
          break;
        case w8e1: break; /* Unused for CynthCart */
        case w8o1: break; /* Unused for CynthCart */
        default: break;
      }
      cr = ((v & TCCTR) >> 5);   /* Transmit control bits */
      switch (cr) {
        case RTSloTID:        /* RTS=low, Transmitting Interrupt Disabled. */
          D("[MC68B50] RTS=low, Transmitting Interrupt Disabled\n");
          break;
        case RTSloTIE: break; /* RTS=low, Transmitting Interrupt Enabled. */
        case RTShiTID: break; /* RTS=high, Transmitting Interrupt Disabled */
        case RTSloTRB: break; /* RTS=low, Transmits a Break level on the Transmit Data Output. Transmitting Interrupt Disabled. */
        default: break;
      }
      cr = ((v & INTEN) >> 7);   /* Receive interrupt enable bit */
      switch (cr) {
        case 0b0: break; /* Receive interrupt disabled */
        case 0b1: /* Receive interrupt enabled */
          D("[MC68B50] Receive interrupt enabled\n");
          break;
        default: break;
      }
      break;
    case STATUS:  /* $de06 ~ status register  ~ read only */
      // c64_->cart_->k6850MemWr[r] = c64_->cart_->k6850MemRd[r] = v; /* Save value to ram */
      break;
    case TXDR:    /* $de05 ~ TX register      ~ write only*/
      c64_->cart_->k6850MemWr[r] = c64_->cart_->k6850MemRd[r] = v; /* Save value to ram */
      break;
    case RXDR:    /* $de07 ~ RX register      ~ read only */
      break;
    default:      /* default always write to read and write */
      c64_->cart_->k6850MemWr[r] = c64_->cart_->k6850MemRd[r] = v;
      break;
  }
}

void MC68B50::emulate()
{
  if(!c64_->cpu_->idf()) { /* If no CPU IRQ already in place */
    if (c64_->cart_->k6850MemRd[STATUS] & IRQ) { /* If IRQ is set */
      D("[MC68B50] Trigger CPU IRQ!\n");
      c64_->cpu_->irq(); /* Trigger CPU IRQ */
    }
  }
}
