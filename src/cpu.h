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

#ifndef EMUDORE_CPU_H
#define EMUDORE_CPU_H


#include <cstdint>
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
#define SR_CARRY         0x01

#define ANE_MAGIC        0xef

/* macro helpers */

#define SET_ZF(val)     (zf(!(uint8_t)(val)))
#define SET_NF(val)     (nf(((uint8_t)(val)&0x80)!=0))


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
  private:
    /* registers */
    uint16_t pc_;
    uint8_t sp_, a_, x_, y_;
    /* flags (p/status reg) */
    bool cf_,zf_,idf_,dmf_,bcf_,of_,nf_;
    /* c64->memory and clock */
    C64 *c64_;
    static unsigned int cycles_;
    /* helpers */
    uint16_t curr_page; /* current page at start of cpu emulation */
    bool pb_crossed;    /* true if page boundary crossed */

    inline uint8_t load_byte(uint16_t addr);
    inline void push(uint8_t);
    inline uint8_t pop();
    inline uint8_t fetch_op();
    inline uint16_t fetch_opw();
    inline uint16_t addr_zero();
    inline uint16_t addr_zerox();
    inline uint16_t addr_zeroy();
    inline uint16_t addr_abs();
    inline uint16_t addr_absy();
    inline uint16_t addr_absx();
    inline uint16_t addr_indx();
    inline uint16_t addr_indy();

    inline uint8_t rol(uint8_t v);
    inline uint8_t ror(uint8_t v);
    inline uint8_t lsr(uint8_t v);
    inline uint8_t asl(uint8_t v);
    inline void tick(uint8_t v){cycles_+=v;};
    inline void backtick(uint8_t v){cycles_-=v;};
    inline uint8_t flags();
    inline void flags(uint8_t v);
    /* instructions : data handling and memory operations */
    inline void sta(uint16_t addr, uint8_t cycles);
    inline void stx(uint16_t addr, uint8_t cycles);
    inline void sty(uint16_t addr, uint8_t cycles);
    inline void lda(uint8_t v, uint8_t cycles);
    inline void ldx(uint8_t v, uint8_t cycles);
    inline void ldy(uint8_t v, uint8_t cycles);
    inline void txs();
    inline void tsx();
    inline void tax();
    inline void txa();
    inline void tay();
    inline void tya();
    inline void pha();
    inline void pla();
    /* instructions: logic operations */
    inline void ora(uint8_t v, uint8_t cycles);
    inline void _and(uint8_t v, uint8_t cycles);
    inline void bit(uint16_t addr, uint8_t cycles);
    inline void rol_a();
    inline void rol_mem(uint16_t addr, uint8_t cycles);
    inline void ror_a();
    inline void ror_mem(uint16_t addr, uint8_t cycles);
    inline void asl_a();
    inline void asl_mem(uint16_t addr, uint8_t cycles);
    inline void lsr_a();
    inline void lsr_mem(uint16_t addr, uint8_t cycles);
    inline void eor(uint8_t v, uint8_t cycles);
    /* instructions: arithmetic operations */
    inline void inc(uint16_t addr, uint8_t cycles);
    inline void dec(uint16_t addr, uint8_t cycles);
    inline void inx();
    inline void iny();
    inline void dex();
    inline void dey();
    inline void adc(uint8_t v, uint8_t cycles);
    inline void sbc(uint8_t v, uint8_t cycles);
    /* instructions: flag access */
    inline void sei();
    inline void cli();
    inline void sec();
    inline void clc();
    inline void sed();
    inline void cld();
    inline void clv();
    inline void php();
    inline void plp();
    /* instructions: control flow */
    inline void cmp(uint8_t v, uint8_t cycles);
    inline void cpx(uint8_t v, uint8_t cycles);
    inline void cpy(uint8_t v, uint8_t cycles);
    inline void rts();
    inline void jsr();
    inline void bne();
    inline void beq();
    inline void bcs();
    inline void bcc();
    inline void bpl();
    inline void bmi();
    inline void bvc();
    inline void bvs();
    inline void jmp();
    inline void jmp_ind();
    /* instructions: misc */
    inline void nop(uint8_t cycles);
    inline void brk();
    inline void rti();
    /* Instructions: illegal */
    inline void jam(uint8_t insn);
    inline void slo(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    inline void lxa(uint8_t v, uint8_t cycles);
    inline void las(uint8_t v);
    inline void shy(uint16_t addr, uint8_t cycles);
    inline void shx(uint16_t addr, uint8_t cycles);
    inline void sha(uint16_t addr, uint8_t cycles);
    inline void sax(uint16_t addr, uint8_t cycles);
    inline void sre(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    inline void rla(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    inline void rra(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    inline void dcp(uint16_t addr, uint8_t cycles_a, uint8_t cycles_b);
    inline void tas(uint16_t addr, uint8_t cycles);
    inline void sbx(uint8_t v, uint8_t cycles);
    inline void isc(uint16_t addr, uint8_t cycles);
    inline void arr();
    inline void xaa(uint8_t v);
  public:
    Cpu(C64 * c64);
    /* cpu state */
    void reset();
    bool emulate();
    /* register access */
    inline uint16_t pc() {return pc_;};
    inline void pc(uint16_t v) {pc_=v;};
    inline uint8_t sp() {return sp_;};
    inline void sp(uint8_t v) {sp_=v;};
    inline uint8_t a() {return a_;};
    inline void a(uint8_t v) {a_=v;};
    inline uint8_t x() {return x_;};
    inline void x(uint8_t v) {x_=v;};
    inline uint8_t y() {return y_;};
    inline void y(uint8_t v) {y_=v;};
    /* flags */
    inline bool cf() {return cf_;};
    inline void cf(bool v) { cf_=v;};
    inline bool zf() {return zf_;};
    inline void zf(bool v) {zf_=v;};
    inline bool idf() {return idf_;};
    inline void idf(bool v) { idf_=v;};
    inline bool dmf() {return dmf_;};
    inline void dmf(bool v) { dmf_=v;};
    inline bool bcf() {return bcf_;};
    inline void bcf(bool v) { bcf_=v;};
    inline bool of() {return of_;};
    inline void of(bool v) { of_=v;};
    inline bool nf() {return nf_;};
    inline void nf(bool v) {nf_=v;};
    /* clock */
    inline unsigned int cycles(){return cycles_;};
    inline void cycles(unsigned int v){cycles_=v;};
    inline void cyclestick(unsigned int v){cycles_+=v;};
    /* interrupts */
    void nmi();
    void irq();
    /* debug */
    static bool loginstructions;
    static bool logillegals;
    void dump_flags();
    void dump_flags(uint8_t flags);
    void dump_regs();
    void dump_regs_insn(uint8_t insn);
    void dump_regs_json();
    void dbg();
    void dbg_a();
    void dbg_b();
};


#endif /* EMUDORE_CPU_H */
