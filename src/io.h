/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * io.h
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

#ifndef EMUDORE_IO_H
#define EMUDORE_IO_H

#include <queue>
#include <chrono>
#include <thread>
#include <vector>
#include <utility>
#include <unordered_map>

#if DESKTOP
#if SDL_ENABLED
#include <SDL.h>
#else
#include <SDL_scancode.h>
typedef uint8_t SDL_Keycode;
#endif
#endif /* DESKTOP */


/**
 * @brief IO devices
 *
 * This class implements Input/Output devices connected to the
 * Commodore 64 such as the screen and keyboard.
 *
 * Current backend is SDL2.
 */
class IO
{
  private:
    C64 *c64_;
    #if DESKTOP
    #if SDL_ENABLED
    SDL_Window *window_;
    SDL_Renderer *renderer_;
    SDL_Texture *texture_;
    SDL_PixelFormat *format_;
    #endif /* SDL_ENABLED */
    uint32_t *frame_;
    #endif /* DESKTOP */
    size_t cols_;
    size_t rows_;
    #if DESKTOP
    unsigned int color_palette[16];
    #endif
    uint8_t keyboard_matrix_[8];
    bool retval_ = true;
    /* keyboard mappings */
    #if DESKTOP
    std::unordered_map<SDL_Keycode,std::pair<int,int>> keymap_;
    std::unordered_map<char,std::vector<SDL_Keycode>> charmap_;
    #endif
    enum kKeyEvent
    {
      kPress,
      kRelease,
    };
    /* key events */
    #if DESKTOP
    std::queue<std::pair<kKeyEvent,SDL_Keycode>> key_event_queue_;
    unsigned int next_key_event_at_;
    static const int kWait = 18000;
    #endif
    /* vertical refresh sync */
    std::chrono::high_resolution_clock::time_point prev_frame_was_at_;
    void vsync();

    /* Key combination vars */
    static bool runstop;
    static bool shiftlock;

    /* Disk drive */
    static bool diskpresent;
  public:
    IO(C64 *c64, bool sdl);
    ~IO();
    bool nosdl;
    void reset(void);
    bool emulate();
    void process_events();

    void init_color_palette();
    void init_keyboard();
    #if DESKTOP
    void handle_keydown(SDL_Keycode k);
    void handle_keyup(SDL_Keycode k);
    void type_character(char c);
    #endif
    inline uint8_t keyboard_matrix_row(int col){return keyboard_matrix_[col];};
    void screen_update_pixel(int x, int y, int color);
    void screen_draw_rect(int x, int y, int n, int color);
    void screen_draw_border(int y, int color);
    void screen_refresh();

    /* Needs moving to independent class */
    void set_disk_loaded(bool ready){diskpresent = ready;};
    bool disk_loaded(void){return diskpresent;};
    const int num_sectors[41] = {
      0,
      21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21, /* 1 ~ 17 */
      19,19,19,19,19,19,19,
      18,18,18,18,18,18,
      17,17,17,17,17,
      17,17,17,17,17		// Tracks 36..40
    };

    const int sector_offset[41] = {
      0,
      0,21,42,63,84,105,126,147,168,189,210,231,252,273,294,315,336,
      357,376,395,414,433,452,471,
      490,508,526,544,562,580,
      598,615,632,649,666,
      683,700,717,734,751	// Tracks 36..40
    };
};

// inline member functions accesible from other classes /////////////////////

inline void IO::screen_update_pixel(int x, int y, int color)
{
  #if DESKTOP
  frame_[y * cols_  + x] = color_palette[color & 0xf];
  #endif
};

#endif /* EMUDORE_IO_H */
