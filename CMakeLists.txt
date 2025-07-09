### Cmake minimum version
cmake_minimum_required(VERSION 3.17)

### Cmake compiler standard versions
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

### Project magic sprinkles
set(PROJECT_NAME adorable)

set(ILLEGAL_LOGGING 0)
set(DBG_SUPPORT -DDEBUGGER_SUPPORT)

### Compiler flags and options
set(COMPILE_OPTS PRIVATE
  -g3
  -Og
  # -Wall
  # -Wunused
  # -Werror
  -fno-omit-frame-pointer
  -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
  -Wno-unused-function # extra check to verify no unused lingering code is present
  -Wno-maybe-uninitialized
  -save-temps
  -fverbose-asm
  -DLOG_ILLEGALS=${ILLEGAL_LOGGING}
  ${DBG_SUPPORT}
)

# Run the project command
project(${PROJECT_NAME} C CXX ASM)

# the `pkg_check_modules` function is created with this call
find_package(PkgConfig REQUIRED)

# these calls create special `PkgConfig::<MODULE>` variables

### Libraries to link
pkg_check_modules(sdl REQUIRED IMPORTED_TARGET sdl2)
set(TARGET_LL
  PkgConfig::sdl
)

### Source files to compile
set(SOURCEFILES
  ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/loader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cpu.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/sid.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/memory.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/c64.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cia2.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/io.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/vic.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/cia1.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/debugger.cpp
)
### Header folders to include
set(TARGET_INCLUDE_DIRS PRIVATE
  .
  src
  ${CMAKE_CURRENT_LIST_DIR}/src
  /usr/local/lib
  /usr/local/include
  /usr/lib
  /usr/include
)

### Compile time
add_executable(${PROJECT_NAME} ${SOURCEFILES})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNIX_COMPILE)

target_include_directories(${PROJECT_NAME} ${TARGET_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${TARGET_LL})
target_sources(${PROJECT_NAME} PUBLIC ${SOURCEFILES})
target_compile_options(${PROJECT_NAME} ${COMPILE_OPTS})
