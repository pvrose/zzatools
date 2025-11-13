cmake_minimum_required(VERSION 3.16)

project(zzalog_common)

set (TARGET zzalog_common)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VENDOR "GM3ZZA")
set(PROGRAMID "ZZALOG")

set(CPPFILES 
  src/utils.cpp
)

set(HPPFILES
  include/callback.h
  include/drawing.h
  include/utils.h
)

# set document directory
if(MSVC)
  set(CMAKE_INSTALL_RPATH C:/ProgramData/${VENDOR}/${PROGRAMID})
else()
  set(CMAKE_INSTALL_RPATH /etc/${VENDOR}/${PROGRAMID})
endif()

# ser compiler options
if (MSVC)
else()
  add_compile_options(-g)
endif()

#FLTK
if (MSVC)
set(FLTK_DIR "C:/Users/pvros/source/repos/fltk/build"
    CACHE FILEPATH "FLTK installation or build directory")
endif()
find_package(FLTK 1.5.0 CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ../)

set(CMAKE_BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR})
add_library(${TARGET})
target_include_directories(${TARGET} PRIVATE 
  ./include
  ${FLTK_INCLUDE_DIR}
)
# Add QBS_logo.h to ensure it gets generated
target_sources(${TARGET} 
  PRIVATE 
    ${CPPFILES}
  PUBLIC 
    FILE_SET common_headers
    TYPE HEADERS
    BASE_DIRS ./include
    FILES ${HPPFILES}
)

install(TARGETS ${TARGET})

