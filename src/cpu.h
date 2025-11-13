/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * cpu.h
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

#ifndef EMUDORE_CPU_H
#define EMUDORE_CPU_H


#include <cstdint>
#include <functional>
#include <iostream>
#include <ios>
#include <memory.h>

/* These define the position of the status
   flags in the status (flags) register
  */
#define SR_NEGATIVE      0x80
#define SR_SIGN          0x80
#define SR_OVERFLOW      0x40
#define SR_UNUSED        0x20
#define SR_BREAK         0x10
#define SR_DECIMAL       0x08
#define SR_INTERRUPT     0x04
#define SR_ZERO          0x02
#define SR_CARRY         0x01 /*  */

#define ANE_MAGIC        0xef

/* macro helpers */

#define SET_ZF(val)     (zf(!(uint8_t)(val)))
#define SET_OF(val)     (of(((uint8_t)(val)&0x40)!=0))
#define SET_NF(val)     (nf(((uint8_t)(val)&0x80)!=0))
#define SET_CF(val)     (cf(((uint8_t)(val)&0x100)!=0))

#define SETFLAG(flag, cond) \
    if (cond) _flags |= (uint8_t)flag; \
    else _flags &= ~(uint8_t)flag;

/**
 * @brief MOS 6510 microprocessor
 */
class Cpu
{
  protected:
    const char * opcodenames[0x100] = { /* For debug logging */
      "BRK impl", "ORA X,ind", "JAM", "SLO X,ind", "NOP zpg", "ORA zpg", "ASL zpg", "SLO zpg", "PHP impl", "ORA #", "ASL A", "ANC #", "NOP abs", "ORA abs", "ASL abs", "SLO abs",
      "BPL rel", "ORA ind,Y", "JAM", "SLO ind,Y", "NOP zpg,X", "ORA zpg,X", "ASL zpg,X", "SLO zpg,X", "CLC impl", "ORA abs,Y", "NOP impl", "SLO abs,Y", "NOP abs,X", "ORA abs,X", "ASL abs,X", "SLO abs,X",
      "JSR abs", "AND X,ind", "JAM", "RLA X,ind", "BIT zpg", "AND zpg", "ROL zpg", "RLA zpg", "PLP impl", "AND #", "ROL A", "ANC #", "BIT abs", "AND abs", "ROL abs", "RLA abs",
      "BMI rel", "AND ind,Y", "JAM", "RLA ind,Y", "NOP zpg,X", "AND zpg,X", "ROL zpg,X", "RLA zpg,X", "SEC impl", "AND abs,Y", "NOP impl", "RLA abs,Y", "NOP abs,X", "AND abs,X", "ROL abs,X", "RLA abs,X",
      "RTI impl", "EOR X,ind", "JAM", "SRE X,ind", "NOP zpg", "EOR zpg", "LSR zpg", "SRE zpg", "PHA impl", "EOR #", "LSR A", "ALR #", "JMP abs", "EOR abs", "LSR abs", "SRE abs",
      "BVC rel", "EOR ind,Y", "JAM", "SRE ind,Y", "NOP zpg,X", "EOR zpg,X", "LSR zpg,X", "SRE zpg,X", "CLI impl", "EOR abs,Y", "NOP impl", "SRE abs,Y", "NOP abs,X", "EOR abs,X", "LSR abs,X", "SRE abs,X",
      "RTS impl", "ADC X,ind", "JAM", "RRA X,ind", "NOP zpg", "ADC zpg", "ROR zpg", "RRA zpg", "PLA impl", "ADC #", "ROR A", "ARR #", "JMP ind", "ADC abs", "ROR abs", "RRA abs",
      "BVS rel", "ADC ind,Y", "JAM", "RRA ind,Y", "NOP zpg,X", "ADC zpg,X", "ROR zpg,X", "RRA zpg,X", "SEI impl", "ADC abs,Y", "NOP impl", "RRA abs,Y", "NOP abs,X", "ADC abs,X", "ROR abs,X", "RRA abs,X",
      "NOP #", "STA X,ind", "NOP #", "SAX X,ind", "STY zpg", "STA zpg", "STX zpg", "SAX zpg", "DEY impl", "NOP #", "TXA impl", "ANE #", "STY abs", "STA abs", "STX abs", "SAX abs",
      "BCC rel", "STA ind,Y", "JAM", "SHA ind,Y", "STY zpg,X", "STA zpg,X", "STX zpg,Y", "SAX zpg,Y", "TYA impl", "STA abs,Y", "TXS impl", "TAS abs,Y", "SHY abs,X", "STA abs,X", "SHX abs,Y", "SHA abs,Y",
      "LDY #", "LDA X,ind", "LDX #", "LAX X,ind", "LDY zpg", "LDA zpg", "LDX zpg", "LAX zpg", "TAY impl", "LDA #", "TAX impl", "LXA #", "LDY abs", "LDA abs", "LDX abs", "LAX abs",
      "BCS rel", "LDA ind,Y", "JAM", "LAX ind,Y", "LDY zpg,X", "LDA zpg,X", "LDX zpg,Y", "LAX zpg,Y", "CLV impl", "LDA abs,Y", "TSX impl", "LAS abs,Y", "LDY abs,X", "LDA abs,X", "LDX abs,Y", "LAX abs,Y",
      "CPY #", "CMP X,ind", "NOP #", "DCP X,ind", "CPY zpg", "CMP zpg", "DEC zpg", "DCP zpg", "INY impl", "CMP #", "DEX impl", "SBX #", "CPY abs", "CMP abs", "DEC abs", "DCP abs",
      "BNE rel", "CMP ind,Y", "JAM", "DCP ind,Y", "NOP zpg,X", "CMP zpg,X", "DEC zpg,X", "DCP zpg,X", "CLD impl", "CMP abs,Y", "NOP impl", "DCP abs,Y", "NOP abs,X", "CMP abs,X", "DEC abs,X", "DCP abs,X",
      "CPX #", "SBC X,ind", "NOP #", "ISC X,ind", "CPX zpg", "SBC zpg", "INC zpg", "ISC zpg", "INX impl", "SBC #", "NOP impl", "USBC #", "CPX abs", "SBC abs", "INC abs", "ISC abs",
      "BEQ rel", "SBC ind,Y", "JAM", "ISC ind,Y", "NOP zpg,X", "SBC zpg,X", "INC zpg,X", "ISC zpg,X", "SED impl", "SBC abs,Y", "NOP impl", "ISC abs,Y", "NOP abs,X", "SBC abs,X", "INC abs,X", "ISC abs, X",
    };
    using InstructionHandler = std::function<void()>;
    std::array<InstructionHandler, 256> instruction_table;
  private:

