/*
 * emudore,Commodore 64 emulator
 * Copyright (c) 2016,Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * cpu.cpp
 *
 * Licensed under the Apache License,Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sstream>

#include <c64.h>

Cpu::Cpu(C64 * c64) :
  c64_(c64)
{
  initialize_instruction_table();
  D("[EMU] Cpu initialized.\n");
}

/**
 * @brief Pre define static CPU cycles variable
 *
 */
unsigned int Cpu::cycles_ = 0;
bool Cpu::loginstructions = false;
// bool Cpu::logillegals = false;

/**
 * @brief Cold reset
 *
 * https://www.c64-wiki.com/index.php/Reset_(Process)
 */
void Cpu::reset()
{
  a_ = x_ = y_ = sp_ = 0;
  _flags = 0b0;
  pc(c64_->mem_->read_word(Memory::kAddrResetVector));
  cycles_ = 6;
}

/**
 * @brief emulate instruction
 * @return returns false if something goes wrong (e.g. illegal instruction)
 *
 * Current limitations:
 *
 * - Illegal instructions are not implemented
 * - Excess cycles due to page boundary crossing are not calculated
 * - Some known architectural bugs are not emulated
 */
bool Cpu::emulate()
{
  /* fetch instruction */
  uint8_t insn = fetch_op();
  pb_crossed = false;
  bool retval = true;
  if (loginstructions) { dump_regs_insn(insn); }
  /* emulate instruction */
  execute_opcode(insn);
  pb_crossed = false;
  return retval;
}

// helpers ///////////////////////////////////////////////////////////////////
static unsigned short d_address;

inline uint8_t Cpu::load_byte(uint16_t addr)
{
  d_address = addr;
  return c64_->mem_->read_byte(addr);
}

inline uint16_t Cpu::load_word(uint16_t addr)
{
  d_address = addr;
  return c64_->mem_->read_word_no_io(addr);
}

inline void Cpu::push(uint8_t v)
{
  uint16_t addr = Memory::kBaseAddrStack+sp_;
  d_address = addr;
  c64_->mem_->write_byte(addr,v);
  sp_--;
}

inline uint8_t Cpu::pop()
{
  uint16_t addr = ++sp_+Memory::kBaseAddrStack;
  d_address = addr;
  return load_byte(addr);
}

inline uint8_t Cpu::fetch_op()
{
  return load_byte(pc_++);
}

inline uint16_t Cpu::fetch_opw()
{
  uint16_t retval = c64_->mem_->read_word(pc_);
  pc_+=2;
  return retval;
}

inline uint16_t Cpu::addr_zero()
{
  uint16_t addr = fetch_op();
  d_address = addr;
  return addr;
}

inline uint16_t Cpu::addr_zerox()
{
  /* wraps around the zeropage */
  uint16_t addr = (fetch_op() + x()) & 0xff;
  d_address = addr;
  return addr;
}

inline uint16_t Cpu::addr_zeroy()
{
  /* wraps around the zeropage */
  uint16_t addr = (fetch_op() + y()) & 0xff;
  d_address = addr;
  return addr;
}

inline uint16_t Cpu::addr_abs()
{
  uint16_t addr = fetch_opw();
  d_address = addr;
  return addr;
}

inline uint16_t Cpu::addr_absy()
{
  uint16_t addr = fetch_opw();
  curr_page = addr&0xff00;
  addr += y();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  d_address = addr;
  return addr;
}

inline uint16_t Cpu::addr_absx()
{
  uint16_t addr = fetch_opw();
  curr_page = addr&0xff00;
  addr += x();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  d_address = addr;
  return addr;
}

inline uint16_t Cpu::addr_indx()
{
  /* wraps around the zeropage */
  uint16_t addr = c64_->mem_->read_word((addr_zero() + x()) & 0xff);
  d_address = addr;
  return addr;
}

inline uint16_t Cpu::addr_indy()
{
  uint16_t addr = c64_->mem_->read_word(addr_zero());
  curr_page = addr&0xff00;
  addr += y();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  d_address = addr;
  return addr;
}

// Instructions: data handling and memory operations  ////////////////////////

