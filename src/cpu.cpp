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
#include <USBSID.h>

Cpu::Cpu(C64 * c64) :
  c64_(c64)
{
 if (LOG_ILLEGALS == 1) logillegals = true;
}

/**
 * @brief Pre define static CPU cycles variable
 *
 */
unsigned int Cpu::cycles_ = 0;
bool Cpu::loginstructions = false;
bool Cpu::logillegals = false;

/**
 * @brief Cold reset
 *
 * https://www.c64-wiki.com/index.php/Reset_(Process)
 */
void Cpu::reset()
{
  a_ = x_ = y_ = sp_ = 0;
  cf_ = zf_ = idf_ = dmf_ = bcf_ = of_ = nf_ = false;
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
{ /* TODO: Double check cycles! */
  /* fetch instruction */
  uint8_t insn = fetch_op();
  pb_crossed = false;
  unsigned int __c = cycles();
  /* D("START INSN: %02X C1:%u\n",insn,__c); */
  bool retval = true;
  bool ill = logillegals; /* set to true for illegal instruction logging */
  bool linst = loginstructions;
  if (loginstructions) { dump_regs_insn(insn); }
  /* emulate instruction */
  switch(insn)
  {
    case 0x00: /* BRK impl */
      brk();
      break;
    case 0x01: /* ORA (ind,X) */
      ora(load_byte(addr_indx()),6);
      break;
    case 0x02: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0x03: /* SLO (ind,X) ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      slo(addr_indx(),5,3);
      break;
    case 0x04: /* NOP zpg ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_zero());
      nop(3);
      break;
    case 0x05: /* ORA zpg */
      ora(load_byte(addr_zero()),3);
      break;
    case 0x06: /* ASL zpg */
      asl_mem(addr_zero(),5);
      break;
    case 0x07: /* SLO zpg ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      slo(addr_zero(),3,2);
      break;
    case 0x08: /* PHP impl */
      php();
      break;
    case 0x09: /* ORA #imm */
      ora(fetch_op(),2);
      break;
    case 0x0A: /* ASL A */
      asl_a();
      break;
    case 0x0B: /* ANC(AAC) #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      _and(fetch_op(),2);
      if(nf()) cf(true);
      else cf(false);
      break;
    case 0x0C: /* NOP abs  ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_abs());
      nop(4);
      break;
    case 0x0D: /* ORA abs */
      ora(load_byte(addr_abs()),4);
      break;
    case 0x0E: /* ASL abs */
      asl_mem(addr_abs(),6);
      break;
    case 0x0F: /* SLO abs ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      slo(addr_abs(),3,3);
      break;
    case 0x10: /* BPL rel */
      bpl();
      break;
    case 0x11: /* ORA (ind),Y */
      ora(load_byte(addr_indy()),5);
      break;
    case 0x12: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0x13: /* SLO (ind),Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      slo(addr_indy(),5,3);
      break;
    case 0x14: /* NOP zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_zerox());
      nop(4);
      break;
    case 0x15: /* ORA zpg,X */
      ora(load_byte(addr_zerox()),4);
      break;
    case 0x16: /* ASL zpg,X */
      asl_mem(addr_zerox(),6);
      break;
    case 0x17: /* SLO zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      slo(addr_zerox(),2,3);
      break;
    case 0x18: /* CLC impl */
      clc();
      break;
    case 0x19: /* ORA abs,Y */
      ora(load_byte(addr_absy()),4);
      break;
    case 0x1A: /* NOP impl ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      nop(2);
      break;
    case 0x1B: /* SLO abs,Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      slo(addr_absy(),5,2);
      break;
    case 0x1C: /* NOP abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_absx());
      nop(4);
      break;
    case 0x1D: /* ORA abs,X */
      ora(load_byte(addr_absx()),4);
      break;
    case 0x1E: /* ASL abs,X */
      asl_mem(addr_absx(),7);
      break;
    case 0x1F: /* SLO abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      slo(addr_absx(),5,2);
      break;
    case 0x20: /* JSR abs */
      jsr();
      break;
    case 0x21: /* AND (ind,X) */
      _and(load_byte(addr_indx()),6);
      break;
    case 0x22: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0x23: /* RLA (ind,X) ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rla(addr_indx(),5,3);
      break;
    case 0x24: /* BIT zpg */
      bit(addr_zero(),3);
      break;
    case 0x25: /* AND zpg */
      _and(load_byte(addr_zero()),3);
      break;
    case 0x26: /* ROL zpg */
      rol_mem(addr_zero(),5);
      break;
    case 0x27: /* RLA zp ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rla(addr_zero(),2,3);
      break;
    case 0x28: /* PLP impl */
      plp();
      break;
    case 0x29: /* AND #imm */
      _and(fetch_op(),2);
      break;
    case 0x2A: /* ROL A */
      rol_a();
      break;
    case 0x2B: /* ANC(AAC) #imm ~ Same as 0x0B ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      _and(fetch_op(),0);
      if(nf()) cf(true);
      else cf(false);
      break;
    case 0x2C: /* BIT abs */
      bit(addr_abs(),4);
      break;
    case 0x2D: /* AND abs */
      _and(load_byte(addr_abs()),4);
      break;
    case 0x2E: /* ROL abs */
      rol_mem(addr_abs(),6);
      break;
    case 0x2F: /* RLA abs ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rla(addr_abs(),4,2);
      break;
    case 0x30: /* BMI rel */
      bmi();
      break;
    case 0x31: /* AND (ind),Y */
      _and(load_byte(addr_indy()),5);
      break;
    case 0x32: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0x33: /* RLA (ind),Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rla(addr_indy(),5,3);
      break;
    case 0x34: /* NOP zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_zerox());
      nop(4);
      break;
    case 0x35: /* AND zpg,X */
      _and(load_byte(addr_zerox()),4);
      break;
    case 0x36: /* ROL zpg,X */
      rol_mem(addr_zerox(),6);
      break;
    case 0x37: /* RLA zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rla(addr_zerox(),3,3);
      break;
    case 0x38: /* SEC impl */
      sec();
      break;
    case 0x39: /* AND abs,Y */
      _and(load_byte(addr_absy()),4);
      break;
    case 0x3A: /* NOP impl ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      nop(2);
      break;
    case 0x3B: /* RLA abs,Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rla(addr_absy(),3,3);
      break;
    case 0x3C: /* NOP abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_absx());
      nop(4);
      break;
    case 0x3D: /* AND abs,X */
      _and(load_byte(addr_absx()),4);
      break;
    case 0x3E: /* ROL abs,X */
      rol_mem(addr_absx(),7);
      break;
    case 0x3F: /* RLA abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rla(addr_absx(),5,3);
      break;
    case 0x40: /* RTI impl (6) */
      rti();
      break;
    case 0x41: /* EOR (ind,X) */
      eor(load_byte(addr_indx()),6);
      break;
    case 0x42: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0x43: /* SRE (ind,X) ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sre(addr_indx(),6,2);
      break;
    case 0x44:  /* NOP nn ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_zero());
      nop(3);
      break;
    case 0x45: /* EOR nn */
      eor(load_byte(addr_zero()),3);
      break;
    case 0x46: /* LSR nn */
      lsr_mem(addr_zero(),5);
      break;
    case 0x47: /* SRE nn ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sre(addr_zero(),3,2);
      break;
    case 0x48: /* PHA */
      pha();
      break;
    case 0x49: /* EOR #imm */
      eor(fetch_op(),2);
      break;
    case 0x4A: /* LSR A */
      lsr_a();
      break;
    case 0x4B: /* ALR #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      _and(fetch_op(),0);
      lsr_a();
      break;
    case 0x4C: /* JMP abs */
      jmp();
      break;
    case 0x4D: /* EOR abs */
      eor(load_byte(addr_abs()),4);
      break;
    case 0x4E: /* LSR abs */
      lsr_mem(addr_abs(),6);
      break;
    case 0x4F: /* SRE abs ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sre(addr_abs(),3,2);
      break;
    case 0x50: /* BVC */
      bvc();
      break;
    case 0x51: /* EOR (ind),Y */
      eor(load_byte(addr_indy()),5);
      break;
    case 0x52: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0x53: /* SRE (ind),Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sre(addr_indy(),6,2);
      break;
    case 0x54: /* NOP zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_zerox());
      nop(4);
      break;
    case 0x55: /* EOR zpg,X */
      eor(load_byte(addr_zerox()),4);
      break;
    case 0x56: /* LSR zpg,X */
      lsr_mem(addr_zerox(),6);
      break;
    case 0x57: /* SRE zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sre(addr_zerox(),3,2);
      break;
    case 0x58: /* CLI */
      cli();
      break;
    case 0x59: /* EOR abs,Y */
      eor(load_byte(addr_absy()),4);
      break;
    case 0x5A: /* NOP ~ Illegal OPCode */
      nop(2);
      break;
    case 0x5B: /* SRE abs,Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sre(addr_absy(),3,2);
      break;
    case 0x5C: /* NOP abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_absx());
      nop(4);
      break;
    case 0x5D: /* EOR abs,X */
      eor(load_byte(addr_absx()),4);
      break;
    case 0x5E: /* LSR abs,X */
      lsr_mem(addr_absx(),7);
      break;
    case 0x5F: /* SRE abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sre(addr_absx(),3,2);
      break;
    case 0x60: /* RTS */
      rts();
      break;
    case 0x61: /* ADC (ind,X) */
      adc(load_byte(addr_indx()),6);
      break;
    case 0x62: /* JAM ~ Illegal OPCode */
      jam(insn);
      break; // NOTICE: BREAKS FANTA IN SPACE
    case 0x63: /* RRA (ind,X) ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rra(addr_indx(),4,4);
      break;
    case 0x64: /* NOP nn ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_zero());
      nop(3);
      break;
    case 0x65: /* ADC nn */
      adc(load_byte(addr_zero()),3);
      break;
    case 0x66: /* ROR nn */
      ror_mem(addr_zero(),5);
      break;
    case 0x67: /* RRA nn ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rra(addr_zero(),4,4);
      break;
    case 0x68: /* PLA */
      pla();
      break;
    case 0x69: /* ADC #imm */
      adc(fetch_op(),2);
      break;
    case 0x6A: /* ROR A */
      ror_a();
      break;
    case 0x6B: /* ARR #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      arr();
      break;
    case 0x6C: /* JMP (nnnn) */
      jmp_ind();
      break;
    case 0x6D: /* ADC abs */
      adc(load_byte(addr_abs()),4);
      break;
    case 0x6E: /* ROR abs */
      ror_mem(addr_abs(),6);
      break;
    case 0x6F: /* RRA abs ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rra(addr_abs(),3,3);
      break;
    case 0x70: /* BVS */
      bvs();
      break;
    case 0x71: /* ADC (ind),Y */
      adc(load_byte(addr_indy()),5);
      break;
    case 0x72: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0x73: /* RRA (ind),Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rra(addr_indy(),4,4);
      break;
    case 0x74: /* NOP zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_zerox());
      nop(4);
      break;
    case 0x75: /* ADC zpg,X */
      adc(load_byte(addr_zerox()),4);
      break;
    case 0x76: /* ROR zpg,X */
      ror_mem(addr_zerox(),6);
      break;
    case 0x77: /* RRA zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rra(addr_zerox(),4,4);
      break;
    case 0x78: /* SEI */
      sei();
      break;
    case 0x79: /* ADC abs,Y */
      adc(load_byte(addr_absy()),4);
      break;
    case 0x7A: /* NOP ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      nop(2);
      break;
    case 0x7B: /* RRA abs,Y (7) ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rra(addr_absy(),4,3);
      break;
    case 0x7C: /* NOP abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_absx());
      nop(4);
      break;
    case 0x7D: /* ADC abs,X */
      adc(load_byte(addr_absx()),4);
      break;
    case 0x7E: /* ROR abs,X */
      ror_mem(addr_absx(),7);
      break;
    case 0x7F: /* RRA abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      rra(addr_absx(),4,3);
      break;
    case 0x80: /* NOP #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      fetch_op();
      nop(2);
      break;
    case 0x81: /* STA (ind,X) */
      sta(addr_indx(),6);
      break;
    case 0x82: /* NOP #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      fetch_op();
      nop(2);
      break;
    case 0x83: /* SAX (ind,X) ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sax(addr_indx(),6);
      break;
    case 0x84: /* STY nn */
      sty(addr_zero(),3);
      break;
    case 0x85: /* STA nn */
      sta(addr_zero(),3);
      break;
    case 0x86: /* STX nn */
      stx(addr_zero(),3);
      break;
    case 0x87: /* SAX nn ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sax(addr_zero(),3);
      break;
    case 0x88: /* DEY */
      dey(); break;
    case 0x89: /* NOP #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      fetch_op();
      nop(2);
      break;
    case 0x8A: /* TXA */
      txa();
      break;
    case 0x8B: /* XAA(ANE) #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      xaa(load_byte(addr_zero()));
      break;
    case 0x8C: /* STY abs */
      sty(addr_abs(),4);
      break;
    case 0x8D: /* STA abs */
      sta(addr_abs(),4);
      break;
    case 0x8E: /* STX abs */
      stx(addr_abs(),4);
      break;
    case 0x8F: /* SAX abs ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sax(addr_abs(),4);
      break;
    case 0x90: /* BCC nn */
      bcc();
      break;
    case 0x91: /* STA (ind),Y */
      sta(addr_indy(),6);
      break;
    case 0x92: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0x93: /* SHA (ind),Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sha(addr_indy(),6);
      break;
    case 0x94: /* STY zpg,X */
      sty(addr_zerox(),4);
      break;
    case 0x95: /* STA zpg,X */
      sta(addr_zerox(),4);
      break;
    case 0x96: /* STX zpg,Y */
      stx(addr_zeroy(),4);
      break;
    case 0x97: /* SAX zpg,Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sax(addr_zeroy(),4);
      break;
    case 0x98: /* TYA */
      tya();
      break;
    case 0x99: /* STA abs,Y */
      sta(addr_absy(),5);
      break;
    case 0x9A: /* TXS */
      txs();
      break;
    case 0x9B: /* TAS (XAS,SHS) abs,Y ~ Illegal OPCode */ /* NOTICE: UNSTABLE,IGNORED! */
      if (ill) { dump_regs_insn(insn); }
      tas(addr_absy(),5);
      break;
    case 0x9C: /* SHY abs,X ~ Illegal OPCode */ /* NOTICE: UNSTABLE! */
      if (ill) { dump_regs_insn(insn); }
      shy(addr_absx(),5);
      break;
    case 0x9D: /* STA abs,X */
      sta(addr_absx(),5);
      break;
    case 0x9E: /* SHX abs,Y ~ Illegal OPCode */ /* NOTICE: UNSTABLE! */
      if (ill) { dump_regs_insn(insn); }
      shx(addr_absy(),5);
      break;
    case 0x9F: /* SHA abs,Y ~ Illegal OPCode */ /* NOTICE: UNSTABLE! */
      if (ill) { dump_regs_insn(insn); }
      sha(addr_absy(),5);
      break;
    case 0xA0: /* LDY #imm */
      ldy(fetch_op(),2);
      break;
    case 0xA1: /* LDA (ind,X) */
      lda(load_byte(addr_indx()),6);
      break;
    case 0xA2: /* LDX #imm */
      ldx(fetch_op(),2);
      break;
    case 0xA3: /* LAX (ind,X) 6 ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      lda(load_byte(addr_indx()),4);
      tax();
      break;
    case 0xA4: /* LDY nn */
      ldy(load_byte(addr_zero()),3);
      break;
    case 0xA5: /* LDA nn */
      lda(load_byte(addr_zero()),3);
      break;
    case 0xA6: /* LDX nn */
      ldx(load_byte(addr_zero()),3);
      break;
    case 0xA7: /* LAX nn ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      lda(load_byte(addr_zero()),1);
      tax();
      break;
    case 0xA8: /* TAY */
      tay();
      break;
    case 0xA9: /* LDA #imm */
      lda(fetch_op(),2);
      break;
    case 0xAA: /* TAX */
      tax();
      break;
    case 0xAB: /* LXA #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      lxa(fetch_op(),2);
      break;
    case 0xAC: /* LDY abs */
      ldy(load_byte(addr_abs()),4);
      break;
    case 0xAD: /* LDA abs */
      lda(load_byte(addr_abs()),4);
      break;
    case 0xAE: /* LDX abs */
      ldx(load_byte(addr_abs()),4);
      break;
    case 0xAF: /* LAX abs ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      lda(load_byte(addr_abs()),2);
      tax();
      break;
    case 0xB0: /* BCS nn */
      bcs();
      break;
    case 0xB1: /* LDA (ind),Y */
      lda(load_byte(addr_indy()),5);
      break;
    case 0xB2: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0xB3: /* LAX (ind),Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      lda(load_byte(addr_indy()),4);
      tax();
      break;
    case 0xB4: /* LDY zpg,X */
      // ldy(load_byte(addr_zerox()),3); // incorrect
      ldy(load_byte(addr_zerox()),4);
      break;
    case 0xB5: /* LDA zpg,X */
      // lda(load_byte(addr_zerox()),3); // incorrect
      lda(load_byte(addr_zerox()),4);
      break;
    case 0xB6: /* LDX zpg,Y */
      // ldx(load_byte(addr_zeroy()),3); // incorrect
      ldx(load_byte(addr_zeroy()),4);
      break;
    case 0xB7: /* LAX zpg,Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      lda(load_byte(addr_zeroy()),2);
      tax();
      break;
    case 0xB8: /* CLV */
      clv();
      break;
    case 0xB9: /* LDA abs,Y */
      lda(load_byte(addr_absy()),4);
      break;
    case 0xBA: /* TSX */
      tsx();
      break;
    case 0xBB: /* LAS abs,Y (4+1) ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      las(load_byte(addr_absy()));
      break;
    case 0xBC: /* LDY abs,X */
      ldy(load_byte(addr_absx()),4);
      break;
    case 0xBD: /* LDA abs,X */
      lda(load_byte(addr_absx()),4);
      break;
    case 0xBE: /* LDX abs,Y */
      ldx(load_byte(addr_absy()),4);
      break;
    case 0xBF: /* LAX abs,Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      lda(load_byte(addr_absy()),3);
      tax(); /* 2 cycles */
      break;
    case 0xC0: /* CPY #imm */
      cpy(fetch_op(),2);
      break;
    case 0xC1: /* CMP (ind,X) */
      cmp(load_byte(addr_indx()),6);
      break;
    case 0xC2: /* NOP #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      fetch_op();
      nop(2); break;
    case 0xC3: /* DCP (ind,X) ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      dcp(addr_indx(),4,4);
      break;
    case 0xC4: /* CPY nn */
      cpy(load_byte(addr_zero()),3);
      break;
    case 0xC5: /* CMP nn */
      cmp(load_byte(addr_zero()),3);
      break;
    case 0xC6: /* DEC nn */
      dec(addr_zero(),5);
      break;
    case 0xC7: /* DCP nn ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      dcp(addr_zero(),4,4);
      break;
    case 0xC8: /* INY */
      iny();
      break;
    case 0xC9: /* CMP #imm */
      cmp(fetch_op(),2);
      break;
    case 0xCA: /* DEX */
      dex();
      break;
    case 0xCB: /* SBX #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sbx(fetch_op(),2);
      break;
    case 0xCC: /* CPY abs */
      cpy(load_byte(addr_abs()),4);
      break;
    case 0xCD: /* CMP abs */
      cmp(load_byte(addr_abs()),4);
      break;
    case 0xCE: /* DEC abs */
      dec(addr_abs(),6);
      break;
    case 0xCF: /* DCP abs ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      dcp(addr_abs(),3,3);
      break;
    case 0xD0: /* BNE nn */
      bne();
      break;
    case 0xD1: /* CMP (ind),Y */
      cmp(load_byte(addr_indy()),5);
      break;
    case 0xD2: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0xD3: /* DCP (ind),Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      dcp(addr_indy(),4,4);
      break;
    case 0xD4: /* NOP zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_zerox());
      nop(4);
      break;
    case 0xD5: /* CMP zpg,X */
      cmp(load_byte(addr_zerox()),4);
      break;
    case 0xD6: /* DEC zpg,X */
      dec(addr_zerox(),6);
      break;
    case 0xD7: /* DCP ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      dcp(addr_zerox(),4,2);
      break;
    case 0xD8: /* CLD */
      cld();
      break;
    case 0xD9: /* CMP abs,Y */
      cmp(load_byte(addr_absy()),4);
      break;
    case 0xDA: /* NOP ~ Illegal OPCode */
      nop(2);
      break;
    case 0xDB: /* DCP abs,Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      dcp(addr_absy(),4,3);
      break;
    case 0xDC: /* NOP abs,X ~ Illegal OPCode */
      load_byte(addr_absx());
      nop(4);
      break;
    case 0xDD: /* CMP abs,X */
      cmp(load_byte(addr_absx()),4);
      break;
    case 0xDE: /* DEC abs,X */
      dec(addr_absx(),7);
      break;
    case 0xDF: /* DCP abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      dcp(addr_absx(),4,3);
      break;
    case 0xE0: /* CPX #imm */
      cpx(fetch_op(),2);
      break;
    case 0xE1: /* SBC (ind,X) */
      sbc(load_byte(addr_indx()),6);
      break;
    case 0xE2: /* NOP #imm ~ Illegal OPCode */
      fetch_op();
      nop(2);
      break;
    case 0xE3: /* ISC (ind,X) ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      isc(addr_indx(),8);
      break;
    case 0xE4: /* CPX nn */
      cpx(load_byte(addr_zero()),3);
      break;
    case 0xE5: /* SBC nn */
      sbc(load_byte(addr_zero()),3);
      break;
    case 0xE6: /* INC nn */
      inc(addr_zero(),5);
      break;
    case 0xE7: /* ISC nn ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      isc(addr_zero(),5);
      break;
    case 0xE8: /* INX */
      inx();
      break;
    case 0xE9: /* SBC #imm */
      sbc(fetch_op(),2);
      break;
    case 0xEA: /* NOP */
      nop(2);
      break;
    case 0xEB: /* USBC #imm ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      sbc(fetch_op(),2);
      break;
    case 0xEC: /* CPX abs */
      cpx(load_byte(addr_abs()),4);
      break;
    case 0xED: /* SBC abs */
      sbc(load_byte(addr_abs()),4);
      break;
    case 0xEE: /* INC abs */
      inc(addr_abs(),6);
      break;
    case 0xEF: /* ISC abs ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      isc(addr_abs(),6);
      break;
    case 0xF0: /* BEQ nn */
      beq();
      break;
    case 0xF1: /* SBC (ind),Y */
      sbc(load_byte(addr_indy()),5);
      break;
    case 0xF2: /* JAM ~ Illegal OPCode */
      jam(insn);
      break;
    case 0xF3: /* ISC (ind),Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      isc(addr_indy(),8);
      break;
    case 0xF4: /* NOP zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_zerox());
      nop(4);
      break;
    case 0xF5: /* SBC zpg,X */
      sbc(load_byte(addr_zerox()),4);
      break;
    case 0xF6: /* INC zpg,X */
      inc(addr_zerox(),6);
      break;
    case 0xF7: /* ISC zpg,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      isc(addr_zerox(),6);
      break;
    case 0xF8: /* SED */
      sed();
      break;
    case 0xF9: /* SBC abs,Y */
      sbc(load_byte(addr_absy()),4);
      break;
    case 0xFA: /* NOP ~ Illegal OPCode */
      nop(2); break;
    case 0xFB: /* ISC abs,Y ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      isc(addr_absy(),7);
      break;
    case 0xFC: /* NOP abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      load_byte(addr_absx());
      nop(4);
      break;
    case 0xFD: /* SBC abs,X */
      sbc(load_byte(addr_absx()),4);
      break;
    case 0xFE: /* INC abs,X */
      inc(addr_absx(),7);
      break;
    case 0xFF: /* ISC abs,X ~ Illegal OPCode */
      if (ill) { dump_regs_insn(insn); }
      isc(addr_absx(),7);
      break;
    /* Anything else */
    default:
      D("Unknown instruction: %X at %04x,previous 2: %X(%04x) %X(%04x)\n",
        insn,pc(),
        load_byte(pc_-1),pc_-1,
        load_byte(pc_-2),pc_-2
      );
      break;
    retval = false;
  }
  /* D("C2:%u DIFF:%u PBC?%d END\n",
    cycles(),cycles()-__c,pb_crossed); */
  pb_crossed = false;
  return retval;
}

// helpers ///////////////////////////////////////////////////////////////////
static unsigned short d_address;

uint8_t Cpu::load_byte(uint16_t addr)
{
  d_address = addr;
  return c64_->mem_->read_byte(addr);
}

void Cpu::push(uint8_t v)
{
  uint16_t addr = Memory::kBaseAddrStack+sp_;
  d_address = addr;
  c64_->mem_->write_byte(addr,v);
  sp_--;
}

uint8_t Cpu::pop()
{
  uint16_t addr = ++sp_+Memory::kBaseAddrStack;
  d_address = addr;
  return load_byte(addr);
}

uint8_t Cpu::fetch_op()
{
  return load_byte(pc_++);
}

uint16_t Cpu::fetch_opw()
{
  uint16_t retval = c64_->mem_->read_word(pc_);
  pc_+=2;
  return retval;
}

uint16_t Cpu::addr_zero()
{
  uint16_t addr = fetch_op();
  d_address = addr;
  return addr;
}

uint16_t Cpu::addr_zerox()
{
  /* wraps around the zeropage */
  uint16_t addr = (fetch_op() + x()) & 0xff;
  d_address = addr;
  return addr;
}

uint16_t Cpu::addr_zeroy()
{
  /* wraps around the zeropage */
  uint16_t addr = (fetch_op() + y()) & 0xff;
  d_address = addr;
  return addr;
}

uint16_t Cpu::addr_abs()
{
  uint16_t addr = fetch_opw();
  d_address = addr;
  return addr;
}

uint16_t Cpu::addr_absy()
{
  uint16_t addr = fetch_opw();
  curr_page = addr&0xff00;
  addr += y();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  d_address = addr;
  return addr;
}

uint16_t Cpu::addr_absx()
{
  uint16_t addr = fetch_opw();
  curr_page = addr&0xff00;
  addr += x();
  if ((addr&0xff00)>curr_page) pb_crossed = true;
  d_address = addr;
  return addr;
}

uint16_t Cpu::addr_indx()
{
  /* wraps around the zeropage */
  uint16_t addr = c64_->mem_->read_word((addr_zero() + x()) & 0xff);
  d_address = addr;
  return addr;
}

uint16_t Cpu::addr_indy()
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
void Cpu::sta(uint16_t addr,uint8_t cycles)
{
  c64_->mem_->write_byte(addr,a());
  tick(cycles);
}

/**
 * @brief STore X
 */
void Cpu::stx(uint16_t addr,uint8_t cycles)
{
  c64_->mem_->write_byte(addr,x());
  tick(cycles);
}

/**
 * @brief STore Y
 */
void Cpu::sty(uint16_t addr,uint8_t cycles)
{
  c64_->mem_->write_byte(addr,y());
  tick(cycles);
}

/**
 * @brief Transfer X to Stack pointer
 */
void Cpu::txs()
{
  sp(x());
  tick(2);
}

/**
 * @brief Transfer Stack pointer to X
 */
void Cpu::tsx()
{
  x(sp());
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief LoaD Accumulator
 */
void Cpu::lda(uint8_t v,uint8_t cycles)
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
void Cpu::ldx(uint8_t v,uint8_t cycles)
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
void Cpu::ldy(uint8_t v,uint8_t cycles)
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
void Cpu::txa()
{
  a(x());
  SET_ZF(a());
  SET_NF(a());
  tick(2);
}

/**
 * @brief Transfer Accumulator to X
 */
void Cpu::tax()
{
  x(a());
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief Transfer Accumulator to Y
 */
void Cpu::tay()
{
  y(a());
  SET_ZF(y());
  SET_NF(y());
  tick(2);
}

/**
 * @brief Transfer Y to Accumulator
 */
void Cpu::tya()
{
  a(y());
  SET_ZF(a());
  SET_NF(a());
  tick(2);
}

/**
 * @brief PusH Accumulator
 */
void Cpu::pha()
{
  push(a());
  tick(3);
}

/**
 * @brief PuLl Accumulator
 */
void Cpu::pla()
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
void Cpu::ora(uint8_t v,uint8_t cycles)
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
void Cpu::_and(uint8_t v,uint8_t cycles)
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
void Cpu::bit(uint16_t addr,uint8_t cycles)
{
  uint8_t t = load_byte(addr);
  of((t&0x40)!=0);
  SET_NF(t);
  SET_ZF(t&a());
  tick(cycles);
}

/**
 * @brief ROtate Left
 */
uint8_t Cpu::rol(uint8_t v)
{
  uint16_t t = (v << 1) | (uint8_t)cf();
  cf((t&0x100)!=0);
  SET_ZF(t);
  SET_NF(t);
  return (uint8_t)t;
}

/**
 * @brief ROL A register
 */
void Cpu::rol_a()
{
  a(rol(a()));
  tick(2);
}

/**
 * @brief ROL mem
 */
void Cpu::rol_mem(uint16_t addr,uint8_t cycles)
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
uint8_t Cpu::ror(uint8_t v)
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
void Cpu::ror_a()
{
  a(ror(a()));
  tick(2);
}

/**
 * @brief ROR mem
 */
void Cpu::ror_mem(uint16_t addr,uint8_t cycles)
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
uint8_t Cpu::lsr(uint8_t v)
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
void Cpu::lsr_a()
{
  a(lsr(a()));
  tick(2);
}

/**
 * @brief LSR mem
 */
void Cpu::lsr_mem(uint16_t addr,uint8_t cycles)
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
uint8_t Cpu::asl(uint8_t v)
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
void Cpu::asl_a()
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
void Cpu::asl_mem(uint16_t addr,uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  c64_->mem_->write_byte(addr,v);
  c64_->mem_->write_byte(addr,asl(v));
  tick(cycles);
}

/**
 * @brief Exclusive OR
 */
void Cpu::eor(uint8_t v,uint8_t cycles)
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
void Cpu::inc(uint16_t addr,uint8_t cycles)
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
void Cpu::dec(uint16_t addr,uint8_t cycles)
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
void Cpu::inx()
{
  x_+=1;
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief INcrement Y
 */
void Cpu::iny()
{
  y_+=1;
  SET_ZF(y());
  SET_NF(y());
  tick(2);
}

/**
 * @brief DEcrement X
 */
void Cpu::dex()
{
  x_-=1;
  SET_ZF(x());
  SET_NF(x());
  tick(2);
}

/**
 * @brief DEcrement Y
 */
void Cpu::dey()
{
  y_-=1;
  SET_ZF(y());
  SET_NF(y());
  tick(2);
}

/**
 * @brief ADd with Carry
 */
void Cpu::adc(uint8_t v,uint8_t cycles)
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
void Cpu::sbc(uint8_t v,uint8_t cycles)
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
void Cpu::sei()
{
  idf(true);
  tick(2);
}

/**
 * @brief CLear Interrupt flag
 */
void Cpu::cli()
{
  idf(false);
  tick(2);
}

/**
 * @brief SEt Carry flag
 */
void Cpu::sec()
{
  cf(true);
  tick(2);
}

/**
 * @brief CLear Carry flag
 */
void Cpu::clc()
{
  cf(false);
  tick(2);
}

/**
 * @brief SEt Decimal flag
 */
void Cpu::sed()
{
  dmf(true);
  tick(2);
}

/**
 * @brief CLear Decimal flag
 */
void Cpu::cld()
{
  dmf(false);
  tick(2);
}

/**
 * @brief CLear oVerflow flag
 */
void Cpu::clv()
{
  of(false);
  tick(2);
}

uint8_t Cpu::flags()
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

void Cpu::flags(uint8_t v)
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
void Cpu::php()
{
  push(flags());
  tick(3);
}

/**
 * @brief PuLl Processor flags
 */
void Cpu::plp()
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
void Cpu::jsr()
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
void Cpu::jmp()
{
  uint16_t addr = addr_abs();
  pc(addr);
  tick(3);
}

/**
 * @brief JuMP (indirect)
 */
void Cpu::jmp_ind()
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
void Cpu::rts()
{
  uint16_t addr = (pop() + (pop() << 8)) + 1;
  pc(addr);
  tick(6);
}

/**
 * @brief CoMPare
 */
void Cpu::cmp(uint8_t v,uint8_t cycles)
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
void Cpu::cpx(uint8_t v,uint8_t cycles)
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
void Cpu::cpy(uint8_t v,uint8_t cycles)
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
void Cpu::bne()
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
void Cpu::beq()
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
void Cpu::bcs()
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
void Cpu::bcc()
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
void Cpu::bpl()
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
void Cpu::bmi()
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
void Cpu::bvc()
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
void Cpu::bvs()
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
void Cpu::nop(uint8_t cycles)
{
  if(pb_crossed)cycles+=1;
  tick(cycles);
}

/**
 * @brief BReaKpoint
 */
void Cpu::brk()
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
void Cpu::rti()
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
void Cpu::jam(uint8_t insn)
{
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
void Cpu::slo(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  asl_mem(addr,cycles_a);
  ora(load_byte(addr),cycles_b);
}

void Cpu::lxa(uint8_t v,uint8_t cycles)
{
  uint8_t t = ((a() | 0xEE) & v);
  x(t);
  a(t);
  SET_ZF(t);
  SET_NF(t);
  tick(cycles);
}

void Cpu::las(uint8_t v)
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

void Cpu::sax(uint16_t addr,uint8_t cycles)
{
  uint8_t _a = a();
  uint8_t _x = x();
  uint8_t _r = (_a & _x);
  c64_->mem_->write_byte(addr,_r);
  tick(cycles);
}

void Cpu::shy(uint16_t addr,uint8_t cycles)
{
  uint8_t t = ((addr >> 8) + 1);
  uint8_t y_ = y();
  c64_->mem_->write_byte(addr,(y_ & t));
  tick(cycles);
}

void Cpu::shx(uint16_t addr,uint8_t cycles)
{
  uint8_t t = ((addr >> 8) + 1);
  uint8_t x_ = x();
  c64_->mem_->write_byte(addr,(x_ & t));
  tick(cycles);
}

void Cpu::sha(uint16_t addr,uint8_t cycles)
{
  uint8_t t = ((addr >> 8) + 1);
  uint8_t a_ = a();
  uint8_t x_ = x();
  c64_->mem_->write_byte(addr,((a_ & x_) & t));
  tick(cycles);
}

void Cpu::sre(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  uint16_t t = addr;
  lsr_mem(t,cycles_a);
  eor(load_byte(t),cycles_b);
}

void Cpu::rla(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  uint16_t t = addr;
  rol_mem(t,cycles_a);
  _and(load_byte(t),cycles_b);
}

void Cpu::rra(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  uint16_t t = addr;
  ror_mem(t,cycles_a);
  adc(load_byte(t),cycles_b);
}

void Cpu::dcp(uint16_t addr,uint8_t cycles_a,uint8_t cycles_b)
{
  uint16_t t = addr;
  dec(t,cycles_a);
  cmp(load_byte(t),cycles_b);
}

void Cpu::tas(uint16_t addr,uint8_t cycles)
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

void Cpu::sbx(uint8_t v,uint8_t cycles)
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

void Cpu::isc(uint16_t addr,uint8_t cycles)
{
  inc(addr,cycles-=2);
  uint8_t v = load_byte(addr);
  sbc(v,cycles);
}

void Cpu::arr()
{ /* Fixed code with courtesy of Vice 6510core.c */
  unsigned int tmp = (a() & (fetch_op()));
  if(dmf()) {
    int tmp2 = tmp;
    tmp2 |= ((flags() & SR_CARRY) << 8);
    tmp2 >>= 1;
    nf(((flags() & SR_CARRY)?0x80:0)); /* set signed on negative flag */
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

void Cpu::xaa(uint8_t v)
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

void Cpu::dump_flags()
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

void Cpu::dump_flags(uint8_t flags)
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

void Cpu::dump_regs()
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
    pc(),load_byte(pc()),a(),x(),y(),sp(),pflags.str().c_str(),sflags.str().c_str());
}

void Cpu::dump_regs_insn(uint8_t insn)
{
  static uint prev_cycles = cycles();
  D("INSN=%02X '%-9s' ADDR: $%04X CYC=%u ",
    insn,opcodenames[insn],d_address,(cycles()-prev_cycles));
  dump_regs();
  prev_cycles = cycles();
}

void Cpu::dump_regs_json()
{
  D("{");
  D("\"pc\":%d,",pc());
  D("\"a\":%d,",a());
  D("\"x\":%d,",x());
  D("\"y\":%d,",y());
  D("\"sp\":%d",sp());
  D("}\n");
}

void Cpu::dbg()
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

void Cpu::dbg_b()
{
  dbg();
  dump_regs();
}
