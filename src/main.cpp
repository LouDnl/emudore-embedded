/*
 * emudore, Commodore 64 emulator
 * Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
 * Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
 *
 * main.cpp
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

#include <iostream>
#include <string>
#include <algorithm>

#include <c64.h>
#include <loader.h>
#include <cstring>


C64 *c64;
Loader *loader;
bool wget_download_finished = false;
bool file_loaded = false;
bool nosdl = false, isbinary = false,
     havecart = false,
     acia = false, bankswlog = false,
     sidfile = false;

bool loader_cb()
{
  if(!loader->emulate())
    c64->callback(nullptr);
  return true;
}

bool load_file(const char *file) /* TODO: ADD ARGUMENTS FOR SIDFILE PLAY */
{
  // const char *file = argv[1];
  std::string f(file);
  size_t ext_i = f.find_last_of(".");
  if(ext_i != std::string::npos)
  {
    std::string ext(f.substr(ext_i+1));
    std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
    if(ext == "bas"){
      loader->bas(f);
    }
    else if(ext == "bin"){
      if (loader->iscart) {
        havecart = loader->crt();
        return false;
      } else {
        loader->bin(f);
      }
    }
    else if(ext == "prg"){
      loader->prg(f);
    }
    else if(ext == "d64"){
      loader->d64(f);
    }
    else if(ext == "crt"){
      havecart = loader->crt();
      return false; /* Circumvent callback for cart */
    }
    else if(ext == "sid"){
      loader->sid(f);
      sidfile = true;
    }
  }
  return true;
}

void checkargs(int argc, char **argv)
{
  if(argc>=1) {
    for(int a = 1; a < argc; a++) {
      /* log on create */
      if(!strcmp(argv[a], "-cli")) {nosdl = true;}
      if(!strcmp(argv[a], "-bin")) {isbinary = true;}
      if(!strcmp(argv[a], "-midi")) {acia = true;} /* BUG: Segmentation fault when used with loading a .bin file */

      if(!strcmp(argv[a], "-s")) {
        loader->subtune = (strtol(argv[a+1], NULL, 10) - 1);
        printf("SUBTUNE: %d\n",loader->subtune);
      }

      if(!strcmp(argv[a], "-run")) {loader->basic_run = true;} /* Default false */
      if(!strcmp(argv[a], "-norun")) {loader->autorun = false;} /* Default true */
      if(!strcmp(argv[a], "-crt")) {loader->iscart = true;} /*  */
      if(!strcmp(argv[a], "-init")) {
        loader->init_addr = strtol(argv[a+1], NULL, 16); /* HEX */
        printf("INIT: %d\n",loader->init_addr);
      }
      if(!strcmp(argv[a], "-normal")) {loader->normal_start = true;}
      if(!strcmp(argv[a], "-lowercase")) {loader->lowercase = true;}

      if(!strcmp(argv[a], "-logbanksw")) {bankswlog = true;} /* Bankswitching at boot */
      if(!strcmp(argv[a], "-logcpu")) {Cpu::loginstructions = true;}
      if(!strcmp(argv[a], "-loginstr")) {loader->instrlog = true;}
      if(!strcmp(argv[a], "-logmemrw")) {loader->memrwlog = true;}
      if(!strcmp(argv[a], "-logcia1rw")) {loader->cia1rwlog = true;}
      if(!strcmp(argv[a], "-logcia2rw")) {loader->cia2rwlog = true;}
      if(!strcmp(argv[a], "-logsidrw")) {loader->sidrwlog = true;}
      if(!strcmp(argv[a], "-logiorw")) {loader->iorwlog = true;}
      if(!strcmp(argv[a], "-logvicrw")) {loader->vicrwlog = true;}
      if(!strcmp(argv[a], "-logplarw")) {loader->plarwlog = true;}
      if(!strcmp(argv[a], "-logcartrw")) {loader->cartrwlog = true;}
      if(!strcmp(argv[a], "-logtimings")) {C64::log_timings = true;}

      /* Check for file */
      if((strchr(argv[a], '.') != NULL)) {loader->filename = argv[a];}

      /* Help */
      if(!strcmp(argv[a], "-h")) {
        printf("***** EMUDORE HELP *****\n");
        printf("\n");

        printf("-cli           : start without SDL and screen\n");
        printf("-crt           : use if cart file is .bin (binary)\n");
        printf("-bin           : unused\n");
        printf("-midi          : hack for emulating mc68b60 acia on cart\n");

        printf("\n");
        printf("-run           : start PRG's from basic with RUN (default: false)\n");
        printf("-norun         : do not autostart PRG's (default: true)\n");
        printf("-normal        : run normal emulation for PSID tune play\n");
        printf("                 othwerwise only emulates CPU and CIA1\n");
        printf("-s #           : set SID subtune to play\n");

        printf("\n");
        printf("-init ####     : force init address for PRG/BIN in hex\n");
        printf("                 hex address without 0x e.g. 1000\n");
        printf("-lowercase     : start basic in lowercase\n");

        printf("\n");
        printf("-logtimings    : log timings between emulation cycles\n");
        printf("-logcpu        : log cpu instructions from boot\n");
        printf("-loginstr      : log cpu instructions after loader\n");
        printf("-logbanksw     : log runtime bank switches\n");
        printf("-logmemrw      : log mem read/writes\n");
        printf("-logcia1rw     : log cia1 read/writes\n");
        printf("-logcia2rw     : log cia2 read/writes\n");
        printf("-logsidrw      : log sid read/writes\n");
        printf("-logiorw       : log io read/writes\n");
        printf("-logvicrw      : log vic read/writes\n");
        printf("-logplarw      : log pla read/writes (unused)\n");
        printf("-logcartrw     : log cartridges read/writes\n");
        exit(1);
      }

    }
  }
}

