##
# emudore, Commodore 64 emulator
# Copyright (c) 2016, Mario Ballano <mballano@gmail.com>
# Changes and additions (c) 2025, LouD <emudore@mail.loudai.nl>
#
# CMakeLists.txt
#
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

### Cmake minimum version
cmake_minimum_required(VERSION 3.17)

### Cmake compiler standard versions
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

### Project magic sprinkles
set(PROJECT_NAME adorable)

# enable or disable radare2 debugging support
set(DBG_SUPPORT 1)
# enabled/disable SDL window (SET TO 0 FOR EMBEDDED!)
set(SDL_ENABLED 1)
# DESKTOP and EMBEDDED are mutually exclusive
set(DESKTOP 1)
set(EMBEDDED 0) # Requires Pico SDK components!
# enable or disable hardware sid driver
set(USBSID_DRIVER 1)

set(DBG -g3)
set(OPT -O0)
set(WARN PRIVATE
  # -Wall
  # -Wunused
  # -Werror
  -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
  -Wno-unused-function # extra check to verify no unused lingering code is present
  -Wno-maybe-uninitialized
)

### Compiler flags and options
set(COMPILE_OPTS PRIVATE
  ${DBG}
  ${OPT}
  -fno-omit-frame-pointer
  -fverbose-asm
  -save-temps
  -DDEBUGGER_SUPPORT=${DBG_SUPPORT}
  -DDESKTOP=${DESKTOP}
  -DSDL_ENABLED=${SDL_ENABLED}
  -DUSBSID_DRIVER=${USBSID_DRIVER}
  -DEMBEDDED=${EMBEDDED}
)

# Run the project command
project(${PROJECT_NAME} C CXX ASM)

if(DESKTOP EQUAL 1)
# the `pkg_check_modules` function is created with this call
find_package(PkgConfig REQUIRED)
endif()

if(USBSID_DRIVER EQUAL 1)
# these calls create special `PkgConfig::<MODULE>` variables
pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)
pkg_check_modules(udev REQUIRED IMPORTED_TARGET libudev)

### Libraries to link
set(TARGET_LL
  PkgConfig::libusb
  PkgConfig::udev
)
endif()

### Source files to compile
set(SOURCEFILES
  ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/sidfile.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cpu.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/memory.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/c64.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cia1.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cia2.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/io.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/vic.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/sidadapter.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/pla.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cart.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cart/MC68B50.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/debugger.cpp
)
### Header folders to include
set(TARGET_INCLUDE_DIRS PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/src
  ${CMAKE_CURRENT_LIST_DIR}/src/cart
  /usr/local/lib
  /usr/local/include
  /usr/lib
  /usr/include
)

if(USBSID_DRIVER EQUAL 1)
  set(SOURCEFILES
    ${SOURCEFILES}
    ${CMAKE_CURRENT_LIST_DIR}/lib/USBSID-Pico-driver/USBSID.cpp
  )
  set(TARGET_INCLUDE_DIRS
    ${TARGET_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}/lib/USBSID-Pico-driver
  )
endif()

if(SDL_ENABLED EQUAL 1)
  pkg_check_modules(sdl REQUIRED IMPORTED_TARGET sdl2)
  set(TARGET_LL
    ${TARGET_LL}
    PkgConfig::sdl
    )
else()
  set(TARGET_INCLUDE_DIRS
    ${TARGET_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}/lib/SDL)
endif()

### Compile time
add_executable(${PROJECT_NAME} ${SOURCEFILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNIX_COMPILE)

target_include_directories(${PROJECT_NAME} ${TARGET_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${TARGET_LL})
target_sources(${PROJECT_NAME} PUBLIC ${SOURCEFILES})
target_compile_options(${PROJECT_NAME} ${COMPILE_OPTS})
