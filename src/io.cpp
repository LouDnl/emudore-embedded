/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * io.cpp
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

#include <stdexcept>

#include <c64.h>
#include <io.h>


// clas ctor and dtor //////////////////////////////////////////////////////////

/* Key combination vars */
bool IO::runstop = false;
bool IO::shiftlock = false;

/* Disk drive */
bool IO::diskpresent = false;

IO::IO(C64 *c64,bool sdl) :
  c64_(c64),
  nosdl(sdl)
{
  #if DESKTOP
  // printf("sdl: %d %d\n",sdl,nosdl);
  #if SDL_ENABLED
  if(!nosdl) {
    SDL_Init(SDL_INIT_VIDEO);
    /**
    * We create the window double the original pixel size,
    * the renderer takes care of upscaling
    */
    window_ = SDL_CreateWindow(
          "emudore",
          SDL_WINDOWPOS_UNDEFINED,
          SDL_WINDOWPOS_UNDEFINED,
          Vic::kVisibleScreenWidth * 2,
          Vic::kVisibleScreenHeight * 2,
          SDL_WINDOW_OPENGL
    );
  }
  #endif
  #endif /* DESKTOP */
  cols_ = Vic::kVisibleScreenWidth;
  rows_ = Vic::kVisibleScreenHeight;
  /* use a single texture and hardware acceleration */
  #if DESKTOP
  #if SDL_ENABLED
  if(!nosdl) {
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    texture_  = SDL_CreateTexture(renderer_,
                                  SDL_PIXELFORMAT_ARGB8888,
                                  SDL_TEXTUREACCESS_STREAMING,
                                  cols_,
                                  rows_);
    format_ = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    }
  #endif
  #endif /* DESKTOP */
  /**
   * unfortunately, we need to keep a copy of the rendered frame
   * in our own memory, there does not seem to be a way around
   * that would allow manipulating pixels straight on the GPU
   * memory due to how the image is internally stored, etc..
   *
   * The rendered frame gets uploaded to the GPU on every
   * screen refresh.
   */
  #if DESKTOP
  frame_  = new uint32_t[cols_ * rows_]();
  #endif
  init_color_palette();
  init_keyboard();
  #if DESKTOP
  next_key_event_at_ = 0;
  #endif
  prev_frame_was_at_ = std::chrono::high_resolution_clock::now();
}

IO::~IO()
{
  #if DESKTOP
  delete [] frame_;
  #if SDL_ENABLED
  if(!nosdl) {
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyTexture(texture_);
    SDL_FreeFormat(format_);
    SDL_Quit();
  }
  #endif
  #endif /* DESKTOP */
}

void IO::reset()
{
  #if DESKTOP
  next_key_event_at_ = 0;
  #endif
  prev_frame_was_at_ = std::chrono::high_resolution_clock::now();
}

// init io devices  ////////////////////////////////////////////////////////////

/**
 * @brief init keyboard state and keymap
 */
