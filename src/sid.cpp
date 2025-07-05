

#include "sid.h"
#include "util.h"


Sid::Sid()
{

}

void Sid::set_cycles(void)
{
  prev_cpu_cycles_ = cpu_->cycles();
}

uint8_t Sid::read_register(uint8_t r)
{
  return 0;
}

void Sid::write_register(uint16_t r, uint8_t v)
{
  printf("[WR] $%04X:%02X %u\n", r, v, (cpu_->cycles() - prev_cpu_cycles_));
  prev_cpu_cycles_ = cpu_->cycles();
}
