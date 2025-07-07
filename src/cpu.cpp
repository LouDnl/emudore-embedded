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

#include "cpu.h"
#include "util.h"
#include <sstream>

/**
 * @brief Cold reset
 *
 * https://www.c64-wiki.com/index.php/Reset_(Process)
 */
void Cpu::reset()
{
  a_ = x_ = y_ = sp_ = 0;
  cf_ = zf_ = idf_ = dmf_ = bcf_ = of_ = nf_ = false;
  pc(mem_->read_word(Memory::kAddrResetVector));
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
  // uint16_t addr_ill = 0;
  /* fetch instruction */
  uint8_t insn = fetch_op();
  bool retval = true;
  /* emulate instruction */
  switch(insn)
  {
  case 0x0: /* BRK */
    brk(); break;
  case 0x1: /* ORA (nn,X) */
    ora(load_byte(addr_indx()),6); break;
  case 0x2: /* JAM ~ Illegal */
    jam(); break;
  case 0x3: /* SLO (nn,X) ~ Illegal */
    slo(addr_indx(), 5, 3);
    break;
  case 0x4: /* NOP ~ Illegal */
    nop(3); break;
  case 0x5: /* ORA nn */
    ora(load_byte(addr_zero()),3); break;
  case 0x6: /* ASL nn */
    asl_mem(addr_zero(),5); break;
  case 0x7: /* SLO nn ~ Illegal */
    slo(addr_zero(), 3, 2);
    break;
  case 0x8: /* PHP */
    php(); break;
  case 0x9: /* ORA #nn */
    ora(fetch_op(),2); break;
  case 0xA: /* ASL A */
    asl_a(); break;
  case 0xB: /* ANC #nn ~ Illegal */
    _and(fetch_op(),0);
    asl_a(); /* 2 cycles */
    break;
  case 0xC: /* NOP nnnn  ~ Illegal */
   nop(4); break;
  case 0xD: /* ORA nnnn */
    ora(load_byte(addr_abs()),4); break;
  case 0xE: /* ASL nnnn */
    asl_mem(addr_abs(),6); break;
  case 0xF:  /* SLO nnnn ~ Illegal */
    slo(addr_abs(), 3, 3);
    break;
  case 0x10: /* BPL nn */
    bpl(); break;
  case 0x11: /* ORA (nn,Y) */
    ora(load_byte(addr_indy()),5); break;
  case 0x12: /* JAM ~ Illegal */
    jam(); break;
  case 0x13: /* SLO (nn,Y) ~ Illegal */
    slo(addr_indy(), 5, 3);
    break;
  case 0x14: /* NOP nn ~ Illegal */
    nop(4); break;
  case 0x15: /* ORA nn,X */
    ora(load_byte(addr_zerox()),4); break;
  case 0x16: /* ASL nn,X */
    asl_mem(addr_zerox(),6); break;
  case 0x17: /* SLO nn ~ Illegal */
    break;
  case 0x18: /* CLC */
    clc(); break;
  case 0x19: /* ORA nnnn,Y */
    ora(load_byte(addr_absy()),4); break;
  case 0x1A: /* NOP ~ Illegal */
    nop(2); break;
  case 0x1B: /* SLO nnnn,Y ~ Illegal */
    slo(addr_absy(), 5, 2);
    break;
  case 0x1C: /* NOP nnnn,X ~ Illegal */
    nop(4); break;
  case 0x1D: /* ORA nnnn,X */
    ora(load_byte(addr_absx()),4); break;
  case 0x1E: /* ASL nnnn,X */
    asl_mem(addr_absx(),7); break;
  case 0x1F: /* SLO nnnn,X ~ Illegal */
    slo(addr_absx(), 5, 2);
    break;
  case 0x20: /* JSR */
    jsr(); break;
  case 0x21: /* AND (nn,X) */
    _and(load_byte(addr_indx()),6); break;
  case 0x22: /* JAM ~ Illegal */
    jam(); break;
  case 0x23: /* RLA (nn,X) ~ Illegal */
    rol_mem(addr_indx(),5);
    _and(load_byte(addr_indx()),3);
    break;
  case 0x24: /* BIT nn */
    bit(addr_zero(),3); break;
  case 0x25: /* AND nn */
    _and(load_byte(addr_zero()),3); break;
  case 0x26: /* ROL nn */
    rol_mem(addr_zero(),5); break;
  case 0x27: /* RLA zp ~ Illegal */
    rol_mem(addr_zero(), 2);
    _and(load_byte(addr_zero()),3);
    break;
  case 0x28: /* PLP */
    plp(); break;
  case 0x29: /* AND #nn */
    _and(fetch_op(),2); break;
  case 0x2A: /* ROL A */
    rol_a(); break;
  case 0x2B: /* ANC #nn ~ Illegal */
    _and(fetch_op(),0);
    rol_a(); /* 2 cycles */
    break;
  case 0x2C: /* BIT nnnn */
    bit(addr_abs(),4); break;
  case 0x2D: /* AND nnnn */
    _and(load_byte(addr_abs()),4); break;
  case 0x2E: /* ROL nnnn */
    rol_mem(addr_abs(),6); break;
  case 0x2F: /* RLA nnnn ~ Illegal */
    rol_mem(addr_abs(), 4);
    _and(load_byte(addr_abs()),2);
    break;
  case 0x30: /* BMI nn */
    bmi(); break;
  case 0x31: /* AND (nn,Y) */
    _and(load_byte(addr_indy()),5); break;
  case 0x32: /* JAM ~ Illegal */
    jam(); break;
  case 0x33: /* RLA (nn,Y) ~ Illegal */
    rol_mem(addr_indy(),5);
    _and(load_byte(addr_indy()),3);
    break;
  case 0x34: /* NOP ~ Illegal */
    nop(4); break;
  case 0x35: /* AND nn,X */
    _and(load_byte(addr_zerox()),4); break;
  case 0x36: /* ROL nn,X */
    rol_mem(addr_zerox(),6); break;
  case 0x37: /* RLA nn,X ~ Illegal */
    rol_mem(addr_zerox(),3);
    _and(load_byte(addr_zerox()),3);
    break;
  case 0x38: /* SEC */
    sec(); break;
  case 0x39: /* AND nnnn,Y */
    _and(load_byte(addr_absy()),4); break;
  case 0x3A: /* NOP ~ Illegal */
    nop(2); break;
  case 0x3B: /* RLA nnnn,Y ~ Illegal */
    rol_mem(addr_absy(),3);
    _and(load_byte(addr_absy()),3);
    break;
  case 0x3C: /* NOP ~ Illegal */
    nop(4); break;
  case 0x3D: /* AND nnnn,X */
    _and(load_byte(addr_absx()),4); break;
  case 0x3E: /* ROL nnnn,X */
    rol_mem(addr_absx(),7); break;
  case 0x3F: /* RLA nnnn ~ Illegal */
    rol_mem(addr_abs(),5);
    _and(load_byte(addr_abs()),3);
    break;
  case 0x40: /* RTI */
    rti(); break;
  case 0x41: /* EOR (nn,X) */
    eor(load_byte(addr_indx()),6); break;
  case 0x42: /* JAM ~ Illegal */
    jam(); break;
  case 0x43: /* SRE (nn,X) ~ Illegal */
    lsr_mem(addr_indx(), 6);
    eor(load_byte(addr_indx()),2);
    break;
  case 0x44: /* NOP ~ Illegal */
    nop(3); break;
  case 0x45: /* EOR nn */
    eor(load_byte(addr_zero()),3); break;
  case 0x46: /* LSR nn */
    lsr_mem(addr_zero(),5); break;
  case 0x47: /* SRE nn ~ Illegal */
    lsr_mem(addr_zero(), 3);
    eor(load_byte(addr_zero()),2);
    break;
  case 0x48: /* PHA */
    pha(); break;
  case 0x49: /* EOR #nn */
    eor(fetch_op(),2); break;
  case 0x4A: /* LSR A */
    lsr_a(); break;
  case 0x4B: /* ALR #nn ~ Illegal */
    _and(fetch_op(), 0);
    lsr_a();
    break;
  case 0x4C: /* JMP nnnn */
    jmp(); break;
  case 0x4D: /* EOR nnnn */
    eor(load_byte(addr_abs()),4); break;
  case 0x4E: /* LSR nnnn */
    lsr_mem(addr_abs(),6); break;
  case 0x4F: /* SRE nnnn ~ Illegal */
    lsr_mem(addr_abs(), 3);
    eor(load_byte(addr_abs()),2);
    break;
  case 0x50: /* BVC */
    bvc(); break;
  case 0x51: /* EOR (nn,Y) */
    eor(load_byte(addr_indy()),5); break;
  case 0x52: /* JAM ~ Illegal */
    jam(); break;
  case 0x53: /* SRE (nn,Y) ~ Illegal */
    lsr_mem(addr_indy(), 6);
    eor(load_byte(addr_indy()),2);
    break;
  case 0x54: /* NOP ~ Illegal */
    nop(4); break;
  case 0x55: /* EOR nn,X */
    eor(load_byte(addr_zerox()),4); break;
  case 0x56: /* LSR nn,X */
    lsr_mem(addr_zerox(),6); break;
  case 0x57: /* SRE nn,X ~ Illegal */
    lsr_mem(addr_zerox(), 3);
    eor(load_byte(addr_zerox()),2);
    break;
  case 0x58: /* CLI */
    cli(); break;
  case 0x59: /* EOR nnnn,Y */
    eor(load_byte(addr_absy()),4); break;
  case 0x5A: /* NOP ~ Illegal */
    nop(2); break;
  case 0x5B: /* SRE nnnn,Y ~ Illegal */
    lsr_mem(addr_absy(), 3);
    eor(load_byte(addr_absy()),2);
    break;
  case 0x5C: /* NOP ~ Illegal */
    nop(4); break;
  case 0x5D: /* EOR nnnn,X */
    eor(load_byte(addr_absx()),4); break;
  case 0x5E: /* LSR nnnn,X */
    lsr_mem(addr_absx(),7); break;
  case 0x5F: /* SRE nnnn,X ~ Illegal */
    lsr_mem(addr_absx(), 3);
    eor(load_byte(addr_absx()),2);
    break;
  case 0x60: /* RTS */
    rts(); break;
  case 0x61: /* ADC (nn,X) */
    adc(load_byte(addr_indx()),6); break;
  case 0x62: /* JAM ~ Illegal */
    jam(); break; // NOTICE: BREAKS FANTA IN SPACE
  case 0x63: /* RRA (nn,X) ~ Illegal */
    ror_mem(addr_indx(),4); break;
    adc(load_byte(addr_indx()),4); break;
    break;
  case 0x64: /* NOP ~ Illegal */
    nop(3); break;
  case 0x65: /* ADC nn */
    adc(load_byte(addr_zero()),3); break;
  case 0x66: /* ROR nn */
    ror_mem(addr_zero(),5); break;
  case 0x67: /* RRA nn ~ Illegal */
    ror_mem(addr_zero(),4); break;
    adc(load_byte(addr_zero()),4); break;
    break;
  case 0x68: /* PLA */
    pla(); break;
  case 0x69: /* ADC #nn */
    adc(fetch_op(),2); break;
  case 0x6A: /* ROR A */
    ror_a(); break;
  case 0x6B: /* ARR #nn ~ Illegal */
    _and(fetch_op(), 0);
    ror_a();
    break;
  case 0x6C: /* JMP (nnnn) */
    jmp_ind(); break;
  case 0x6D: /* ADC nnnn */
    adc(load_byte(addr_abs()),4); break;
  case 0x6E: /* ROR nnnn */
    ror_mem(addr_abs(),6); break;
  case 0x6F: /* RRA nnnn ~ Illegal */
    ror_mem(addr_abs(),3);
    adc(load_byte(addr_abs()),3);
    break;
  case 0x70: /* BVS */
    bvs(); break;
  case 0x71: /* ADC (nn,Y) */
    adc(load_byte(addr_indy()),5); break;
  case 0x72: /* JAM ~ Illegal */
    jam(); break;
  case 0x73: /* RRA (nn,Y) ~ Illegal */
    ror_mem(addr_indy(),4);
    adc(load_byte(addr_indy()),4);
    break;
  case 0x74: /* NOP ~ Illegal */
    nop(4); break;
  case 0x75: /* ADC nn,X */
    adc(load_byte(addr_zerox()),4); break;
  case 0x76: /* ROR nn,X */
    ror_mem(addr_zerox(),6); break;
  case 0x77: /* RRA nn,X ~ Illegal */
    ror_mem(addr_zerox(),4);
    adc(load_byte(addr_zerox()),4);
    break;
  case 0x78: /* SEI */
    sei(); break;
  case 0x79: /* ADC nnnn,Y */
    adc(load_byte(addr_absy()),4); break;
  case 0x7A: /* NOP ~ Illegal */
    nop(2); break;
  case 0x7B: /* RRA nnnn,Y ~ Illegal */
    ror_mem(addr_absy(),4);
    adc(load_byte(addr_absy()),3);
    break;
  case 0x7C: /* NOP ~ Illegal */
    nop(4); break;
  case 0x7D: /* ADC nnnn,X */
    adc(load_byte(addr_absx()),4); break;
  case 0x7E: /* ROR nnnn,X */
    ror_mem(addr_absx(),7); break;
  case 0x7F: /* RRA nnnn,X ~ Illegal */
    ror_mem(addr_absx(),4);
    adc(load_byte(addr_absx()),3);
    break;
  case 0x80: /* NOP ~ Illegal */
    nop(2); break;
  case 0x81: /* STA (nn,X) */
    sta(addr_indx(),6); break;
  case 0x82: /* NOP ~ Illegal */
    nop(2); break;
  case 0x83: /* RRA (nn,X) ~ Illegal */
    ror_mem(addr_indx(),4);
    adc(load_byte(addr_indx()),3);
    break;
  case 0x84: /* STY nn */
    sty(addr_zero(),3); break;
  case 0x85: /* STA nn */
    sta(addr_zero(),3); break;
  case 0x86: /* STX nn */
    stx(addr_zero(),3); break;
  case 0x87: /* SAX nn ~ Illegal */
    sax(addr_zero(), 1, 2);
    break;
  case 0x88: /* DEY */
    dey(); break;
  case 0x89: /* NOP ~ Illegal */
    nop(2); break;
  case 0x8A: /* TXA */
    txa(); break;
  case 0x8B: /* XAA #nn ~ Illegal */
    D("XAA (%X) called at %04x\n", insn,pc()); /* NOTICE: DO NOT USE! */
    break;
  case 0x8C: /* STY nnnn */
    sty(addr_abs(),4); break;
  case 0x8D: /* STA nnnn */
    sta(addr_abs(),4); break;
  case 0x8E: /* STX nnnn */
    stx(addr_abs(),4); break;
  case 0x8F: /* SAX nnnn ~ Illegal */
    sax(addr_abs(), 2, 2);
    break;
  case 0x90: /* BCC nn */
    bcc(); break;
  case 0x91: /* STA (nn,Y) */
    sta(addr_indy(),6); break;
  case 0x92: /* JAM ~ Illegal */
    jam(); break;
  case 0x93: /* SHA (nn,Y) ~ Illegal */ /* ISSUE: WORKINGS UNKNOWN! */
    sha(addr_indy(), 3, 3);
    break;
  case 0x94: /* STY nn,X */
    sty(addr_zerox(),4); break;
  case 0x95: /* STA nn,X */
    sta(addr_zerox(),4); break;
  case 0x96: /* STX nn,Y */
    stx(addr_zeroy(),4); break;
  case 0x97: /* SAX nn,Y ~ Illegal */
    sax(addr_zeroy(), 2, 2);
    break;
  case 0x98: /* TYA */
    tya(); break;
  case 0x99: /* STA nnnn,Y */
    sta(addr_absy(),5); break;
  case 0x9A: /* TXS */
    txs(); break;
  case 0x9B: /* TAS nnnn,Y ~ Illegal */
    break;
  case 0x9C: /* SHY nnnn,X ~ Illegal */ // TODO: Add
    break;
  case 0x9D: /* STA nnnn,X */
    sta(addr_absx(),5); break;
  case 0x9E: /* SHX nnnn,Y ~ Illegal */ // TODO: Add
    break;
  case 0x9F: /* SHA nnnn,Y ~ Illegal */
    sha(addr_absy(), 2, 3);
    break;
  case 0xA0: /* LDY #nn */
    ldy(fetch_op(),2); break;
  case 0xA1: /* LDA (nn,X) */
    lda(load_byte(addr_indx()),6); break;
  case 0xA2: /* LDX #nn */
    ldx(fetch_op(),2); break;
  case 0xA3: /* LAX (nn,X) ~ Illegal */
    lda(load_byte(addr_indx()),4);
    tax();
    break;
  case 0xA4: /* LDY nn */
    ldy(load_byte(addr_zero()),3); break;
  case 0xA5: /* LDA nn */
    lda(load_byte(addr_zero()),3); break;
  case 0xA6: /* LDX nn */
    ldx(load_byte(addr_zero()),3); break;
  case 0xA7: /* LAX nn ~ Illegal */
    lda(load_byte(addr_zero()),1);
    tax();
    break;
  case 0xA8: /* TAY */
    tay(); break;
  case 0xA9: /* LDA #nn */
    lda(fetch_op(),2); break;
  case 0xAA: /* TAX */
    tax(); break;
  case 0xAB: /* LXA #nn ~ Illegal */
    lda(fetch_op(),2);
    tax();
    break;
  case 0xAC: /* LDY nnnn */
    ldy(load_byte(addr_abs()),4); break;
  case 0xAD: /* LDA nnnn */
    lda(load_byte(addr_abs()),4); break;
  case 0xAE: /* LDX nnnn */
    ldx(load_byte(addr_abs()),4); break;
  case 0xAF: /* LAX nnnn ~ Illegal */
    lda(load_byte(addr_abs()),2);
    tax();
    break;
  case 0xB0: /* BCS nn */
    bcs(); break;
  case 0xB1: /* LDA (nn,Y) */
    lda(load_byte(addr_indy()),5); break;
  case 0xB2: /* JAM ~ Illegal */
    jam(); break;
  case 0xB3: /* LAX (nn,Y) ~ Illegal */
    lda(load_byte(addr_indy()),4);
    tax();
    break;
  case 0xB4: /* LDY nn,X */
    ldy(load_byte(addr_zerox()),3); break;
  case 0xB5: /* LDA nn,X */
    lda(load_byte(addr_zerox()),3); break;
  case 0xB6: /* LDX nn,Y */
    ldx(load_byte(addr_zeroy()),3); break;
  case 0xB7: /* LAX nn,Y ~ Illegal */
    lda(load_byte(addr_zeroy()),2);
    tax();
    break;
  case 0xB8: /* CLV */
    clv(); break;
  case 0xB9: /* LDA nnnn,Y */
    lda(load_byte(addr_absy()),4); break;
  case 0xBA: /* TSX */
    tsx(); break;
  case 0xBB: /* LAS nnnn,Y ~ Illegal */
    tsx(); txa(); /* 2 + 2 cycles */
    _and(load_byte(addr_absy()),0);
    tax(); txs(); /* 2 + 2 cycles */
    backtick(3); /* 5 cycles total so reverse 3 */
    break;
  case 0xBC: /* LDY nnnn,X */
    ldy(load_byte(addr_absx()),4); break;
  case 0xBD: /* LDA nnnn,X */
    lda(load_byte(addr_absx()),4); break;
  case 0xBE: /* LDX nnnn,Y */
    ldx(load_byte(addr_absy()),4); break;
  case 0xBF: /* LAX nnnn,Y ~ Illegal */
    lda(load_byte(addr_absy()),3);
    tax(); /* 2 cycles */
    break;
  case 0xC0: /* CPY #nn */
    cpy(fetch_op(),2); break;
  case 0xC1: /* CMP (nn,X) */
    cmp(load_byte(addr_indx()),6); break;
  case 0xC2: /* NOP ~ Illegal */
    nop(2); break;
  case 0xC3: /* DCP (nn,X) ~ Illegal */
    dec(addr_indx(),4);
    cmp(load_byte(addr_indx()),4);
    break;
  case 0xC4: /* CPY nn */
    cpy(load_byte(addr_zero()),3); break;
  case 0xC5: /* CMP nn */
    cmp(load_byte(addr_zero()),3); break;
  case 0xC6: /* DEC nn */
    dec(addr_zero(),5); break;
  case 0xC7: /* DCP nn ~ Illegal */
    dec(addr_zero(),4);
    cmp(load_byte(addr_zero()),4);
    break;
  case 0xC8: /* INY */
    iny(); break;
  case 0xC9: /* CMP #nn */
    cmp(fetch_op(),2); break;
  case 0xCA: /* DEX */
    dex(); break;
  case 0xCB: /* SBX #nn ~ Illegal */ // TODO: Add
    break;
  case 0xCC: /* CPY nnnn */
    cpy(load_byte(addr_abs()),4); break;
  case 0xCD: /* CMP nnnn */
    cmp(load_byte(addr_abs()),4); break;
  case 0xCE: /* DEC nnnn */
    dec(addr_abs(),6); break;
  case 0xCF: /* DCP nnnn ~ Illegal */
    dec(addr_abs(),3);
    cmp(load_byte(addr_abs()),3);
    break;
  case 0xD0: /* BNE nn */
    bne(); break;
  case 0xD1: /* CMP (nn,Y) */
    cmp(load_byte(addr_indy()),5); break;
  case 0xD2: /* JAM ~ Illegal */
    jam(); break;
  case 0xD3: /* DCP (nn,Y) ~ Illegal */
    dec(addr_indy(),4);
    cmp(load_byte(addr_indy()),4);
    break;
  case 0xD4: /* NOP ~ Illegal */
    nop(4); break;
  case 0xD5: /* CMP nn,X */
    cmp(load_byte(addr_zerox()),4); break;
  case 0xD6: /* DEC nn,X */
    dec(addr_zerox(),6); break;
  case 0xD7: /* DCP ~ Illegal */
    dec(addr_zerox(),4); cmp(load_byte(addr_absx()),2); break;
  case 0xD8: /* CLD */
    cld(); break;
  /* CMP nnnn,Y */
  case 0xD9: cmp(load_byte(addr_absy()),4); break;
  /* NOP ~ Illegal */
  case 0xDA: nop(2); break;
  /* DCP nnnn,Y ~ Illegal */
  case 0xDB:
    dec(addr_absy(),4);
    cmp(load_byte(addr_absy()),3);
    break;
  case 0xDC: /* NOP ~ Illegal */
    nop(4); break;
  case 0xDD: /* CMP nnnn,X */
    cmp(load_byte(addr_absx()),4); break;
  case 0xDE: /* DEC nnnn,X */
    dec(addr_absx(),7); break;
  case 0xDF: /* DCP nnnn,X ~ Illegal */
    dec(addr_absx(),4);
    cmp(load_byte(addr_absx()),3);
    break;
  case 0xE0: /* CPX #nn */
    cpx(fetch_op(),2); break;
  case 0xE1: /* SBC (nn,X) */
    sbc(load_byte(addr_indx()),6); break;
  case 0xE2: /* NOP ~ Illegal */
    nop(2); break;
  case 0xE3: /* ISC (nn,X) ~ Illegal */
    break;
  case 0xE4: /* CPX nn */
    cpx(load_byte(addr_zero()),3); break;
  case 0xE5: /* SBC nn */
    sbc(load_byte(addr_zero()),3); break;
  case 0xE6: /* INC nn */
    inc(addr_zero(),5); break;
  case 0xE7: /* ISC nn ~ Illegal */
    inc(addr_zero(),3);
    sbc(addr_zero(),2);
    break;
  case 0xE8: /* INX */
    inx(); break;
  case 0xE9: /* SBC #nn */
    sbc(fetch_op(),2); break;
  case 0xEA: /* NOP */
    nop(2); break;
  case 0xEB: /* USBC #nn ~ Illegal */
    sbc(fetch_op(),2); break;
    break;
  case 0xEC: /* CPX nnnn */
    cpx(load_byte(addr_abs()),4); break;
  case 0xED: /* SBC nnnn */
    sbc(load_byte(addr_abs()),4); break;
  case 0xEE: /* INC nnnn */
    inc(addr_abs(),6); break;
  case 0xEF: /* ISC nnnn ~ Illegal */
    inc(addr_abs(),3);
    sbc(addr_abs(),3);
    break;
  case 0xF0: /* BEQ nn */
    beq(); break;
  case 0xF1: /* SBC (nn,Y) */
    sbc(load_byte(addr_indy()),5); break;
  case 0xF2: /* JAM ~ Illegal */
    jam(); break;
  case 0xF3: /* ISC (nn,Y) ~ Illegal */
    inc(addr_indy(),3);
    sbc(addr_indy(),3);
    break;
  case 0xF4: /* NOP ~ Illegal */
    nop(4); break;
  case 0xF5: /* SBC nn,X */
    sbc(load_byte(addr_zerox()),4); break;
  case 0xF6: /* INC nn,X */
    inc(addr_zerox(),6); break;
  case 0xF7: /* ISC nn,X ~ Illegal */
    inc(addr_zerox(),3);
    sbc(addr_zerox(),3);
    break;
  case 0xF8: /* SED */
    sed(); break;
  case 0xF9: /* SBC nnnn,Y */
    sbc(load_byte(addr_absy()),4); break;
  case 0xFA: /* NOP ~ Illegal */
    nop(2); break;
  case 0xFB: /* ISC nnnn,y ~ Illegal */
    inc(addr_absy(),3);
    sbc(addr_absy(),4);
    break;
  case 0xFC: /* NOP ~ Illegal */
    nop(4); break;
  case 0xFD: /* SBC nnnn,X */
    sbc(load_byte(addr_absx()),4); break;
  case 0xFE: /* INC nnnn,X */
    inc(addr_absx(),7); break;
  case 0xFF: /* ISC nnnn,X ~ Illegal */
    inc(addr_absx(),3);
    sbc(addr_absx(),4);
    break;
  /* Anything else */
  default:
    D("Unknown instruction: %X at %04x\n", insn,pc());
    // retval = false;
  }
  return retval;
}