void IO::init_keyboard()
{
  /* init keyboard matrix state */
  for(size_t i=0 ; i < sizeof(keyboard_matrix_) ; i++)
  {
    keyboard_matrix_[i] = 0xff;
  }
  /* character to sdl key map */
  #if DESKTOP
  charmap_['A']  = {SDL_SCANCODE_A};
  charmap_['B']  = {SDL_SCANCODE_B};
  charmap_['C']  = {SDL_SCANCODE_C};
  charmap_['D']  = {SDL_SCANCODE_D};
  charmap_['E']  = {SDL_SCANCODE_E};
  charmap_['F']  = {SDL_SCANCODE_F};
  charmap_['G']  = {SDL_SCANCODE_G};
  charmap_['H']  = {SDL_SCANCODE_H};
  charmap_['I']  = {SDL_SCANCODE_I};
  charmap_['J']  = {SDL_SCANCODE_J};
  charmap_['K']  = {SDL_SCANCODE_K};
  charmap_['L']  = {SDL_SCANCODE_L};
  charmap_['M']  = {SDL_SCANCODE_M};
  charmap_['N']  = {SDL_SCANCODE_N};
  charmap_['O']  = {SDL_SCANCODE_O};
  charmap_['P']  = {SDL_SCANCODE_P};
  charmap_['Q']  = {SDL_SCANCODE_Q};
  charmap_['R']  = {SDL_SCANCODE_R};
  charmap_['S']  = {SDL_SCANCODE_S};
  charmap_['T']  = {SDL_SCANCODE_T};
  charmap_['U']  = {SDL_SCANCODE_U};
  charmap_['V']  = {SDL_SCANCODE_V};
  charmap_['W']  = {SDL_SCANCODE_W};
  charmap_['X']  = {SDL_SCANCODE_X};
  charmap_['Y']  = {SDL_SCANCODE_Y};
  charmap_['Z']  = {SDL_SCANCODE_Z};
  charmap_['1']  = {SDL_SCANCODE_1};
  charmap_['2']  = {SDL_SCANCODE_2};
  charmap_['3']  = {SDL_SCANCODE_3};
  charmap_['4']  = {SDL_SCANCODE_4};
  charmap_['5']  = {SDL_SCANCODE_5};
  charmap_['6']  = {SDL_SCANCODE_6};
  charmap_['7']  = {SDL_SCANCODE_7};
  charmap_['8']  = {SDL_SCANCODE_8};
  charmap_['9']  = {SDL_SCANCODE_9};
  charmap_['0']  = {SDL_SCANCODE_0};
  charmap_['\n'] = {SDL_SCANCODE_RETURN};
  charmap_[' ']  = {SDL_SCANCODE_SPACE};
  charmap_[',']  = {SDL_SCANCODE_COMMA};
  charmap_['.']  = {SDL_SCANCODE_PERIOD};
  charmap_['/']  = {SDL_SCANCODE_SLASH};
  charmap_[';']  = {SDL_SCANCODE_SEMICOLON};
  charmap_['=']  = {SDL_SCANCODE_EQUALS};
  charmap_['-']  = {SDL_SCANCODE_MINUS};
  charmap_[':']  = {SDL_SCANCODE_BACKSLASH};
  charmap_['+']  = {SDL_SCANCODE_LEFTBRACKET};
  charmap_['*']  = {SDL_SCANCODE_RIGHTBRACKET};
  charmap_['@']  = {SDL_SCANCODE_APOSTROPHE};
  charmap_['(']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_8};
  charmap_[')']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_9};
  charmap_['<']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_COMMA};
  charmap_['>']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_PERIOD};
  charmap_['"']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_2};
  charmap_['$']  = {SDL_SCANCODE_LSHIFT,SDL_SCANCODE_4};
  /* keymap letters */
  keymap_[SDL_SCANCODE_A] = std::make_pair(1,2);
  keymap_[SDL_SCANCODE_B] = std::make_pair(3,4);
  keymap_[SDL_SCANCODE_C] = std::make_pair(2,4);
  keymap_[SDL_SCANCODE_D] = std::make_pair(2,2);
  keymap_[SDL_SCANCODE_E] = std::make_pair(1,6);
  keymap_[SDL_SCANCODE_F] = std::make_pair(2,5);
  keymap_[SDL_SCANCODE_G] = std::make_pair(3,2);
  keymap_[SDL_SCANCODE_H] = std::make_pair(3,5);
  keymap_[SDL_SCANCODE_I] = std::make_pair(4,1);
  keymap_[SDL_SCANCODE_J] = std::make_pair(4,2);
  keymap_[SDL_SCANCODE_K] = std::make_pair(4,5);
  keymap_[SDL_SCANCODE_L] = std::make_pair(5,2);
  keymap_[SDL_SCANCODE_M] = std::make_pair(4,4);
  keymap_[SDL_SCANCODE_N] = std::make_pair(4,7);
  keymap_[SDL_SCANCODE_O] = std::make_pair(4,6);
  keymap_[SDL_SCANCODE_P] = std::make_pair(5,1);
  keymap_[SDL_SCANCODE_Q] = std::make_pair(7,6);
  keymap_[SDL_SCANCODE_R] = std::make_pair(2,1);
  keymap_[SDL_SCANCODE_S] = std::make_pair(1,5);
  keymap_[SDL_SCANCODE_T] = std::make_pair(2,6);
  keymap_[SDL_SCANCODE_U] = std::make_pair(3,6);
  keymap_[SDL_SCANCODE_V] = std::make_pair(3,7);
  keymap_[SDL_SCANCODE_W] = std::make_pair(1,1);
  keymap_[SDL_SCANCODE_X] = std::make_pair(2,7);
  keymap_[SDL_SCANCODE_Y] = std::make_pair(3,1);
  keymap_[SDL_SCANCODE_Z] = std::make_pair(1,4);
  /* keymap numbers */
  keymap_[SDL_SCANCODE_1] = std::make_pair(7,0);
  keymap_[SDL_SCANCODE_2] = std::make_pair(7,3);
  keymap_[SDL_SCANCODE_3] = std::make_pair(1,0);
  keymap_[SDL_SCANCODE_4] = std::make_pair(1,3);
  keymap_[SDL_SCANCODE_5] = std::make_pair(2,0);
  keymap_[SDL_SCANCODE_6] = std::make_pair(2,3);
  keymap_[SDL_SCANCODE_7] = std::make_pair(3,0);
  keymap_[SDL_SCANCODE_8] = std::make_pair(3,3);
  keymap_[SDL_SCANCODE_9] = std::make_pair(4,0);
  keymap_[SDL_SCANCODE_0] = std::make_pair(4,3);
  /* keymap function keys */
  keymap_[SDL_SCANCODE_F1] = std::make_pair(0,4);
  keymap_[SDL_SCANCODE_F3] = std::make_pair(0,5);
  keymap_[SDL_SCANCODE_F5] = std::make_pair(0,6);
  keymap_[SDL_SCANCODE_F7] = std::make_pair(0,3);
  /* keymap: other */
  keymap_[SDL_SCANCODE_RETURN]    = std::make_pair(0,1);
  keymap_[SDL_SCANCODE_SPACE]     = std::make_pair(7,4);
  keymap_[SDL_SCANCODE_LSHIFT]    = std::make_pair(1,7);
  keymap_[SDL_SCANCODE_RSHIFT]    = std::make_pair(6,4);
  keymap_[SDL_SCANCODE_COMMA]     = std::make_pair(5,7);
  keymap_[SDL_SCANCODE_PERIOD]    = std::make_pair(5,4);
  keymap_[SDL_SCANCODE_SLASH]     = std::make_pair(6,7);
  keymap_[SDL_SCANCODE_SEMICOLON] = std::make_pair(6,2);
  keymap_[SDL_SCANCODE_EQUALS]    = std::make_pair(6,5);
  keymap_[SDL_SCANCODE_BACKSPACE] = std::make_pair(0,0);
  keymap_[SDL_SCANCODE_MINUS]     = std::make_pair(5,3);
  /* CRSR */
  keymap_[SDL_SCANCODE_UP]        = std::make_pair(0,7); // needs auto shift
  keymap_[SDL_SCANCODE_DOWN]      = std::make_pair(0,7);
  keymap_[SDL_SCANCODE_LEFT]      = std::make_pair(0,2); // needs auto shift
  keymap_[SDL_SCANCODE_RIGHT]     = std::make_pair(0,2);
  /* keymap: these are mapped to other keys */
  keymap_[SDL_SCANCODE_HOME]         = std::make_pair(6,3); // CLR
  keymap_[SDL_SCANCODE_BACKSLASH]    = std::make_pair(5,5); // :
  keymap_[SDL_SCANCODE_LEFTBRACKET]  = std::make_pair(5,0); // +
  keymap_[SDL_SCANCODE_RIGHTBRACKET] = std::make_pair(6,1); // *
  keymap_[SDL_SCANCODE_APOSTROPHE]   = std::make_pair(5,6); // @
  keymap_[SDL_SCANCODE_LGUI]         = std::make_pair(7,5); // (Win/Cmd) CBM / commodore key
  keymap_[SDL_SCANCODE_LCTRL]        = std::make_pair(7,2); // CTRL
  keymap_[SDL_SCANCODE_RCTRL]        = std::make_pair(7,2); // CTRL
  keymap_[SDL_SCANCODE_LALT]         = std::make_pair(7,2); // CTRL
  keymap_[SDL_SCANCODE_RALT]         = std::make_pair(7,2); // CTRL
  keymap_[SDL_SCANCODE_ESCAPE]       = std::make_pair(7,7); // RUN/STOP
  keymap_[SDL_SCANCODE_PAGEUP]       = std::make_pair(7,5); // RESTORE
  #endif /* DESKTOP */
}

