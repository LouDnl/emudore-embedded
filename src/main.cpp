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


C64 *c64;
Loader *loader;
bool wget_download_finished = false;
bool file_loaded = false;
bool nosdl = false, isbinary = false,
     havecart = false,
     midi = false, bankswlog = false;


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
      loader->bin(f);
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
      if(!strcmp(argv[a], "-midi")) {midi = true;} /* BUG: Segmentation fault when used with loading a .bin file */
      if(!strcmp(argv[a], "-logbanksw")) {bankswlog = true;}

      if(!strcmp(argv[a], "-crt")) {loader->iscart = true;}
      if(!strcmp(argv[a], "-init")) {loader->init_addr = strtol(argv[a+1], NULL, 16);}
      if(!strcmp(argv[a], "-norun")) {loader->autorun = false;}
      if(!strcmp(argv[a], "-lowercase")) {loader->lowercase = true;}
      if(!strcmp(argv[a], "-loginstr")) {Cpu::loginstructions = true;}
      if(!strcmp(argv[a], "-logill")) {Cpu::logillegals = true;}
      if(!strcmp(argv[a], "-logmemrw")) {loader->memrwlog = true;}
      if(!strcmp(argv[a], "-logcia1rw")) {loader->cia1rwlog = true;}
      if(!strcmp(argv[a], "-logcia2rw")) {loader->cia2rwlog = true;}
      if(!strcmp(argv[a], "-logsidrw")) {loader->sidrwlog = true;}
      if(!strcmp(argv[a], "-logiorw")) {loader->iorwlog = true;}
      if(!strcmp(argv[a], "-logplarw")) {loader->plarwlog = true;}
      if(!strcmp(argv[a], "-logcartrw")) {loader->cartrwlog = true;}

      /* Check for file */
      if((strchr(argv[a], '.') != NULL)) {loader->filename = argv[a];}

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
  c64 = new C64(nosdl,isbinary,havecart,bankswlog,midi,loader->filename);

  if (argc != 1) {
    loader->C64ctr(c64); /* Init machine in Loader */
    /* check if asked load a program */
    if(file_loaded) {
      /* Process arguments and handle file loading callback */
      c64->callback(loader_cb);
    } else {
      /* Only process arguments */
      loader->handle_args();
    }
  }

  /* Continue machine startup */
  c64->start();
  return 0;
}