// helpers ///////////////////////////////////////////////////////////////////

uint8_t Cpu::load_byte(uint16_t addr)
{
  return mem_->read_byte(addr);
}

void Cpu::push(uint8_t v)
{
  uint16_t addr = Memory::kBaseAddrStack+sp_;
  mem_->write_byte(addr,v);
  sp_--;
}

uint8_t Cpu::pop()
{
  uint16_t addr = ++sp_+Memory::kBaseAddrStack;
  return load_byte(addr);
}

uint8_t Cpu::fetch_op()
{
  return load_byte(pc_++);
}

uint16_t Cpu::fetch_opw()
{
  uint16_t retval = mem_->read_word(pc_);
  pc_+=2;
  return retval;
}

uint16_t Cpu::addr_zero()
{
  uint16_t addr = fetch_op();
  return addr;
}

uint16_t Cpu::addr_zerox()
{
  /* wraps around the zeropage */
  uint16_t addr = (fetch_op() + x()) & 0xff ;
  return addr;
}

uint16_t Cpu::addr_zeroy()
{
  /* wraps around the zeropage */
  uint16_t addr = (fetch_op() + y()) & 0xff;
  return addr;
}

uint16_t Cpu::addr_abs()
{
  uint16_t addr = fetch_opw();
  return addr;
}

