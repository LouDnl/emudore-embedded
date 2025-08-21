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
