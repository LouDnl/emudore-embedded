/*
 * Function timer for emudore
 * Copyright (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * timer.cpp
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

#ifndef _C64_TIMER_
#define _C64_TIMER_

#include <cstdio>
#include <chrono>
using namespace std::chrono;

#if DESKTOP
#include <thread>
static int run_thread;
static pthread_mutex_t timer_mutex;
#endif

static int data_available;
static uint64_t cyc,del,delc,dbg,cb,cart,cpu,cia1,cia2,vic,io;

class BenchmarkTimer
{
  public:
    #if DESKTOP
    static void *_Timer_Thread(void *context)
    { /* Required for supplying private function to pthread_create */
      return ((BenchmarkTimer *)context)->StartThread();
    }
    #endif

  private:

  /* std::chrono:: in nanoseconds */
  high_resolution_clock::time_point _measure_init;
  high_resolution_clock::time_point _measure_start;
  high_resolution_clock::time_point _measure_end;

  #if DESKTOP
  pthread_t threadid;

  void *StartThread(void)
  {
    pthread_setname_np(pthread_self(), "Timer Thread");
    pthread_detach(pthread_self());
    pthread_mutex_lock(&timer_mutex);

    while (run_thread == 1) {
      if (data_available == 1) {
        printf("[C]%2lu [D]%5lu(%2lu) [TOT]%5lu [CART]%5lu [CPU]%5lu [CIA1]%5lu [CIA2]%5lu [VIC]%5lu [IO]%5lu [DBG]%5lu [CB]%5lu\n",
          cyc,del,delc,(cart+cpu+cia1+cia2+vic+io+dbg+cb),cart,cpu,cia1,cia2,vic,io,dbg,cb
        );
        data_available = 0;
      } else {
        asm("nop");
      }
    }

    pthread_mutex_unlock(&timer_mutex);
    pthread_exit(NULL);
    return NULL;
  };
  #endif

  public:
    BenchmarkTimer() :
      /* Pre set/ pre fill the timers */
      _measure_init(high_resolution_clock::now()),
      _measure_start(_measure_init),
      _measure_end(_measure_init)
    {
      data_available = 0;
      #if DESKTOP
      run_thread = 1;
      int error = pthread_create(&this->threadid, NULL, &this->_Timer_Thread, NULL);
      if (error != 0) {
        fprintf(stderr, "[TIMER] Thread can't be created :[%s]\n", strerror(error));
      }
      #endif
    };
    ~BenchmarkTimer()
    {
      data_available = 0;
      #if DESKTOP
      run_thread = 0;
      #endif
    };


    void MeasurementStart(void)
    {
      _measure_start = high_resolution_clock::now();
    };

    void MeasurementEnd()
    {
      _measure_end = high_resolution_clock::now();
    };

    uint64_t _MeasurementResult(void)
    {
      // auto ttw = duration_cast<nanoseconds>(_measure_end - _measure_start);
      auto ttw = (_measure_end - _measure_start);
      std::uint64_t diff = ttw.count();
      // printf("%4lld %lld %lld\n",ttw,_measure_start,_measure_end);
      return diff;
    };

    uint64_t MeasurementResult(void)
    {
      auto ttw = duration_cast<nanoseconds>(_measure_end - _measure_start);
      std::uint64_t diff = ttw.count();
      return diff;
    };

    std::chrono::duration<long int, std::ratio<1, 1000000000>> _TimeDiff(void)
    {
      const std::chrono::duration<long int, std::ratio<1, 1000000000>> diff = (_measure_end - _measure_start);
      return diff;
    };

    uint64_t TimeDiff(void)
    {
      // const std::chrono::duration<long int, std::ratio<1, 1000000000>> diff = (_measure_end - _measure_start);
      auto ttw = duration_cast<nanoseconds>(_measure_end - _measure_start);
      // using cast = std::chrono::duration<std::uint16_t>;
      // std::uint16_t r = duration_cast< cast >(diff).count();
      std::uint64_t diff = ttw.count();
      return diff;
    };

    std::chrono::duration<long int, std::ratio<1, 1000000000>> TimeSinceStart(void)
    {
      high_resolution_clock::time_point _now = high_resolution_clock::now();
      const std::chrono::duration<long int, std::ratio<1, 1000000000>> diff = (_now - _measure_init);
      return diff;
    };

    void receive_data(unsigned int _c,uint64_t _del,uint64_t _del_c,uint64_t _dbg,uint64_t _cb,uint64_t _cart,uint64_t _cpu,uint64_t _cia1,uint64_t _cia2,uint64_t _vic,uint64_t _io)
    {
      static unsigned int _prev_cycles;
      cyc  =  (_c - _prev_cycles);
      del  = _del;
      delc = _del_c;
      dbg  = _dbg;
      cb   = _cb;
      cart = _cart;
      cpu  = _cpu;
      cia1 = _cia1;
      cia2 = _cia2;
      vic  = _vic;
      io   = _io;
      _prev_cycles = _c;
      data_available = 1;
      #if EMBEDDED
      printf("[C]%2lu [D]%5lu(%2lu) [TOT]%5lu [CART]%5lu [CPU]%5lu [CIA1]%5lu [CIA2]%5lu [VIC]%5lu [IO]%5lu [DBG]%5lu [CB]%5lu\n",
        cyc,del,delc,(cart+cpu+cia1+cia2+vic+io+dbg+cb),cart,cpu,cia1,cia2,vic,io,dbg,cb
      );
      #endif
      return;
    }

};


#endif /* _C64_TIMER_ */