uint16_t Cpu::addr_absy()
{
  uint16_t addr = fetch_opw() + y();
  return addr;
}

uint16_t Cpu::addr_absx()
{
  uint16_t addr = fetch_opw() + x();
  return addr;
}

uint16_t Cpu::addr_indx()
{
  /* wraps around the zeropage */
  uint16_t addr = mem_->read_word((addr_zero() + x()) & 0xff);
  return addr;
}

uint16_t Cpu::addr_indy()
{
  uint16_t addr = mem_->read_word(addr_zero()) + y();
  return addr;
}

// Instructions: data handling and memory operations  ////////////////////////

/**
 * @brief STore Accumulator
 */
void Cpu::sta(uint16_t addr, uint8_t cycles)
{
  mem_->write_byte(addr,a());
  tick(cycles);
}

/**
 * @brief STore X
 */
void Cpu::stx(uint16_t addr, uint8_t cycles)
{
  mem_->write_byte(addr,x());
  tick(cycles);
}

/**
 * @brief STore Y
 */
void Cpu::sty(uint16_t addr, uint8_t cycles)
{
  mem_->write_byte(addr,y());
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
void Cpu::lda(uint8_t v, uint8_t cycles)
{
  a(v);
  SET_ZF(a());
  SET_NF(a());
  tick(cycles);
}

/**
 * @brief LoaD X
 */
void Cpu::ldx(uint8_t v, uint8_t cycles)
{
  x(v);
  SET_ZF(x());
  SET_NF(x());
  tick(cycles);
}

/**
 * @brief LoaD Y
 */
void Cpu::ldy(uint8_t v, uint8_t cycles)
{
  y(v);
  SET_ZF(y());
  SET_NF(y());
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
void Cpu::ora(uint8_t v, uint8_t cycles)
{
  a(a()|v);
  SET_ZF(a());
  SET_NF(a());
  tick(cycles);
}

/**
 * @brief Logical AND
 */
void Cpu::_and(uint8_t v, uint8_t cycles)
{
  a(a()&v);
  SET_ZF(a());
  SET_NF(a());
  tick(cycles);
}

/**
 * @brief BIT test
 */
void Cpu::bit(uint16_t addr, uint8_t cycles)
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
void Cpu::rol_mem(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  mem_->write_byte(addr,v);
  mem_->write_byte(addr,rol(v));
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
void Cpu::ror_mem(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  mem_->write_byte(addr,v);
  mem_->write_byte(addr,ror(v));
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
void Cpu::lsr_mem(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  mem_->write_byte(addr,v);
  mem_->write_byte(addr,lsr(v));
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
 * cycles. First the unmodified data that was read is written back, and then
 * the modified data is written. This characteristic may cause issues by
 * twice accessing hardware that acts on a write. This anomaly continued
 * through the entire NMOS line, but was fixed in the CMOS derivatives, in
 * which the processor will do two reads and one write cycle.
 * --
 *
 * I have come across code that uses this side-effect as a feature, for
 * instance, the following instruction will acknowledge VIC interrupts
 * on the first write cycle:
 *
 * ASL $d019
 *
 * So.. we need to mimic the behaviour.
 */
void Cpu::asl_mem(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  mem_->write_byte(addr,v);
  mem_->write_byte(addr,asl(v));
  tick(cycles);
}

/**
 * @brief Exclusive OR
 */
void Cpu::eor(uint8_t v, uint8_t cycles)
{
  a(a()^v);
  SET_ZF(a());
  SET_NF(a());
  tick(cycles);
}

// Instructions: arithmetic operations  //////////////////////////////////////

/**
 * @brief INCrement
 */
void Cpu::inc(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  mem_->write_byte(addr,v);
  v++;
  mem_->write_byte(addr,v);
  SET_ZF(v);
  SET_NF(v);
}

/**
 * @brief DECrement
 */
void Cpu::dec(uint16_t addr, uint8_t cycles)
{
  uint8_t v = load_byte(addr);
  /* see ASL doc */
  mem_->write_byte(addr,v);
  v--;
  mem_->write_byte(addr,v);
  SET_ZF(v);
  SET_NF(v);
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
void Cpu::adc(uint8_t v, uint8_t cycles)
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
}

/**
 * @brief SuBstract with Carry
 */
void Cpu::sbc(uint8_t v, uint8_t cycles)
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
  /* unused, always set */
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
  uint16_t addr = mem_->read_word(addr_abs());
  pc(addr);
  tick(3);
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
 * @brief Branch if Not Equal
 */
void Cpu::bne()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(!zf()) pc(addr);
  tick(2);
}

/**
 * @brief CoMPare
 */
void Cpu::cmp(uint8_t v, uint8_t cycles)
{
  uint16_t t;
  t = a() - v;
  cf(t<0x100);
  t = t&0xff;
  SET_ZF(t);
  SET_NF(t);
  tick(cycles);
}

/**
 * @brief CoMPare X
 */
void Cpu::cpx(uint8_t v, uint8_t cycles)
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
void Cpu::cpy(uint8_t v, uint8_t cycles)
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
 * @brief Branch if Equal
 */
void Cpu::beq()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(zf()) pc(addr);
  tick(2);
}

/**
 * @brief Branch if Carry is Set
 */
void Cpu::bcs()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(cf()) pc(addr);
  tick(2);
}

/**
 * @brief Branch if Carry is Clear
 */
void Cpu::bcc()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(!cf()) pc(addr);
  tick(2);
}

/**
 * @brief, Branch if PLus
 */
void Cpu::bpl()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(!nf()) pc(addr);
  tick(2);
}

