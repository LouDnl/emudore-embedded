//============================================================================
// Description : SID file parser
// Filename    : sidfile.h
// Source repo : https://github.com/LouDnl/SidBerry
// Authors     : Gianluca Ghettini, LouD
// Last update : 2025
// License     : GPL2 https://github.com/LouDnl/SidBerry/blob/master/LICENSE
//============================================================================

#ifndef _SIDFILE_H_
#define _SIDFILE_H_

#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>
#include <stdio.h>
using namespace std;

#define PSID_MIN_HEADER_LENGTH 118 // Version 1
#define PSID_MAX_HEADER_LENGTH 130 // Version 2 (124), 3 & 4, 78 (130)

// Offsets of fields in header (all fields big-endian)
enum
{
    SIDFILE_PSID_ID = 0x0,          // 'PSID'
    SIDFILE_PSID_VERSION_H = 4,   // 1, 2, 3 or 4
    SIDFILE_PSID_VERSION_L = 5,   // 1, 2, 3 or 4
    SIDFILE_PSID_LENGTH_H = 6,    // Header length
    SIDFILE_PSID_LENGTH_L = 7,    // Header length
    SIDFILE_PSID_START_H = 8,     // C64 load address
    SIDFILE_PSID_START_L = 9,     // C64 load address
    SIDFILE_PSID_INIT_H = 10,     // C64 init routine address
    SIDFILE_PSID_INIT_L = 11,     // C64 init routine address
    SIDFILE_PSID_MAIN_H = 12,     // C64 replay routine address
    SIDFILE_PSID_MAIN_L = 13,     // C64 replay routine address
    SIDFILE_PSID_NUMBER_H = 14,   // Number of subsongs
    SIDFILE_PSID_NUMBER_L = 15,   // Number of subsongs
    SIDFILE_PSID_DEFSONG_H = 16,  // Main subsong number
    SIDFILE_PSID_DEFSONG_L = 17,  // Main subsong number
    SIDFILE_PSID_SPEED = 18,      // Speed flags (1 bit/song)
    SIDFILE_PSID_NAME = 22,       // Module name (ISO Latin1 character set)
    SIDFILE_PSID_AUTHOR = 54,     // Author name (dto.)
    SIDFILE_PSID_COPYRIGHT = 86,  // Release year and Copyright info (dto.)

    SIDFILE_PSID_FLAGS_H = 118,    // WORD Flags (only in version 2, 3 & 4 header)
    SIDFILE_PSID_FLAGS_L = 119,    // WORD Flags (only in version 2, 3 & 4 header)
    SIDFILE_PSID_STARTPAGE  = 120,  // BYTE startPage (relocStartPage)
    SIDFILE_PSID_PAGELENGTH = 121, // BYTE pageLength (relocPages)
    SIDFILE_PSID_SECONDSID     = 0x7A,  // 122 BYTE secondSIDAddress $42..$FE
    SIDFILE_PSID_THIRDSID      = 0x7B,  // 123 BYTE thirdSIDAddress $42..$FE

    SIDFILEPLUS_PSID_SECONDSID = 0x7A,  // 122 BYTE secondSIDAddress $42..$FE
    SIDFILEPLUS_PSID_THIRDSID  = 0x7C,  // 124 BYTE thirdSIDAddress $42..$FE
    SIDFILEPLUS_PSID_FOURTHSID = 0x7E,  // 126 BYTE fourthSIDAddress $42..$FE
};

enum
{
    SIDFILE_OK,
    SIDFILE_ERROR_FILENOTFOUND,
    SIDFILE_ERROR_MALFORMED
};

enum
{
    SIDFILE_SPEED_50HZ,
    SIDFILE_SPEED_60HZ
};

/* Clock cycles per second
  * Clock speed: 0.985 MHz (PAL) or 1.023 MHz (NTSC)
  *
  * For some reason both 1022727 and 1022730 are
  * mentioned as NTSC clock cycles per second
  * Going for the rates specified by Vice it should
  * be 1022730, except in the link about raster time
  * on c64 wiki it's 1022727.
  * I chose to implement both, let's see how this
  * works out
  *
  * https://sourceforge.net/p/vice-emu/code/HEAD/tree/trunk/vice/src/c64/c64.h
  */

