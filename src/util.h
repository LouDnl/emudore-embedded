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

#ifndef EMUDORE_UTIL_H
#define EMUDORE_UTIL_H

#include <stdio.h>

#ifndef NDEBUG
#define D(...) fprintf (stderr,__VA_ARGS__)
#else
#define D(...) do {} while (0)
#endif

#define PBIT(v) D("%X 0b%d%d%d%d%d%d%d%d\n", \
  v, \
  (v&0x80)>>7, \
  (v&0x40)>>6, \
  (v&0x20)>>5, \
  (v&0x10)>>4, \
  (v&0x8)>>3, \
  (v&0x4)>>2, \
  (v&0x2)>>1, \
  (v&0x1) \
); \

#define ISSET_BIT(v,b)  ((v&(1<<b))!=0)

#if defined(__APPLE__)
# include <machine/endian.h>
# include <libkern/OSByteOrder.h>
#elif defined(_MSC_VER)
# include <stdlib.h>
#else
# include <byteswap.h>
#endif

#if !defined(htonll) && !defined(ntohll)
#if __BYTE_ORDER == __LITTLE_ENDIAN
# if defined(__APPLE__)
#  define htonll(x) OSSwapInt64(x)
#  define ntohll(x) OSSwapInt64(x)
# elif defined(_MSC_VER)
#  define htonll(x) _byteswap_uint64(x)
#  define ntohll(x) _byteswap_uint64(x)
# else
#  define htonll(x) bswap_64(x)
#  define ntohll(x) bswap_64(x)
# endif
#else
# define htonll(x) (x)
# define ntohll(x) (x)
#endif
#endif

#endif