/**
 * @brief init c64 color palette
 */
void IO::init_color_palette()
{
  #if DESKTOP
  #if SDL_ENABLED
  if(!nosdl) {
    color_palette[0]   = SDL_MapRGB(format_, 0x00, 0x00, 0x00);
    color_palette[1]   = SDL_MapRGB(format_, 0xff, 0xff, 0xff);
    color_palette[2]   = SDL_MapRGB(format_, 0xab, 0x31, 0x26);
    color_palette[3]   = SDL_MapRGB(format_, 0x66, 0xda, 0xff);
    color_palette[4]   = SDL_MapRGB(format_, 0xbb, 0x3f, 0xb8);
    color_palette[5]   = SDL_MapRGB(format_, 0x55, 0xce, 0x58);
    color_palette[6]   = SDL_MapRGB(format_, 0x1d, 0x0e, 0x97);
    color_palette[7]   = SDL_MapRGB(format_, 0xea, 0xf5, 0x7c);
    color_palette[8]   = SDL_MapRGB(format_, 0xb9, 0x74, 0x18);
    color_palette[9]   = SDL_MapRGB(format_, 0x78, 0x53, 0x00);
    color_palette[10]  = SDL_MapRGB(format_, 0xdd, 0x93, 0x87);
    color_palette[11]  = SDL_MapRGB(format_, 0x5b, 0x5b, 0x5b);
    color_palette[12]  = SDL_MapRGB(format_, 0x8b, 0x8b, 0x8b);
    color_palette[13]  = SDL_MapRGB(format_, 0xb0, 0xf4, 0xac);
    color_palette[14]  = SDL_MapRGB(format_, 0xaa, 0x9d, 0xef);
    color_palette[15]  = SDL_MapRGB(format_, 0xb8, 0xb8, 0xb8);
  }
  #endif
  #endif /* DESKTOP */
}