/* Clock cycles per second */
enum clock_speeds
{
  DEFAULT = 1000000,  /* 1 MHz     = 1 us */
  PAL     = 985248,   /* 0.985 MHz = 1.014973 us */
  NTSC    = 1022727,  /* 1.023 MHz = 0.977778 us */
  DREAN   = 1023440,  /* 1.023 MHz = 0.977097 us */
  NTSC2   = 1022730,  /* 1.023 MHz = 0.977778 us */
};
/* Refreshrates (cycles) in microseconds */
enum refresh_rates
{
  HZ_DEFAULT = 20000,  /* 50Hz ~ 20000 == 20 us */
  HZ_EU      = 19950,  /* 50Hz ~ 20000 == 20 us    / 50.125Hz ~ 19.950124688279 exact */
  HZ_US      = 16715,  /* 60Hz ~ 16667 == 16.67 us / 59.826Hz ~ 16.715140574332 exact */
};
/* Rasterrates (cycles) in microseconds
  * Source: https://www.c64-wiki.com/wiki/raster_time
  *
  * PAL: 1 horizontal raster line takes 63 cycles
  * or 504 pixels including side borders
  * whole screen consists of 312 horizontal lines
  * for a frame including upper and lower borders
  * 63 * 312 CPU cycles is 19656 for a complete
  * frame update @ 985248 Hertz
  * 985248 / 19656 = approx 50.12 Hz frame rate
  *
  * NTSC: 1 horizontal raster line takes 65 cycles
  * whole screen consists of 263 rasters per frame
  * 65 * 263 CPU cycles is 17096 for a complete
  * frame update @ 985248 Hertz
  * 1022727 / 17096 = approx 59.83 Hz frame rate
  *
  */
enum raster_rates
{
  R_DEFAULT = 20000,  /* 20us  ~ fallback */
  R_EU      = 19656,  /* PAL:  63 cycles * 312 lines = 19656 cycles per frame update @  985248 Hz = 50.12 Hz frame rate */
  R_US      = 17096,  /* NTSC: 65 cycles * 263 lines = 17096 cycles per frame update @ 1022727 Hz = 59.83 Hz Hz frame rate */
};

enum scan_lines
{
  C64_PAL_SCANLINES = 312,
  C64_NTSC_SCANLINES = 263
};

enum scanline_cycles
{
  C64_PAL_SCANLINE_CYCLES = 63,
  C64_NTSC_SCANLINE_CYCLES = 65
};
static const enum clock_speeds clockSpeed[]   = { DEFAULT, PAL, NTSC, DREAN, NTSC2 };
static const enum refresh_rates refreshRate[] = { HZ_DEFAULT, HZ_EU, HZ_US, HZ_US, HZ_US };
static const enum raster_rates rasterRate[]   = { R_DEFAULT, R_EU, R_US, R_US, R_US };
static long cycles_per_sec    = DEFAULT;     /* default @ 1000000 */
static long cycles_per_frame  = HZ_DEFAULT;  /* default @ 20000 */
static long cycles_per_raster = R_DEFAULT;   /* default @ 20000 */

// static const enum clock_speeds clockSpeed[] = {UNKNOWN, PAL, NTSC, BOTH, DREAN};
// static const enum refresh_rates refreshRate[] = {DEFAULT, EU, US, GLOBAL};
static const enum scan_lines scanLines[] = {C64_PAL_SCANLINES, C64_PAL_SCANLINES, C64_NTSC_SCANLINES, C64_NTSC_SCANLINES};
static const enum scanline_cycles scanlinesCycles[] = {C64_PAL_SCANLINE_CYCLES, C64_PAL_SCANLINE_CYCLES, C64_NTSC_SCANLINE_CYCLES, C64_NTSC_SCANLINE_CYCLES};

class SidFile
{
private:
    std::string moduleName;
    std::string authorName;
    std::string copyrightInfo;
    std::string sidType;
    int numOfSongs;
    int firstSong;
    uint16_t sidVersion;
    uint16_t dataOffset;
    uint16_t initAddr;
    uint16_t playAddr;
    uint64_t loadAddr;
    uint32_t speedFlags;
    uint8_t dataBuffer[0x10000];
    uint16_t dataLength;
    uint16_t sidFlags;
    uint16_t clockSpeed;
    uint16_t chipType;
    uint16_t chipType2;
    uint16_t chipType3;
    uint8_t startPage;
    uint8_t pageLength;
    uint8_t secondSID;
    uint8_t thirdSID;
    uint8_t fourthSID;

    uint8_t Read8(const uint8_t *p, int offset);
    uint16_t Read16(const uint8_t *p, int offset);
    uint32_t Read32(const uint8_t *p, int offset);
    bool IsPSIDHeader(const uint8_t *p);
    unsigned char reverse(unsigned char b);

public:
    SidFile();
    int Parse(std::string file);
    std::string GetSidType();
    std::string GetModuleName();
    std::string GetAuthorName();
    std::string GetCopyrightInfo();
    int GetSongSpeed(int songNum);
    int GetNumOfSongs();
    int GetFirstSong();
    uint8_t *GetDataPtr();
    uint16_t GetDataLength();
    uint16_t GetSidVersion();
    uint16_t GetSidFlags();
    uint16_t GetChipType(int n);
    uint16_t GetSIDaddr(int n);
    uint16_t GetClockSpeed();
    uint16_t GetDataOffset();
    uint16_t GetLoadAddress();
    uint16_t GetInitAddress();
    uint16_t GetPlayAddress();
};

#endif /* _SIDFILE_H_ */
