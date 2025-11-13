cmake_minimum_required(VERSION 3.16)

project(pugixml)

set (TARGET pugixml)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VENDOR "GM3ZZA")
set(PROGRAMID "ZZALOG")

set(CPPFILES 
  pugixml.cpp
)

set(HPPFILES
  pugiconfig.hpp
  pugixml.hpp
)

# ser compiler options
if (MSVC)
else()
  add_compile_options(-g)
endif()


set(CMAKE_BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR})
add_library(${TARGET})
target_include_directories(${TARGET} PRIVATE 
  ./)
# Add QBS_logo.h to ensure it gets generated
target_sources(${TARGET} 
  PRIVATE 
    ${CPPFILES}
  PUBLIC 
    FILE_SET common_headers
    TYPE HEADERS
    FILES ${HPPFILES}
)

install(TARGETS ${TARGET})