  /* registers */
    uint16_t pc_;
    uint8_t sp_, a_, x_, y_;

    /* flags (p/status reg) */
    /*   nf, of, -, bcf, dmf, idf, zf, cf */
    /* 0b 1   1  1    1    1    1   1   1 */
    uint8_t _flags = 0b11111111;

    /* c64->memory and clock */
    C64 *c64_;
    static unsigned int cycles_;

    /* helpers */
    uint16_t curr_page; /* current page at start of cpu emulation */
    bool pb_crossed;    /* true if page boundary crossed */

    uint8_t load_byte(uint16_t addr);
    uint16_t load_word(uint16_t addr);
    void push(uint8_t);
    uint8_t pop();
    uint8_t fetch_op();
    uint16_t fetch_opw();
    uint16_t addr_zero();
    uint16_t addr_zerox();
    uint16_t addr_zeroy();
    uint16_t addr_abs();
    uint16_t addr_absy();
    uint16_t addr_absx();
    uint16_t addr_indx();
    uint16_t addr_indy();

    uint8_t rol(uint8_t v);
    uint8_t ror(uint8_t v);
    uint8_t lsr(uint8_t v);
    uint8_t asl(uint8_t v);
    void tick(uint8_t v){cycles_+=v;};
    void backtick(uint8_t v){cycles_-=v;};
    uint8_t flags();
    void flags(uint8_t v);
    /* instructions : data handling and memory operations */
    void sta(uint16_t addr, uint8_t cycles);
    void stx(uint16_t addr, uint8_t cycles);
    void sty(uint16_t addr, uint8_t cycles);
    void lda(uint8_t v, uint8_t cycles);
    void ldx(uint8_t v, uint8_t cycles);
    void ldy(uint8_t v, uint8_t cycles);
    void txs();
    void tsx();
    void tax();
    void txa();
    void tay();
    void tya();
    void pha();
    void pla();
    /* instructions: logic operations */
    void ora(uint8_t v, uint8_t cycles);
    void _and(uint8_t v, uint8_t cycles);
    void bit(uint16_t addr, uint8_t cycles);
    void rol_a();
    void rol_mem(uint16_t addr, uint8_t cycles);
    void ror_a();
    void ror_mem(uint16_t addr, uint8_t cycles);
    void asl_a();
    void asl_mem(uint16_t addr, uint8_t cycles);
    void lsr_a();
    void lsr_mem(uint16_t addr, uint8_t cycles);
    void eor(uint8_t v, uint8_t cycles);
    /* instructions: arithmetic operations */
    void inc(uint16_t addr, uint8_t cycles);
    void dec(uint16_t addr, uint8_t cycles);
    void inx();
    void iny();
    void dex();
    void dey();
    void adc(uint8_t v, uint8_t cycles);
    void sbc(uint8_t v, uint8_t cycles);
    /* instructions: flag access */
    void sei();
    void cli();
    void sec();
    void clc();
    void sed();
    void cld();
    void clv();
    void php();
    void plp();
    /* instructions: control flow */
    void cmp(uint8_t v, uint8_t cycles);
    void cpx(uint8_t v, uint8_t cycles);
    void cpy(uint8_t v, uint8_t cycles);
    void rts();
    void jsr();
    void bne();
    void beq();
    void bcs();
    void bcc();
    void bpl();
    void bmi();
    void bvc();
    void bvs();
    void jmp();
    void jmp_ind();
    /* instructions: misc */
    void nop(uint8_t cycles);
    void brk();
    void rti();
    /* Instructions: illegal */
    void jam(uint8_t insn);
    void slo(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    void lxa(uint8_t v, uint8_t cycles);
    void anc(uint8_t v);
    void las(uint8_t v);
    void lax(uint8_t v, uint8_t cycles);
    void shy(uint16_t addr, uint8_t cycles);
    void shx(uint16_t addr, uint8_t cycles);
    void sha(uint16_t addr, uint8_t cycles);
    void sax(uint16_t addr, uint8_t cycles);
    void sre(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    void rla(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    void rla_(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    void rra(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    void dcp(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    void tas(uint16_t addr, uint8_t cycles);
    void sbx(uint8_t v, uint8_t cycles);
    void isc(uint16_t addr, uint8_t cycles);
    void arr();
    void xaa(uint8_t v);
  public:
    Cpu(C64 * c64);

    /* cpu state */
    void reset();
    bool emulate();

    /* register access */
    inline uint16_t pc() { return pc_; };
    inline void pc(uint16_t v) { pc_=v; };
    inline uint8_t sp() { return sp_; };
    inline void sp(uint8_t v) { sp_=v; };
    inline uint8_t a() { return a_; };
    inline void a(uint8_t v) { a_=v; };
    inline uint8_t x() { return x_; };
    inline void x(uint8_t v) { x_=v; };
    inline uint8_t y() { return y_; };
    inline void y(uint8_t v) { y_=v; };

    /* flags */
    /*   nf, of, -, bcf, dmf, idf, zf, cf */
    /* 0b 1   1  1    1    1    1   1   1 */
    inline bool getflag(int flag) { return (_flags & flag); };
    inline bool cf(void)    { return getflag(SR_CARRY); }; /* cf_ */
    inline void cf(bool v)  { SETFLAG(SR_CARRY, v); }; /* cf_=v */
    inline bool zf(void)    { return getflag(SR_ZERO); }; /* zf_ */
    inline void zf(bool v)  { SETFLAG(SR_ZERO, v); }; /* zf_=v */
    inline bool idf(void)   { return getflag(SR_INTERRUPT); }; /* idf_ */
    inline void idf(bool v) { SETFLAG(SR_INTERRUPT, v); }; /* idf_=v */
    inline bool dmf(void)   { return getflag(SR_DECIMAL); }; /* dmf_ */
    inline void dmf(bool v) { SETFLAG(SR_DECIMAL, v); }; /* dmf_=v */
    inline bool bcf(void)   { return getflag(SR_BREAK); }; /* bcf_ */
    inline void bcf(bool v) { SETFLAG(SR_BREAK, v); }; /* bcf_=v */
    inline bool of(void)    { return getflag(SR_OVERFLOW); }; /* of_ */
    inline void of(bool v)  { SETFLAG(SR_OVERFLOW, v); }; /* of_=v */
    inline bool nf(void)    { return getflag(SR_NEGATIVE); }; /* nf_ */
    inline void nf(bool v)  { SETFLAG(SR_NEGATIVE, v); }; /* nf_=v */

    /* clock */
    unsigned int cycles(){return cycles_;};
    void cycles(unsigned int v){cycles_=v;};

    /* interrupts */
    void nmi();
    void irq();

    /* debug */
    static bool loginstructions;
    void dump_flags();
    void dump_flags(uint8_t flags);
    void dump_regs();
    void dump_regs_insn(uint8_t insn);
    void dump_regs_json();
    void dbg();
    void dbg_a();
    void dbg_b();

  private:
    void initialize_instruction_table() {
      /* 0x00 ~ 0x0F */
      instruction_table[0x00] = [this]() { this->brk(); };                                           /* BRK impl */
      instruction_table[0x01] = [this]() { this->ora(this->load_byte(this->addr_indx()), 6); };      /* ORA (ind,X) */
      instruction_table[0x02] = [this]() { this->jam(0x02); };                                       /* JAM ~ Illegal OPCode */
      instruction_table[0x03] = [this]() { this->slo(this->addr_indx(), 5, 3); };                    /* SLO (ind,X) ~ Illegal OPCode */
      instruction_table[0x04] = [this]() { this->load_byte(addr_zero()); this->nop(3); };            /* NOP zpg ~ Illegal OPCode */
      instruction_table[0x05] = [this]() { this->ora(this->load_byte(this->addr_zero()), 3); };      /* ORA zpg */
      instruction_table[0x06] = [this]() { this->asl_mem(this->addr_zero(), 5); };                   /* ASL zpg */
      instruction_table[0x07] = [this]() { this->slo(this->addr_zero(), 3, 2); };                    /* SLO zpg ~ Illegal OPCode */
      instruction_table[0x08] = [this]() { this->php(); };                                           /* PHP impl */
      instruction_table[0x09] = [this]() { this->ora(this->fetch_op(), 2); };                        /* ORA #imm */
      instruction_table[0x0A] = [this]() { this->asl_a(); };                                         /* ASL A */
      instruction_table[0x0B] = [this]() { this->anc(this->fetch_op()); };                           /* ANC(AAC) #imm ~ Illegal OPCode */
      instruction_table[0x0C] = [this]() { this->load_byte(this->addr_abs()); this->nop(4); };       /* NOP abs  ~ Illegal OPCode */
      instruction_table[0x0D] = [this]() { this->ora(this->load_byte(this->addr_abs()), 4); };       /* ORA abs */
      instruction_table[0x0E] = [this]() { this->asl_mem(this->addr_abs(), 6); };                    /* ASL abs */
      instruction_table[0x0F] = [this]() { this->slo(this->addr_abs(), 3, 3); };                     /* SLO abs ~ Illegal OPCode */
      /* 0x10 ~ 0x1F */
      instruction_table[0x10] = [this]() { this->bpl(); };                                           /* BPL rel */
      instruction_table[0x11] = [this]() { this->ora(this->load_byte(this->addr_indy()), 5); };      /* ORA (ind),Y */
      instruction_table[0x12] = [this]() { this->jam(0x12); };                                       /* JAM ~ Illegal OPCode */
      instruction_table[0x13] = [this]() { this->slo(this->addr_indy(), 5, 3); };                    /* SLO (ind),Y ~ Illegal OPCode */
      instruction_table[0x14] = [this]() { this->load_byte(this->addr_zerox()); this->nop(4); };     /* NOP zpg,X ~ Illegal OPCode */
      instruction_table[0x15] = [this]() { this->ora(this->load_byte(this->addr_zerox()), 4); };     /* ORA zpg,X */
      instruction_table[0x16] = [this]() { this->asl_mem(this->addr_zerox(), 6); };                  /* ASL zpg,X */
      instruction_table[0x17] = [this]() { this->slo(this->addr_zerox(), 2, 3); };                   /* SLO zpg,X ~ Illegal OPCode */
      instruction_table[0x18] = [this]() { this->clc(); };                                           /* CLC impl */
      instruction_table[0x19] = [this]() { this->ora(this->load_byte(this->addr_absy()), 4); };      /* ORA abs,Y */
      instruction_table[0x1A] = [this]() { this->nop(2); };                                          /* NOP impl ~ Illegal OPCode */
      instruction_table[0x1B] = [this]() { this->slo(this->addr_absy(), 4, 2); };                    /* SLO abs,Y ~ Illegal OPCode */
      instruction_table[0x1C] = [this]() { this->load_byte(this->addr_absx()); this->nop(4); };      /* NOP abs,X ~ Illegal OPCode */
      instruction_table[0x1D] = [this]() { this->ora(this->load_byte(this->addr_absx()), 4); };      /* ORA abs,X */
      instruction_table[0x1E] = [this]() { this->asl_mem(this->addr_absx(), 7); };                   /* ASL abs,X */
      instruction_table[0x1F] = [this]() { this->slo(this->addr_absx(), 4, 2); };                    /* SLO abs,X ~ Illegal OPCode */
      /* 0x20 ~ 0x2F */
      instruction_table[0x20] = [this]() { this->jsr(); };                                           /* JSR abs */
      instruction_table[0x21] = [this]() { this->_and(this->load_byte(this->addr_indx()), 6); };     /* AND (ind,X) */
      instruction_table[0x22] = [this]() { this->jam(0x22); };                                       /* JAM ~ Illegal OPCode */
      instruction_table[0x23] = [this]() { this->rla(this->addr_indx(), 5, 3); };                    /* RLA (ind,X) ~ Illegal OPCode */
      instruction_table[0x24] = [this]() { this->bit(this->addr_zero(), 3); };                       /* BIT zpg */
      instruction_table[0x25] = [this]() { this->_and(this->load_byte(this->addr_zero()), 3); };     /* AND zpg */
      instruction_table[0x26] = [this]() { this->rol_mem(this->addr_zero(), 5); };                   /* ROL zpg */
      instruction_table[0x27] = [this]() { this->rla(this->addr_zero(), 3, 2); };
      instruction_table[0x28] = [this]() { this->plp(); };
      instruction_table[0x29] = [this]() { this->_and(this->fetch_op(), 2); };
      instruction_table[0x2A] = [this]() { this->rol_a(); };
      instruction_table[0x2B] = [this]() { this->lxa(this->fetch_op(), 2); };
      instruction_table[0x2C] = [this]() { this->bit(this->addr_abs(), 4); };
      instruction_table[0x2D] = [this]() { this->_and(this->load_byte(this->addr_abs()), 4); };
      instruction_table[0x2E] = [this]() { this->rol_mem(this->addr_abs(), 6); };
      instruction_table[0x2F] = [this]() { this->rla_(this->addr_abs(), 4, 2); };                     /* RLA abs ~ Illegal OPCode */

      instruction_table[0x30] = [this]() { this->bmi(); };
      instruction_table[0x31] = [this]() { this->_and(this->load_byte(this->addr_indy()), 5); };
      instruction_table[0x32] = [this]() { this->jam(0x32); };
      instruction_table[0x33] = [this]() { this->rla(this->addr_indy(), 5, 3); };
      instruction_table[0x34] = [this]() { this->load_byte(this->addr_zerox()); this->nop(4); };
      instruction_table[0x35] = [this]() { this->_and(this->load_byte(this->addr_zerox()), 4); };
      instruction_table[0x36] = [this]() { this->rol_mem(this->addr_zerox(), 6); };
      instruction_table[0x37] = [this]() { this->rla(this->addr_zerox(), 4, 2); };
      instruction_table[0x38] = [this]() { this->sec(); };
      instruction_table[0x39] = [this]() { this->_and(this->load_byte(this->addr_absy()), 4); };
      instruction_table[0x3A] = [this]() { this->nop(2); };
      instruction_table[0x3B] = [this]() { this->rla(this->addr_absy(), 4, 2); };
      instruction_table[0x3C] = [this]() { this->load_byte(this->addr_absx()); this->nop(4); };
      instruction_table[0x3D] = [this]() { this->_and(this->load_byte(this->addr_absx()), 4); };
      instruction_table[0x3E] = [this]() { this->rol_mem(this->addr_absx(), 7); };
      instruction_table[0x3F] = [this]() { this->rla(this->addr_absx(), 4, 2); };

      instruction_table[0x40] = [this]() { this->rti(); };
      instruction_table[0x41] = [this]() { this->eor(this->load_byte(this->addr_indx()), 6); };
      instruction_table[0x42] = [this]() { this->jam(0x42); };
      instruction_table[0x43] = [this]() { this->sre(this->addr_indx(), 5, 3); };
      instruction_table[0x44] = [this]() { this->load_byte(this->addr_zero()); this->nop(3); };
      instruction_table[0x45] = [this]() { this->eor(this->load_byte(this->addr_zero()), 3); };
      instruction_table[0x46] = [this]() { this->lsr_mem(this->addr_zero(), 5); };
      instruction_table[0x47] = [this]() { this->sre(this->addr_zero(), 3, 2); };
      instruction_table[0x48] = [this]() { this->pha(); };
      instruction_table[0x49] = [this]() { this->eor(this->fetch_op(), 2); };
      instruction_table[0x4A] = [this]() { this->lsr_a(); };
      instruction_table[0x4B] = [this]() { this->_and(fetch_op(),0); this->lsr_a(); };
      instruction_table[0x4C] = [this]() { this->jmp(); };
      instruction_table[0x4D] = [this]() { this->eor(this->load_byte(this->addr_abs()), 4); };
      instruction_table[0x4E] = [this]() { this->lsr_mem(this->addr_abs(), 6); };
      instruction_table[0x4F] = [this]() { this->sre(this->addr_abs(), 4, 2); };

      instruction_table[0x50] = [this]() { this->bvc(); };
      instruction_table[0x51] = [this]() { this->eor(this->load_byte(this->addr_indy()), 5); };
      instruction_table[0x52] = [this]() { this->jam(0x52); };
      instruction_table[0x53] = [this]() { this->sre(this->addr_indy(), 5, 3); };
      instruction_table[0x54] = [this]() { this->load_byte(this->addr_zerox()); this->nop(4); };
      instruction_table[0x55] = [this]() { this->eor(this->load_byte(this->addr_zerox()), 4); };
      instruction_table[0x56] = [this]() { this->lsr_mem(this->addr_zerox(), 6); };
      instruction_table[0x57] = [this]() { this->sre(this->addr_zerox(), 4, 2); };
      instruction_table[0x58] = [this]() { this->cli(); };
      instruction_table[0x59] = [this]() { this->eor(this->load_byte(this->addr_absy()), 4); };
      instruction_table[0x5A] = [this]() { this->nop(2); };
      instruction_table[0x5B] = [this]() { this->sre(this->addr_absy(), 4, 2); };
      instruction_table[0x5C] = [this]() { this->load_byte(this->addr_absx()); this->nop(4); };
      instruction_table[0x5D] = [this]() { this->eor(this->load_byte(this->addr_absx()), 4); };
      instruction_table[0x5E] = [this]() { this->lsr_mem(this->addr_absx(), 7); };
      instruction_table[0x5F] = [this]() { this->sre(this->addr_absx(), 4, 2); };

      instruction_table[0x60] = [this]() { this->rts(); };
      instruction_table[0x61] = [this]() { this->adc(this->load_byte(this->addr_indx()), 6); };
      instruction_table[0x62] = [this]() { this->jam(0x62); };
      instruction_table[0x63] = [this]() { this->rra(this->addr_indx(), 5, 3); };
      instruction_table[0x64] = [this]() { this->load_byte(this->addr_zero()); this->nop(3); };
      instruction_table[0x65] = [this]() { this->adc(this->load_byte(this->addr_zero()), 3); };
      instruction_table[0x66] = [this]() { this->ror_mem(this->addr_zero(), 5); };
      instruction_table[0x67] = [this]() { this->rra(this->addr_zero(), 3, 2); };
      instruction_table[0x68] = [this]() { this->pla(); };
      instruction_table[0x69] = [this]() { this->adc(this->fetch_op(), 2); };
      instruction_table[0x6A] = [this]() { this->ror_a(); };
      instruction_table[0x6B] = [this]() { this->arr(); };
      instruction_table[0x6C] = [this]() { this->jmp_ind(); };
      instruction_table[0x6D] = [this]() { this->adc(this->load_byte(this->addr_abs()), 4); };
      instruction_table[0x6E] = [this]() { this->ror_mem(this->addr_abs(), 6); };
      instruction_table[0x6F] = [this]() { this->rra(this->addr_abs(), 4, 2); };

      instruction_table[0x70] = [this]() { this->bvs(); };
      instruction_table[0x71] = [this]() { this->adc(this->load_byte(this->addr_indy()), 5); };
      instruction_table[0x72] = [this]() { this->jam(0x72); };
      instruction_table[0x73] = [this]() { this->rra(this->addr_indy(), 5, 3); };
      instruction_table[0x74] = [this]() { this->load_byte(this->addr_zerox()); this->nop(4); };
      instruction_table[0x75] = [this]() { this->adc(this->load_byte(this->addr_zerox()), 4); };
      instruction_table[0x76] = [this]() { this->ror_mem(this->addr_zerox(), 6); };
      instruction_table[0x77] = [this]() { this->rra(this->addr_zerox(), 4, 2); };
      instruction_table[0x78] = [this]() { this->sei(); };
      instruction_table[0x79] = [this]() { this->adc(this->load_byte(this->addr_absy()), 4); };
      instruction_table[0x7A] = [this]() { this->nop(2); };
      instruction_table[0x7B] = [this]() { this->rra(this->addr_absy(), 4, 2); };
      instruction_table[0x7C] = [this]() { this->load_byte(this->addr_absx()); this->nop(4); };
      instruction_table[0x7D] = [this]() { this->adc(this->load_byte(this->addr_absx()), 4); };
      instruction_table[0x7E] = [this]() { this->ror_mem(this->addr_absx(), 7); };
      instruction_table[0x7F] = [this]() { this->rra(this->addr_absx(), 4, 2); };

      instruction_table[0x80] = [this]() { this->fetch_op(); this->nop(2); };
      instruction_table[0x81] = [this]() { this->sta(this->addr_indx(), 6); };
      instruction_table[0x82] = [this]() { this->fetch_op(); this->nop(2); };
      instruction_table[0x83] = [this]() { this->sax(this->addr_indx(), 3); };
      instruction_table[0x84] = [this]() { this->sty(this->addr_zero(), 3); };
      instruction_table[0x85] = [this]() { this->sta(this->addr_zero(), 3); };
      instruction_table[0x86] = [this]() { this->stx(this->addr_zero(), 3); };
      instruction_table[0x87] = [this]() { this->sax(this->addr_zero(), 3); };
      instruction_table[0x88] = [this]() { this->dey(); };
      instruction_table[0x89] = [this]() { this->fetch_op(); this->nop(2); };
      instruction_table[0x8A] = [this]() { this->txa(); };
      instruction_table[0x8B] = [this]() { this->tas(this->addr_abs(), 4); };
      instruction_table[0x8C] = [this]() { this->sty(this->addr_abs(), 4); };
      instruction_table[0x8D] = [this]() { this->sta(this->addr_abs(), 4); };
      instruction_table[0x8E] = [this]() { this->stx(this->addr_abs(), 4); };
      instruction_table[0x8F] = [this]() { this->sax(this->addr_abs(), 4); };

      instruction_table[0x90] = [this]() { this->bcc(); };
      instruction_table[0x91] = [this]() { this->sta(this->addr_indy(), 6); };
      instruction_table[0x92] = [this]() { this->jam(0x92); };
      instruction_table[0x93] = [this]() { this->sha(this->addr_indy(), 6); };
      instruction_table[0x94] = [this]() { this->sty(this->addr_zerox(), 4); };
      instruction_table[0x95] = [this]() { this->sta(this->addr_zerox(), 4); };
      instruction_table[0x96] = [this]() { this->stx(this->addr_zeroy(), 4); };
      instruction_table[0x97] = [this]() { this->sax(this->addr_zeroy(), 4); };
      instruction_table[0x98] = [this]() { this->tya(); };
      instruction_table[0x99] = [this]() { this->sta(this->addr_absy(), 5); };
      instruction_table[0x9A] = [this]() { this->txs(); };
      instruction_table[0x9B] = [this]() { this->tas(this->addr_absy(), 5); };
      instruction_table[0x9C] = [this]() { this->shy(this->addr_absx(), 5); };
      instruction_table[0x9D] = [this]() { this->sta(this->addr_absx(), 5); };
      instruction_table[0x9E] = [this]() { this->shx(this->addr_absy(), 5); };
      instruction_table[0x9F] = [this]() { this->sha(this->addr_absy(), 5); };

      instruction_table[0xA0] = [this]() { this->ldy(this->fetch_op(), 2); };
      instruction_table[0xA1] = [this]() { this->lda(this->load_byte(this->addr_indx()), 6); };
      instruction_table[0xA2] = [this]() { this->ldx(this->fetch_op(), 2); };
      instruction_table[0xA3] = [this]() { this->lax(this->load_byte(this->addr_indx()), 6); };
      instruction_table[0xA4] = [this]() { this->ldy(this->load_byte(this->addr_zero()), 3); };
      instruction_table[0xA5] = [this]() { this->lda(this->load_byte(this->addr_zero()), 3); };
      instruction_table[0xA6] = [this]() { this->ldx(this->load_byte(this->addr_zero()), 3); };
      instruction_table[0xA7] = [this]() { this->lax(this->load_byte(this->addr_zero()), 3); };
      instruction_table[0xA8] = [this]() { this->tay(); };
      instruction_table[0xA9] = [this]() { this->lda(this->fetch_op(), 2); };
      instruction_table[0xAA] = [this]() { this->tax(); };
      instruction_table[0xAB] = [this]() { this->lxa(this->fetch_op(), 2); };
      instruction_table[0xAC] = [this]() { this->ldy(this->load_byte(this->addr_abs()), 4); };
      instruction_table[0xAD] = [this]() { this->lda(this->load_byte(this->addr_abs()), 4); };
      instruction_table[0xAE] = [this]() { this->ldx(this->load_byte(this->addr_abs()), 4); };
      instruction_table[0xAF] = [this]() { this->lax(this->load_byte(this->addr_abs()), 4); };

      instruction_table[0xB0] = [this]() { this->bcs(); };
      instruction_table[0xB1] = [this]() { this->lda(this->load_byte(this->addr_indy()), 5); };
      instruction_table[0xB2] = [this]() { this->jam(0xB2); };
      instruction_table[0xB3] = [this]() { this->lax(this->load_byte(this->addr_indy()), 4); };
      instruction_table[0xB4] = [this]() { this->ldy(this->load_byte(this->addr_zerox()), 4); };
      instruction_table[0xB5] = [this]() { this->lda(this->load_byte(this->addr_zerox()), 4); };
      instruction_table[0xB6] = [this]() { this->ldx(this->load_byte(this->addr_zeroy()), 4); };
      instruction_table[0xB7] = [this]() { this->lax(this->load_byte(this->addr_zeroy()), 4); };
      instruction_table[0xB8] = [this]() { this->clv(); };
      instruction_table[0xB9] = [this]() { this->lda(this->load_byte(this->addr_absy()), 4); };
      instruction_table[0xBA] = [this]() { this->tsx(); };
      instruction_table[0xBB] = [this]() { this->las(this->load_byte(this->addr_absy())); };
      instruction_table[0xBC] = [this]() { this->ldy(this->load_byte(this->addr_absx()), 4); };
      instruction_table[0xBD] = [this]() { this->lda(this->load_byte(this->addr_absx()), 4); };
      instruction_table[0xBE] = [this]() { this->ldx(this->load_byte(this->addr_absy()), 4); };
      instruction_table[0xBF] = [this]() { this->lax(this->load_byte(this->addr_absy()), 4); };

      instruction_table[0xC0] = [this]() { this->cpy(this->fetch_op(), 2); };
      instruction_table[0xC1] = [this]() { this->cmp(this->load_byte(this->addr_indx()), 6); };
      instruction_table[0xC2] = [this]() { this->fetch_op(); this->nop(2); };
      instruction_table[0xC3] = [this]() { this->dcp(this->addr_indx(), 5, 3); };
      instruction_table[0xC4] = [this]() { this->cpy(this->load_byte(this->addr_zero()), 3); };
      instruction_table[0xC5] = [this]() { this->cmp(this->load_byte(this->addr_zero()), 3); };
      instruction_table[0xC6] = [this]() { this->dec(this->addr_zero(), 5); };
      instruction_table[0xC7] = [this]() { this->dcp(this->addr_zero(), 3, 2); };
      instruction_table[0xC8] = [this]() { this->iny(); };
      instruction_table[0xC9] = [this]() { this->cmp(this->fetch_op(), 2); };
      instruction_table[0xCA] = [this]() { this->dex(); };
      instruction_table[0xCB] = [this]() { this->sbx(this->fetch_op(), 2); };
      instruction_table[0xCC] = [this]() { this->cpy(this->load_byte(this->addr_abs()), 4); };
      instruction_table[0xCD] = [this]() { this->cmp(this->load_byte(this->addr_abs()), 4); };
      instruction_table[0xCE] = [this]() { this->dec(this->addr_abs(), 6); };
      instruction_table[0xCF] = [this]() { this->dcp(this->addr_abs(), 4, 2); };

      instruction_table[0xD0] = [this]() { this->bne(); };
      instruction_table[0xD1] = [this]() { this->cmp(this->load_byte(this->addr_indy()), 5); };
      instruction_table[0xD2] = [this]() { this->jam(0xD2); };
      instruction_table[0xD3] = [this]() { this->dcp(this->addr_indy(), 5, 3); };
      instruction_table[0xD4] = [this]() { this->load_byte(this->addr_zerox()); this->nop(4); };
      instruction_table[0xD5] = [this]() { this->cmp(this->load_byte(this->addr_zerox()), 4); };
      instruction_table[0xD6] = [this]() { this->dec(this->addr_zerox(), 6); };
      instruction_table[0xD7] = [this]() { this->dcp(this->addr_zerox(), 4, 2); };
      instruction_table[0xD8] = [this]() { this->cld(); };
      instruction_table[0xD9] = [this]() { this->cmp(this->load_byte(this->addr_absy()), 4); };
      instruction_table[0xDA] = [this]() { this->nop(2); };
      instruction_table[0xDB] = [this]() { this->dcp(this->addr_absy(), 4, 2); };
      instruction_table[0xDC] = [this]() { this->load_byte(this->addr_absx()); this->nop(4); };
      instruction_table[0xDD] = [this]() { this->cmp(this->load_byte(this->addr_absx()), 4); };
      instruction_table[0xDE] = [this]() { this->dec(this->addr_absx(), 7); };
      instruction_table[0xDF] = [this]() { this->dcp(this->addr_absx(), 5, 2); };

      instruction_table[0xE0] = [this]() { this->cpx(this->fetch_op(), 2); };
      instruction_table[0xE1] = [this]() { this->sbc(this->load_byte(this->addr_indx()), 6); };
      instruction_table[0xE2] = [this]() { this->fetch_op(); this->nop(2); };
      instruction_table[0xE3] = [this]() { this->isc(this->addr_indx(), 8); };
      instruction_table[0xE4] = [this]() { this->cpx(this->load_byte(this->addr_zero()), 3); };
      instruction_table[0xE5] = [this]() { this->sbc(this->load_byte(this->addr_zero()), 3); };
      instruction_table[0xE6] = [this]() { this->inc(this->addr_zero(), 5); };
      instruction_table[0xE7] = [this]() { this->isc(this->addr_zero(), 5); };
      instruction_table[0xE8] = [this]() { this->inx(); };
      instruction_table[0xE9] = [this]() { this->sbc(this->fetch_op(), 2); };
      instruction_table[0xEA] = [this]() { this->nop(2); };
      instruction_table[0xEB] = [this]() { this->sbc(this->fetch_op(), 2); };
      instruction_table[0xEC] = [this]() { this->cpx(this->load_byte(this->addr_abs()), 4); };
      instruction_table[0xED] = [this]() { this->sbc(this->load_byte(this->addr_abs()), 4); };
      instruction_table[0xEE] = [this]() { this->inc(this->addr_abs(), 6); };
      instruction_table[0xEF] = [this]() { this->isc(this->addr_abs(), 6); };

      instruction_table[0xF0] = [this]() { this->beq(); };
      instruction_table[0xF1] = [this]() { this->sbc(this->load_byte(this->addr_indy()), 5); };
      instruction_table[0xF2] = [this]() { this->jam(0xF2); };
      instruction_table[0xF3] = [this]() { this->isc(this->addr_indy(), 8); };
      instruction_table[0xF4] = [this]() { this->load_byte(this->addr_zerox()); this->nop(4); };
      instruction_table[0xF5] = [this]() { this->sbc(this->load_byte(this->addr_zerox()), 4); };
      instruction_table[0xF6] = [this]() { this->inc(this->addr_zerox(), 6); };
      instruction_table[0xF7] = [this]() { this->isc(this->addr_zerox(), 6); };
      instruction_table[0xF8] = [this]() { this->sed(); };
      instruction_table[0xF9] = [this]() { this->sbc(this->load_byte(this->addr_absy()), 4); };
      instruction_table[0xFA] = [this]() { this->nop(2); };
      instruction_table[0xFB] = [this]() { this->isc(this->addr_absy(), 6); };
      instruction_table[0xFC] = [this]() { this->load_byte(this->addr_absx()); this->nop(4); };
      instruction_table[0xFD] = [this]() { this->sbc(this->load_byte(this->addr_absx()), 4); };
      instruction_table[0xFE] = [this]() { this->inc(this->addr_absx(), 7); };
      instruction_table[0xFF] = [this]() { this->isc(this->addr_absx(), 7); };

    };

    void execute_opcode(uint8_t insn) {
      if (instruction_table[insn]) {  /* Check if the entry is initialized */
        this->instruction_table[insn]();
      } else {
        std::cerr << "Fatal Error: Attempted to execute uninitialized opcode: 0x"
                  << std::hex << static_cast<int>(insn) << std::endl;
      }
    }
};


#endif /* EMUDORE_CPU_H */
