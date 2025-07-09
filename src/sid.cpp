

#include "sid.h"
#include "util.h"


Sid::Sid()
{

}

void Sid::set_cycles(void)
{
  if ((cpu_->cycles() - prev_flush_cpu_cycles_) >= ((1 / 50.125) * 1000000)) {
    printf("SID Flush called @ %u cycles, last was at %u, diff %u\n",
      cpu_->cycles(), prev_flush_cpu_cycles_,
      (cpu_->cycles()- prev_flush_cpu_cycles_));
  }
  prev_cpu_cycles_ = cpu_->cycles();
  prev_flush_cpu_cycles_ = cpu_->cycles();
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