/**
 * @brief Branch if MInus
 */
void Cpu::bmi()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(nf()) pc(addr);
  tick(2);
}

/**
 * @brief Branch if oVerflow Clear
 */
void Cpu::bvc()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(!of()) pc(addr);
  tick(2);
}

/**
 * @brief Branch if oVerflow Set
 */
void Cpu::bvs()
{
  uint16_t addr = (int8_t) fetch_op() + pc();
  if(of()) pc(addr);
  tick(2);
}

// misc //////////////////////////////////////////////////////////////////////

/**
 * @brief No OPeration
 */
void Cpu::nop(uint8_t cycles)
{
  tick(cycles);
}

/**
 * @brief BReaKpoint
 */
void Cpu::brk()
{
  push(((pc()+1) >> 8) & 0xff);
  push(((pc()+1) & 0xff));
  push(flags());
  pc(mem_->read_word(Memory::kAddrIRQVector));
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
  tick(7);
}

// illegals  /////////////////////////////////////////////////////////////////

void Cpu::jam()
{
  printf("JAM! Instr %02X @ %04X\n", load_byte(pc_-1), pc());
  // for (;;) {};
}

void Cpu::slo(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b)
{
  asl_mem(addr, cycles_a);
  ora(load_byte(addr),cycles_b);
}

void Cpu::sax(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b)
{
  php();
  pha();
  stx(addr,cycles_a);
  _and(load_byte(addr),0);
  sta(addr,cycles_b);
  pla();
  plp();
}

void Cpu::sha(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b)
{ /* ISSUE: WORKINGS UNKNOWN! */
  php();
  pha();
  stx(mem_->read_byte(0x02),0);
  _and(load_byte(mem_->read_byte(0x02)),0);
  _and(load_byte(fetch_op()),0);
  sta(addr,cycles_a);
  ldx(addr_zero(),cycles_b);
  pla();
  plp();
}

void Cpu::tas(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b)
{ /* ISSUE: WORKINGS UNKNOWN! */
  php();
  sta(mem_->read_byte(0x03),0);
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
    pc(mem_->read_word(Memory::kAddrIRQVector));
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
  pc(mem_->read_word(Memory::kAddrNMIVector));
  tick(7);
}

// debugging /////////////////////////////////////////////////////////////////

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
  D("pc=%04x, a=%02x x=%02x y=%02x sp=%02x flags= %s\n",
    pc(),a(),x(),y(),sp(),sflags.str().c_str());
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