int main(int argc, char **argv)
{
  loader = new Loader();
  if(argc != 1) {
    checkargs(argc, argv);
    if (loader->filename != NULL) {
      file_loaded = load_file(loader->filename);
    }
  }

  /* Init Machine start */
  if (file_loaded) {
    c64 = new C64(nosdl,isbinary,havecart,bankswlog,acia,std::string {""});
  } else {
    c64 = new C64(nosdl,isbinary,havecart,bankswlog,acia,loader->filename);
  }

  if (argc != 1) {
    loader->C64ctr(c64); /* Init machine in Loader */
    /* check if asked load a program */
    if(file_loaded) {
      if (!sidfile) {
        /* Process arguments and handle file loading callback */
        c64->callback(loader_cb);
      } else if (loader->isrsid()) {
      //   c64->emulate(); /* run single emulation cycle */
        loader->emulate();
      } else {
        c64->callback(loader_cb);
      }
    } else {
      /* Only process arguments */
      loader->handle_args();
    }
  }

  /* Continue machine startup */
  if (!sidfile) {
    c64->start();
  } else {
    bool em_cpu, em_cia1, em_cia2, em_vic, em_io, em_cart;
    em_cpu  = true;  /* always true */
    em_cia1 = true;  /* always true, breaks any type of play otherwise */
    em_cia2 = (C64::is_rsid ? true : false);
    em_vic  = (C64::is_rsid ? true : nosdl ? false : true); /* always true for RSID DESKTOP! */
    em_io   = (nosdl ? false : true); /* based on -cli */
    em_cart = false; /* always false */
    printf("START: %d %d %d %d %d %d\n",em_cpu, em_cia1, em_cia2, em_vic, em_io, em_cart);
    if (!loader->isrsid()) { /* PSID */
      /* NOTICE: ANY LOGGING WILL SLOW PLAY DRAMATICALLY!! */
      while (c64->is_looping()) {
        // c64->emulate();  // BUG: HANGS IF YOU WANT TO CLOSE IT
        if (loader->normal_start) {
          c64->emulate();
        } else {
          // c64->emulate_specified(); // BUG: BREAKS SIDPLAY IF CALLBACK IS CALLED TOO SOON
          c64->emulate_specified(
            em_cpu,   /* CPU */
            em_cia1,  /* CIA1 */
            em_cia2,  /* CIA2 */
            em_vic,   /* VIC */
            em_io,    /* IO */
            em_cart); /* CART */
        }
      }
    } else { /* RSID */
      c64->start();
    }
  }
  delete c64;
  return 0;
}
