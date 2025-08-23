/*
 * Cartridge expansion port
 * Copyright (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * cart.h
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

#ifndef EMUDORE_C64CART_H
#define EMUDORE_C64CART_H


/* Pre declarations */
class C64;
class MC68B50;

class Cart
{
  private:
    /* MC68B50 ACIA ROM & RAM */
    uint8_t *mem_rom_mc6850_;

    /* Main pointer */
    C64 * c64_;

    /* Cart yes or no? */
    bool havecart = false;
    bool cartactive = false;

  public:
    Cart(C64 * c64);
    ~Cart();

    void reset(void);
    void emulate(void);

    void write_register(uint8_t r, uint8_t v);
    uint8_t read_register(uint8_t r);

    /* Emulated cart ic's */
    MC68B50 *mc6850_;

    /* MC68B50 ACIA ROM & RAM */
    void init_mc6850(void);
    bool acia_active = false;
    uint8_t *k6850MemWr; /* $de00 */
    uint8_t *k6850MemRd; /* $de00 */

};


#endif /* EMUDORE_C64CART_H */