// emulation ///////////////////////////////////////////////////////////////////

bool IO::emulate()
{
  process_events();
  return retval_;
}

void IO::process_events()
{
  #if DESKTOP
  #if SDL_ENABLED
  if(!nosdl) {
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
      switch(event.type)
      {
      case SDL_KEYDOWN:
        handle_keydown(event.key.keysym.scancode);
        break;
      case SDL_KEYUP:
        handle_keyup(event.key.keysym.scancode);
        break;
      case SDL_QUIT:
        retval_ = false;
        break;
      }
    }
  }
  #endif /* SDL_ENABLED */
  /* process fake keystrokes if any */
  if(!key_event_queue_.empty() &&
     c64_->cpu_->cycles() > next_key_event_at_)
  {
    std::pair<kKeyEvent,SDL_Keycode> &ev = key_event_queue_.front();
    key_event_queue_.pop();
    switch(ev.first)
    {
    case kPress:
      handle_keydown(ev.second);
      break;
    case kRelease:
      handle_keyup(ev.second);
      break;
    }
    next_key_event_at_ = c64_->cpu_->cycles() + kWait;
  }
  #endif /* DESKTOP */
}

// keyboard handling ///////////////////////////////////////////////////////////

/**
 * @brief emulate keydown
 */
#if DESKTOP
void IO::handle_keydown(SDL_Keycode k)
{
  try
  {
    /* D("CODE: %2X\n",k); */

    uint8_t mask = ~(1 << keymap_.at(k).second); /* PRB */
    switch (k) { /* Handle special keypress combo's */
      uint8_t shiftmask;
      case SDL_SCANCODE_ESCAPE:
        runstop = true;
        break;
      case SDL_SCANCODE_CAPSLOCK: /* (Un)set shiftlock */
        shiftlock = !shiftlock;
        if (shiftlock) {
          /* shiftlock off */
          shiftmask = ~(1 << keymap_.at(SDL_SCANCODE_LSHIFT).second);
          keyboard_matrix_[keymap_.at(SDL_SCANCODE_LSHIFT).first] &= shiftmask;
        } else {
          /* shiftlock on */
          shiftmask = (1 << keymap_.at(SDL_SCANCODE_LSHIFT).second);
          keyboard_matrix_[keymap_.at(SDL_SCANCODE_LSHIFT).first] |= shiftmask;
        }
        break;
      case SDL_SCANCODE_UP: /* Let up be up */
      case SDL_SCANCODE_LEFT: /* And left be left */
          keyboard_matrix_[keymap_.at(SDL_SCANCODE_LSHIFT).first]
            &= ~(1 << keymap_.at(SDL_SCANCODE_LSHIFT).second);
        break;
      case SDL_SCANCODE_PAGEUP:
        if (runstop == true) {
          /* RUN/STOP RESTORE PRESSED */
          if(c64_->sid_->isSIDplaying()) {
            // enable kernel and basic rom in ram
            c64_->sid_->set_playing(false);
            c64_->mem_->write_byte(0x0001, 0x37);
          }
          c64_->sid_->reset();
          c64_->cart_->reset(); /* NOTE: Disables cart contents e.g. MC6850 */
          c64_->pla_->reset();
          this->reset(); /* IO */
          c64_->vic_->reset();
          c64_->cia1_->reset();
          c64_->cia2_->reset();
          c64_->cpu_->reset();
        }
        break;
      default:
        break;
    }
    keyboard_matrix_[keymap_.at(k).first] &= mask; /* PRA */

    c64_->mem_->kCIA1MemWr[0x00] |= (1<<keymap_.at(k).first);  /* PRA ~ ROW */
    c64_->mem_->kCIA1MemWr[0x01] |= (1<<keymap_.at(k).second); /* PRB ~ COL */
  }
  catch(const std::out_of_range){
    printf ("KEY %02X IS OUT OF RANGE\n",k);
  }
}
#endif /* DESKTOP */

