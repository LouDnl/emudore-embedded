#include <c64.h>
#include <MC68B50.h>


/**
 * @brief: Setup must happen _after_ memory
 */
Cart::Cart(C64 * c64) :
  c64_(c64)
{
  havecart = c64_->havecart;

  if (havecart) { /* TODO: Define cart type from .crt file if applied */
    cartactive = true;
    mc6850_ = new MC68B50(c64_); /* TODO: Only if needed for cart type */
    init_mc6850();
  }
}

Cart::~Cart()
{
  if (cartactive)  { delete mc6850_; }
  if (acia_active) { delete [] mem_rom_mc6850_; }
}

void Cart::init_mc6850(void)
{
  mem_rom_mc6850_ = new uint8_t[Memory::kPageSize]();
  acia_active = true;
  k6850MemWr = &c64_->mem_->mem_ram()[Memory::kAddrIO1Page];
  k6850MemRd = &mem_rom_mc6850_[0];
}

void Cart::reset(void)
{
  if (cartactive) {
    mc6850_->reset();
  }
}

void Cart::emulate(void)
{
  if (cartactive) {
    mc6850_->emulate();
  }
}

void Cart::write_register(uint8_t r, uint8_t v)
{
  if (cartactive) {
    mc6850_->write_register(r,v);
  }
}

uint8_t Cart::read_register(uint8_t r)
{
  uint8_t retval = 0;
  if (cartactive) {
    retval = mc6850_->read_register(r);
  }
  return retval;
}
