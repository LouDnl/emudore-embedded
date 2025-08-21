#ifndef EMUDORE_65B80_H
#define EMUDORE_65B80_H


#include <iostream>
#include <cstdint>

/* Fake data for testing */
/* NoteOn/Channel1, Note, Touchspeed */
static uint8_t midi_keydown[3] = { 0x90, 0x1F, 0x3F };
/* NoteOff/Channel1, Note, Touchspeed */
static uint8_t midi_keyup[3] = { 0x80, 0x1F, 0x1F };
static bool _keydown = false, _keyup = true;
static int n_read = 3;

/**
 * @brief Motorola 68B50 Asynchronous Communications Interface Adapter
 *
 * Emulates the Motorola 68B50 ACIA (for Midi and or Uart communication)
 *
 */
class MC68B50
{
  private:

    /* Main pointer */
    C64 *c64_;

  public:
    MC68B50(C64 * c64);
    ~MC68B50();

    void write_register(uint8_t r, uint8_t v);
    uint8_t read_register(uint8_t r);

    /* Handle midi keyboard keydown and keyup */
    void fake_keydown(void);
    void fake_keyup(void);

    void reset();
    void emulate();

};


#endif /* EMUDORE_65B80_H */