/**
 * @brief STore Accumulator
 */
inline void Cpu::sta(uint16_t addr,uint8_t cycles)
{
  c64_->mem_->write_byte(addr,a());
  tick(cycles);
}

/**
 * @brief STore X
 */
inline void Cpu::stx(uint16_t addr,uint8_t cycles)
{
  c64_->mem_->write_byte(addr,x());
  tick(cycles);
}

/**
 * @brief STore Y
 */
inline void Cpu::sty(uint16_t addr,uint8_t cycles)
{
  c64_->mem_->write_byte(addr,y());
  tick(cycles);
}

/**
 * @brief Transfer X to Stack pointer
 */
inline void Cpu::txs()
{
  sp(x());
  tick(2);
}

/**
 * @brief Transfer Stack pointer to X
 */
inline void Cpu::tsx()
{
  x(sp());
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief LoaD Accumulator
 */
inline void Cpu::lda(uint8_t v,uint8_t cycles)
{
  a(v);
  SET_ZF(a());
  SET_NF(a());
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

/**
 * @brief LoaD X
 */
inline void Cpu::ldx(uint8_t v,uint8_t cycles)
{
  x(v);
  SET_ZF(x());
  SET_NF(x());
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

/**
 * @brief LoaD Y
 */
inline void Cpu::ldy(uint8_t v,uint8_t cycles)
{
  y(v);
  SET_ZF(y());
  SET_NF(y());
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

/**
 * @brief Transfer X to Accumulator
 */
inline void Cpu::txa()
{
  a(x());
  SET_ZF(a());
  SET_NF(a());
  tick(2);
}

/**
 * @brief Transfer Accumulator to X
 */
inline void Cpu::tax()
{
  x(a());
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief Transfer Accumulator to Y
 */
inline void Cpu::tay()
{
  y(a());
  SET_ZF(y());
  SET_NF(y());
  tick(2);
}

/**
 * @brief Transfer Y to Accumulator
 */
inline void Cpu::tya()
{
  a(y());
  SET_ZF(a());
  SET_NF(a());
  tick(2);
}

/**
 * @brief PusH Accumulator
 */
inline void Cpu::pha()
{
  push(a());
  tick(3);
}

/**
 * @brief PuLl Accumulator
 */
inline void Cpu::pla()
{
  a(pop());
  SET_ZF(a());
  SET_NF(a());
  tick(4);
}

// Instructions: logic operations  ///////////////////////////////////////////

/**
 * @brief Logical OR on Accumulator
 */
inline void Cpu::ora(uint8_t v,uint8_t cycles)
{
  a(a()|v);
  SET_ZF(a());
  SET_NF(a());
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

/**
 * @brief Logical AND
 */
inline void Cpu::_and(uint8_t v,uint8_t cycles)
{
  a(a()&v);
  SET_ZF(a());
  SET_NF(a());
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

/**
 * @brief BIT test
 */
inline void Cpu::bit(uint16_t addr,uint8_t cycles)
{
  uint8_t t = load_byte(addr);
  SET_NF(t);
  SET_OF(t);
  SET_ZF(t&a());
  tick(cycles);
}

/**
 * @brief ROtate Left
 */
inline uint8_t Cpu::rol(uint8_t v)
{
  uint16_t t = (v << 1) | (uint8_t)cf();
  cf((t&0x100)!=0);
  // SET_CF(t); // BUG: Not working yet :-)
  SET_ZF(t);
  SET_NF(t);
  return (uint8_t)t;
}

/**
 * @brief ROL A register
 */
inline void Cpu::rol_a()
{
  a(rol(a()));
  tick(2);
}

/**
 * @brief ROL mem
 */
inline void Cpu::rol_mem(uint16_t addr,uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  c64_->mem_->write_byte(addr,v);
  c64_->mem_->write_byte(addr,rol(v));
  tick(cycles);
}

/**
 * @brief ROtate Right
 */
inline uint8_t Cpu::ror(uint8_t v)
{
  uint16_t t = (v >> 1) | (uint8_t)(cf() << 7);
  cf((v&0x1)!=0);
  SET_ZF(t);
  SET_NF(t);
  return (uint8_t)t;
}

/**
 * @brief ROR A register
 */
inline void Cpu::ror_a()
{
  a(ror(a()));
  tick(2);
}

/**
 * @brief ROR mem
 */
inline void Cpu::ror_mem(uint16_t addr,uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  c64_->mem_->write_byte(addr,v);
  c64_->mem_->write_byte(addr,ror(v));
  tick(cycles);
}

/**
 * @brief Logic Shift Right
 */
inline uint8_t Cpu::lsr(uint8_t v)
{
  uint8_t t = v >> 1;
  cf((v&0x1)!=0);
  SET_ZF(t);
  SET_NF(t);
  return t;
}

/**
 * @brief LSR A
 */
inline void Cpu::lsr_a()
{
  a(lsr(a()));
  tick(2);
}

/**
 * @brief LSR mem
 */
inline void Cpu::lsr_mem(uint16_t addr,uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  c64_->mem_->write_byte(addr,v);
  c64_->mem_->write_byte(addr,lsr(v));
  tick(cycles);
}

/**
 * @brief Arithmetic Shift Left
 */
inline uint8_t Cpu::asl(uint8_t v)
{
  uint8_t t = (v << 1) & 0xff;
  cf((v&0x80)!=0);
  SET_ZF(t);
  SET_NF(t);
  return t;
}

/**
 * @brief ASL A
 */
inline void Cpu::asl_a()
{
  a(asl(a()));
  tick(2);
}

/**
 * @brief ASL mem
 *
 * ASL and the other read-modify-write instructions contain a bug (wikipedia):
 *
 * --
 * The 6502's read-modify-write instructions perform one read and two write
 * cycles. First the unmodified data that was read is written back,and then
 * the modified data is written. This characteristic may cause issues by
 * twice accessing hardware that acts on a write. This anomaly continued
 * through the entire NMOS line,but was fixed in the CMOS derivatives,in
 * which the processor will do two reads and one write cycle.
 * --
 *
 * I have come across code that uses this side-effect as a feature,for
 * instance,the following instruction will acknowledge VIC interrupts
 * on the first write cycle:
 *
 * ASL $d019
 *
 * So.. we need to mimic the behaviour.
 */
inline void Cpu::asl_mem(uint16_t addr,uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  c64_->mem_->write_byte(addr,v);
  c64_->mem_->write_byte(addr,asl(v));
  tick(cycles);
}

/**
 * @brief Exclusive OR
 */
inline void Cpu::eor(uint8_t v,uint8_t cycles)
{
  a(a()^v);
  SET_ZF(a());
  SET_NF(a());
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

// Instructions: arithmetic operations  //////////////////////////////////////

/**
 * @brief INCrement
 */
inline void Cpu::inc(uint16_t addr,uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  c64_->mem_->write_byte(addr,v);
  v++;
  c64_->mem_->write_byte(addr,v);
  SET_ZF(v);
  SET_NF(v);
  tick(cycles);
}

/**
 * @brief DECrement
 */
inline void Cpu::dec(uint16_t addr,uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  c64_->mem_->write_byte(addr,v);
  v--;
  c64_->mem_->write_byte(addr,v);
  SET_ZF(v);
  SET_NF(v);
  tick(cycles);  // was missing
}

/**
 * @brief INcrement X
 */
inline void Cpu::inx()
{
  x_+=1;
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief INcrement Y
 */
inline void Cpu::iny()
{
  y_+=1;
  SET_ZF(y());
  SET_NF(y());
  tick(2);
}

/**
 * @brief DEcrement X
 */
inline void Cpu::dex()
{
  x_-=1;
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief DEcrement Y
 */
inline void Cpu::dey()
{
  y_-=1;
  SET_ZF(y());
  SET_NF(y());
  tick(2);
}

/**
 * @brief ADd with Carry
 */
inline void Cpu::adc(uint8_t v,uint8_t cycles)
{
  uint16_t t;
  if(dmf())
  {
    t = (a()&0xf) + (v&0xf) + (cf() ? 1 : 0);
    if (t > 0x09)
      t += 0x6;
    t += (a()&0xf0) + (v&0xf0);
    if((t & 0x1f0) > 0x90)
      t += 0x60;
  }
  else
  {
    t = a() + v + (cf() ? 1 : 0);
  }
  cf(t>0xff);
  t=t&0xff;
  of(!((a()^v)&0x80) && ((a()^t) & 0x80));
  SET_ZF(t);
  SET_NF(t);
  a((uint8_t)t);
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

/**
 * @brief SuBstract with Carry
 */
inline void Cpu::sbc(uint8_t v,uint8_t cycles)
{
  uint16_t t;
  if(dmf())
  {
    t = (a()&0xf) - (v&0xf) - (cf() ? 0 : 1);
    if((t & 0x10) != 0)
      t = ((t-0x6)&0xf) | ((a()&0xf0) - (v&0xf0) - 0x10);
    else
      t = (t&0xf) | ((a()&0xf0) - (v&0xf0));
    if((t&0x100)!=0)
      t -= 0x60;
  }
  else
  {
    t = a() - v - (cf() ? 0 : 1);
  }
  cf(t<0x100);
  t=t&0xff;
  of(((a()^t)&0x80) && ((a()^v) & 0x80));
  SET_ZF(t);
  SET_NF(t);
  a((uint8_t)t);
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

// Instructions: flag access /////////////////////////////////////////////////

/**
 * @brief SEt Interrupt flag
 */
inline void Cpu::sei()
{
  idf(true);
  tick(2);
}

/**
 * @brief CLear Interrupt flag
 */
inline void Cpu::cli()
{
  idf(false);
  tick(2);
}

/**
 * @brief SEt Carry flag
 */
inline void Cpu::sec()
{
  cf(true);
  tick(2);
}

/**
 * @brief CLear Carry flag
 */
inline void Cpu::clc()
{
  cf(false);
  tick(2);
}

/**
 * @brief SEt Decimal flag
 */
inline void Cpu::sed()
{
  dmf(true);
  tick(2);
}

/**
 * @brief CLear Decimal flag
 */
inline void Cpu::cld()
{
  dmf(false);
  tick(2);
}

/**
 * @brief CLear oVerflow flag
 */
inline void Cpu::clv()
{
  of(false);
  tick(2);
}

inline uint8_t Cpu::flags()
{
  uint8_t v=0;
  v |= cf()  << 0;
  v |= zf()  << 1;
  v |= idf() << 2;
  v |= dmf() << 3;
  /* brk & php instructions push the bcf flag active */
  v |= 1 << 4;
  /* unused,always set */
  v |= 1     << 5;
  v |= of()  << 6;
  v |= nf()  << 7;
  return v;
}

inline void Cpu::flags(uint8_t v)
{
  cf(ISSET_BIT(v,0));
  zf(ISSET_BIT(v,1));
  idf(ISSET_BIT(v,2));
  dmf(ISSET_BIT(v,3));
  of(ISSET_BIT(v,6));
  nf(ISSET_BIT(v,7));
}

/**
 * @brief PusH Processor flags
 */
inline void Cpu::php()
{
  push(flags());
  tick(3);
}

/**
 * @brief PuLl Processor flags
 */
inline void Cpu::plp()
{
  flags(pop());
  tick(4);
}

/**
 * @brief Jump to SubRoutine
 *
 * Note that JSR does not push the address of the next instruction
 * to the stack but the address to the last byte of its own
 * instruction.
 */
inline void Cpu::jsr()
{
  uint16_t addr = addr_abs();
  push(((pc()-1) >> 8) & 0xff);
  push(((pc()-1) & 0xff));
  pc(addr);
  tick(6);
}

/**
 * @brief JuMP
 */
inline void Cpu::jmp()
{
  uint16_t addr = addr_abs();
  pc(addr);
  tick(3);
}

/**
 * @brief JuMP (indirect)
 */
inline void Cpu::jmp_ind()
{
  uint16_t t = c64_->mem_->read_word(pc_);
  uint16_t abs_ = addr_abs(); /* pc += 2 */
  uint16_t addr = c64_->mem_->read_word(abs_);
  /* Introduce indirect JMP bug */
  addr = (((t&0xFF)==0xFF)?((t&0xFF00)|(addr&0xFF)):addr);
  pc(addr);
  // tick(3); // incorrect
  tick(5);
}

/**
 * @brief ReTurn from SubRoutine
 */
inline void Cpu::rts()
{
  uint16_t addr = (pop() + (pop() << 8)) + 1;
  pc(addr);
  tick(6);
}

/**
 * @brief CoMPare
 */
inline void Cpu::cmp(uint8_t v,uint8_t cycles)
{
  uint16_t t;
  t = a() - v;
  cf(t<0x100);
  t = t&0xff;
  SET_ZF(t);
  SET_NF(t);
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

/**
 * @brief CoMPare X
 */
inline void Cpu::cpx(uint8_t v,uint8_t cycles)
{
  uint16_t t;
  t = x() - v;
  cf(t<0x100);
  t = t&0xff;
  SET_ZF(t);
  SET_NF(t);
  tick(cycles);
}

/**
 * @brief CoMPare Y
 */
inline void Cpu::cpy(uint8_t v,uint8_t cycles)
{
  uint16_t t;
  t = y() - v;
  cf(t<0x100);
  t = t&0xff;
  SET_ZF(t);
  SET_NF(t);
  tick(cycles);
}

/**
 * @brief Branch if Not Equal
 */
inline void Cpu::bne()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(!zf()) {
    pc(addr);
    if(pb_crossed) tick(2);
    else tick(1);
  }
  tick(2);
}

/**
 * @brief Branch if Equal
 */
inline void Cpu::beq()
{
  uint16_t addr = (int8_t) fetch_op();
  curr_page = addr&0xff00;
  addr += pc();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  if(zf()) {
    pc(addr);
    if(pb_crossed) tick(2);
    else tick(1);
  }
  tick(2);
}

/**
 * @brief Branch if Carry is Set
 */
inline void Cpu::bcs()
{
  uint16_t addr = (int8_t) fetch_op();
  curr_page = addr&0xff00;
  addr += pc();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  if(cf()) {
    pc(addr);
    if(pb_crossed) tick(2);
    else tick(1);
  }
  tick(2);
}

/**
 * @brief Branch if Carry is Clear
 */
inline void Cpu::bcc()
{
  uint16_t addr = (int8_t) fetch_op();
  curr_page = addr&0xff00;
  addr += pc();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  if(!cf()) {
    pc(addr);
    if(pb_crossed) tick(2);
    else tick(1);
  }
  tick(2);
}

/**
 * @brief,Branch if PLus
 */
inline void Cpu::bpl()
{
  uint16_t addr = (int8_t) fetch_op();
  curr_page = addr&0xff00;
  addr += pc();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  if(!nf()) {
    pc(addr);
    if(pb_crossed) tick(2);
    else tick(1);
  }
  tick(2);
}

/**
 * @brief Branch if MInus
 */
inline void Cpu::bmi()
{
  uint16_t addr = (int8_t) fetch_op();
  curr_page = addr&0xff00;
  addr += pc();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  if(nf()) {
    pc(addr);
    if(pb_crossed) tick(2);
    else tick(1);
  }
  tick(2);
}

/**
 * @brief Branch if oVerflow Clear
 */
inline void Cpu::bvc()
{
  uint16_t addr = (int8_t) fetch_op();
  curr_page = addr&0xff00;
  addr += pc();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  if(!of()) {
    pc(addr);
    if(pb_crossed) tick(2);
    else tick(1);
  }
  tick(2);
}

/**
 * @brief Branch if oVerflow Set
 */
inline void Cpu::bvs()
{
  uint16_t addr = (int8_t) fetch_op();
  curr_page = addr&0xff00;
  addr += pc();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  if(of()) {
    pc(addr);
    if(pb_crossed) tick(2);
    else tick(1);
  }
  tick(2);
}

// misc //////////////////////////////////////////////////////////////////////

/**
 * @brief No OPeration
 */
inline void Cpu::nop(uint8_t cycles)
{
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

/**
 * @brief BReaKpoint
 */
inline void Cpu::brk()
{ /* ISSUE: BRK BUG DOES NOT WORK YET */
  push(((pc()+1) >> 8) & 0xff);
  push(((pc()+1) & 0xff));
  push(flags());
  pc(c64_->mem_->read_word(Memory::kAddrIRQVector));
  idf(true);
  bcf(true);
  tick(7);
}

/**
 * @brief ReTurn from Interrupt
 */
inline void Cpu::rti()
{
  flags(pop());
  pc(pop() + (pop() << 8));
  // tick(7); /* 7?? */
  tick(6);
}

// illegals  /////////////////////////////////////////////////////////////////

/**
 * @brief JAM / KILL / HALT
 *
 * @param insn
 */
inline void Cpu::jam(uint8_t insn)
{
  (void)insn;
  // if (1) return;
  // printf("JAM! INS: %02X PC @ %04X\nInstr %02X @ %04X\nNext Instr %02X @ %04X\nPrev Instr %02X @ %04X\nPrev Instr %02X @ %04X\nSP:%X A:%X X:%X Y:%X\n",
    // insn,pc(),
    // load_byte(pc_-1),pc_,load_byte(pc_),pc_+1,
    // load_byte(pc_-2),pc_-1,load_byte(pc_-3),pc_-2,
    // sp(),a(),x(),y()
    // );
  // dump_regs();
  // reset();
  // exit(1);
  // pc(pc_--);
  tick(1);
}

/**
 * @brief SLO (ASO) ASL oper + ORA oper
 *
 * @param addr
 * @param cycles_a
 * @param cycles_b
 */
inline void Cpu::slo(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  asl_mem(addr,cycles_a);
  ora(load_byte(addr),cycles_b);
}

inline void Cpu::lxa(uint8_t v,uint8_t cycles)
{
  uint8_t t = ((a() | 0xEE) & v);
  x(t);
  a(t);
  SET_ZF(t);
  SET_NF(t);
  tick(cycles);
}

inline void Cpu::anc(uint8_t v)
{
  _and(v,2);
  if(nf()) cf(true);
  else cf(false);
}

inline void Cpu::las(uint8_t v)
{ /* 4 + 1 cycles if page boundry is crossed */
  uint8_t t = (v & sp());
  a(t);
  x(t);
  sp(t);
  SET_NF(t);
  SET_ZF(t);
  tick(4);
  if(pb_crossed) tick(1);
}

inline void Cpu::lax(uint8_t v, uint8_t cycles)
{
  lda(v,cycles);
  tax();
}

inline void Cpu::sax(uint16_t addr,uint8_t cycles)
{
  uint8_t _a = a();
  uint8_t _x = x();
  uint8_t _r = (_a & _x);
  c64_->mem_->write_byte(addr,_r);
  tick(cycles);
}

inline void Cpu::shy(uint16_t addr,uint8_t cycles)
{
  uint8_t t = ((addr >> 8) + 1);
  uint8_t y_ = y();
  c64_->mem_->write_byte(addr,(y_ & t));
  tick(cycles);
}

inline void Cpu::shx(uint16_t addr,uint8_t cycles)
{
  uint8_t t = ((addr >> 8) + 1);
  uint8_t x_ = x();
  c64_->mem_->write_byte(addr,(x_ & t));
  tick(cycles);
}

inline void Cpu::sha(uint16_t addr,uint8_t cycles)
{
  uint8_t t = ((addr >> 8) + 1);
  uint8_t a_ = a();
  uint8_t x_ = x();
  c64_->mem_->write_byte(addr,((a_ & x_) & t));
  tick(cycles);
}

inline void Cpu::sre(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  uint16_t t = addr;
  lsr_mem(t,cycles_a);
  eor(load_byte(t),cycles_b);
}

inline void Cpu::rla(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  uint16_t t = addr;
  rol_mem(t,cycles_a);
  _and(load_byte(t),cycles_b);
}

inline void Cpu::rla_(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  // uint16_t t = addr;
  // rol_mem(t,cycles_a);
  // _and(load_byte(t),cycles_b);

  // ROL_MEM
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  c64_->mem_->write_byte(addr,v);
  // ROL
  // c64_->mem_->write_byte(addr,rol(v));
  uint16_t t = (v << 1) | (uint8_t)cf();
  cf((t&0x100)!=0);
  SET_ZF(t);
  SET_NF(t);
  // return (uint8_t)t;
  c64_->mem_->write_byte(addr,(uint8_t)t);
  tick(cycles_a);

  // AND
  v = load_byte(addr);
  a(a()&v);
  SET_ZF(a());
  SET_NF(a());
  if(pb_crossed)cycles_b+=1;
  tick(cycles_b);
}

inline void Cpu::rra(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  uint16_t t = addr;
  ror_mem(t,cycles_a);
  adc(load_byte(t),cycles_b);
}

inline void Cpu::dcp(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  uint16_t t = addr;
  dec(t,cycles_a);
  cmp(load_byte(t),cycles_b);
}

inline void Cpu::tas(uint16_t addr,uint8_t cycles)
{
  /* and accu, x and (highbyte + 1) of address */
  uint8_t v = (((a() & x()) & ((addr >> 8) + 1)));

  unsigned int tmp2 = (addr + y());
  if (((addr & 0xff) + y()) > 0xff) {
    tmp2 = ((tmp2 & 0xff) | (v << 8));
    /* write result to address */
    c64_->mem_->write_byte(tmp2, v);
  } else {
    /* write result to address */
    c64_->mem_->write_byte(addr, v);
  }
  sp(a()&x()); /* write a & x to stackpointer unchanged */
  tick(cycles);
}

inline void Cpu::sbx(uint8_t v,uint8_t cycles)
{
  uint8_t a_ = a();
  uint8_t x_ = x();
  uint8_t r_ = a_ & x_;
  uint16_t t = r_ - v;
  cf(t<0x100);
  t = t&0xff;
  SET_ZF(t);
  SET_NF(t);
  x(t);
  tick(cycles);
}

inline void Cpu::isc(uint16_t addr,uint8_t cycles)
{
  inc(addr,cycles-=2);
  uint8_t v = load_byte(addr);
  sbc(v,cycles);
}

inline void Cpu::arr()
{ /* Fixed code with courtesy of Vice 6510core.c */
  unsigned int tmp = (a() & (fetch_op()));
  if(dmf()) {
    int tmp2 = tmp;
    tmp2 |= ((flags() & SR_CARRY) << 8);
    tmp2 >>= 1;
    // compiler integer in boolean situation warning/error
    // nf(((flags() & SR_CARRY)?0x80:0)); /* set signed on negative flag */
    if (flags() & SR_CARRY) {
      nf(0x80); /* set signed on negative flag */
    } else {
      nf(0); /* unset signed on negative flag */
    }
    SET_ZF(tmp2);
    of((tmp2 ^ tmp) & 0x40);
    if (((tmp & 0xf) + (tmp & 0x1)) > 0x5) {
      tmp2 = (tmp2 & 0xf0) | ((tmp2 + 0x6) & 0xf);
    }
    if (((tmp & 0xf0) + (tmp & 0x10)) > 0x50) {
      tmp2 = (tmp2 & 0x0f) | ((tmp2 + 0x60) & 0xf0);
      cf(true);
    } else {
      cf(false);
    }
    a(tmp2);
  } else {
    tmp |= ((flags() & SR_CARRY) << 8);
    tmp >>= 1;
    SET_ZF(tmp);
    SET_NF(tmp);
    cf((tmp & 0x40));
    of((tmp & 0x40) ^ ((tmp & 0x20) << 1));
    a(tmp);
  }
  tick(2);
}

inline void Cpu::xaa(uint8_t v)
{
  uint8_t t = ((a() | ANE_MAGIC) & x() & ((uint8_t)(v)));
  a(t);
  SET_ZF(t);
  SET_NF(t);
  tick(2);
}

// interrupts  ///////////////////////////////////////////////////////////////

/**
 * @brief Interrupt ReQuest
 */
void Cpu::irq()
{
  if(!idf())
  {
    push(((pc()) >> 8) & 0xff);
    push(((pc()) & 0xff));
    /* push flags with bcf cleared */
    push((flags()&0xef));
    pc(c64_->mem_->read_word(Memory::kAddrIRQVector));
    idf(true);
    tick(7);
  }
}

/**
 * @brief Non Maskable Interrupt
 */
void Cpu::nmi()
{
  push(((pc()) >> 8) & 0xff);
  push(((pc()) & 0xff));
  /* push flags with bcf cleared */
  push((flags() & 0xef));
  pc(c64_->mem_->read_word(Memory::kAddrNMIVector));
  tick(7);
}

// debugging /////////////////////////////////////////////////////////////////

inline void Cpu::dump_flags()
{
  D("FLAGS: %02X %d%d%d%d%d%d%d%d\n",
    flags(),
    (flags()&SR_NEGATIVE)>>7,
    (flags()&SR_OVERFLOW)>>6,
    (flags()&SR_UNUSED)>>5,
    (flags()&SR_BREAK)>>4,
    (flags()&SR_DECIMAL)>>3,
    (flags()&SR_INTERRUPT)>>2,
    (flags()&SR_ZERO)>>1,
    (flags()&SR_CARRY)
  );
}

inline void Cpu::dump_flags(uint8_t flags)
{
  D("FLAGS: %02X %d%d%d%d%d%d%d%d\n",
    flags,
    (flags&SR_NEGATIVE)>>7,
    (flags&SR_OVERFLOW)>>6,
    (flags&SR_UNUSED)>>5,
    (flags&SR_BREAK)>>4,
    (flags&SR_DECIMAL)>>3,
    (flags&SR_INTERRUPT)>>2,
    (flags&SR_ZERO)>>1,
    (flags&SR_CARRY)
  );
}

inline void Cpu::dump_regs()
{
  std::stringstream sflags;
  if(cf())  sflags << "CF ";
  if(zf())  sflags << "ZF ";
  if(idf()) sflags << "IDF ";
  if(dmf()) sflags << "DMF ";
  if(bcf()) sflags << "BCF ";
  if(of())  sflags << "OF ";
  if(nf())  sflags << "NF ";

  std::stringstream pflags;
  if(nf())  pflags << "1";
  else  pflags << "0";
  if(of())  pflags << "1";
  else  pflags << "0";
  pflags << "-";
  if(bcf()) pflags << "1";
  else  pflags << "0";
  if(dmf()) pflags << "1";
  else  pflags << "0";
  if(idf()) pflags << "1";
  else  pflags << "0";
  if(zf())  pflags << "1";
  else  pflags << "0";
  if(cf())  pflags << "1";
  else  pflags << "0";

  D("PC=%04x M=%04X,A=%02x X=%02x Y=%02x SP=%02x NV-BDIZC: %s flags= %s\n",
    pc(),load_word(pc()),a(),x(),y(),sp(),pflags.str().c_str(),sflags.str().c_str());
}

inline void Cpu::dump_regs_insn(uint8_t insn)
{
  static uint prev_cycles = cycles();
  D("INSN=%02X '%-9s' ADDR: $%04X VAL: $%02X CYC=%u ",
    insn,
    opcodenames[insn],
    d_address,
    c64_->mem_->read_byte_no_io(d_address),
    (cycles()-prev_cycles));
  dump_regs();
  prev_cycles = cycles();
}

inline void Cpu::dump_regs_json()
{
  D("{");
  D("\"pc\":%d,",pc());
  D("\"a\":%d,",a());
  D("\"x\":%d,",x());
  D("\"y\":%d,",y());
  D("\"sp\":%d",sp());
  D("}\n");
}

inline void Cpu::dbg()
{
  D("INS %02X: %02X %02X %04X\n",load_byte(pc_-1),load_byte(pc_),load_byte(pc_+1),pc_);
  // printf("INS-1 %02X: %02X %02X %04X\n",load_byte(pc_-2),load_byte(pc_-1),load_byte(pc_+2),pc_-1);
  // printf("INS-2 %02X: %02X %02X %04X\n",load_byte(pc_-3),load_byte(pc_-2),load_byte(pc_+3),pc_-2);
}

   void Cpu::dbg_a()
{
  dump_regs();
  dbg();
}

inline void Cpu::dbg_b()
{
  dbg();
  dump_regs();
}