/**
 * @brief emulate keyup
 */
#if DESKTOP
void IO::handle_keyup(SDL_Keycode k)
{
  try
  {
    switch (k) { /* Handle special keypress combo's */
      case SDL_SCANCODE_ESCAPE: runstop = false; break;
      case SDL_SCANCODE_UP: /* depress shift on release */
      case SDL_SCANCODE_LEFT:
        keyboard_matrix_[keymap_.at(SDL_SCANCODE_LSHIFT).first]
          |= (1 << keymap_.at(SDL_SCANCODE_LSHIFT).second);
        break;
      default: break;
    }

    uint8_t mask = (1 << keymap_.at(k).second);    /* PRB */
    keyboard_matrix_[keymap_.at(k).first] |= mask; /* PRA */
    c64_->mem_->kCIA1MemWr[0x01] &= ~(1<<keymap_.at(k).second); /* PRB ~ COL */
    c64_->mem_->kCIA1MemWr[0x00] &= ~(1<<keymap_.at(k).first);  /* PRA ~ ROW */
  }
  catch(const std::out_of_range){}
}
#endif /* DESKTOP */

/**
 * @brief fake press a key, monkeys love it
 *
 * Characters are added to a queue and processed within
 * the emulation loop.
 */
#if DESKTOP
void IO::type_character(char c)
{
  try
  {
    for(SDL_Keycode &k: charmap_.at(toupper(c))) {
      /* D("kPress %c %x\n",c,k); */
      key_event_queue_.push(std::make_pair(kPress,k));
    }
    for(SDL_Keycode &k: charmap_.at(toupper(c))) {
      /* D("kRelease %c %x\n",c,k); */
      key_event_queue_.push(std::make_pair(kRelease,k));
    }
  }
  catch(const std::out_of_range){}
}
#endif /* DESKTOP */

// screen handling /////////////////////////////////////////////////////////////

void IO::screen_draw_rect(int x, int y, int n, int color)
{
  for(int i=0; i < n ; i++)
  {
    screen_update_pixel(x+i,y,color);
  }
}

void IO::screen_draw_border(int y, int color)
{
  screen_draw_rect(0,y,cols_,color);
}

/**
 * @brief refresh screen
 * Called from vic->emulate();
 *
 * Upload the texture to the GPU
 */
void IO::screen_refresh()
{
  #if DESKTOP
  #if SDL_ENABLED
  if(!nosdl) {
    SDL_UpdateTexture(texture_, NULL, frame_, cols_ * sizeof(uint32_t));
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_,texture_, NULL, NULL);
    SDL_RenderPresent(renderer_);
  }
  #endif
  #endif /* DESKTOP */
  /* process events once every frame */
  process_events();
  /* perform vertical refresh sync */
  vsync();
}

/**
 * @brief vsync
 *
 * vsync() is called at the end of every frame, if we are ahead
 * of time compared to a real C64 (very likely) we sleep for a bit,
 * this way we avoid running at full speed allowing the host CPU to
 * take a little nap before getting back to work.
 *
 * This should also help with performance runing on slow computers,
 * uploading data to the GPU is a relatively slow operation, doing
 * more fps obviously has a performance impact.
 *
 * Also, and more importantly, by doing this we emulate the actual
 * speed of the C64 so visual effects do not look accelerated and
 * games become playable :)
 */
void IO::vsync()
{
  using namespace std::chrono;
  auto t = high_resolution_clock::now() - prev_frame_was_at_;
  duration<double> rr(Vic::kRefreshRate);
  /**
   * Microsoft's chrono is buggy and does not properly handle
   * doubles, we need to recast to milliseconds.
   */
  #if DESKTOP
  auto ttw = duration_cast<milliseconds>(rr - t);
  std::this_thread::sleep_for(ttw);
  #elif EMBEDDED
  /* TODO: Cast to nanoseconds and then calculation actual cpu cycles */
  /* Cast duration to microseconds */
  auto ttw = duration_cast<microseconds>(rr - t);
  /* unsigned short cast */
  using cast = std::chrono::duration<std::uint16_t>;
  /* Cast microseconds duration to unsigned short */
  std::uint16_t cycles = duration_cast< cast >(ttw).count();
  /* delay no cycles */
  cycled_delay_operation(cycles);
  #endif /* EMBEDDED */
  prev_frame_was_at_ = std::chrono::high_resolution_clock::now();
}
