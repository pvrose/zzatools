cmake_minimum_required(VERSION 3.16)

project(zzalog)

set(VENDOR "GM3ZZA")
set(PROGRAMID "ZZALOG")

set(TARGET zzalog)

# set document directory
if(MSVC)
  set(CMAKE_INSTALL_RPATH C:/ProgramData/${VENDOR}/${PROGRAMID})
else()
  set(CMAKE_INSTALL_RPATH /etc/${VENDOR}/${PROGRAMID})
endif()


add_subdirectory(common)
add_subdirectory(pugixml)
add_subdirectory(widgets)
add_subdirectory(zzalog_fe)

set(DATAFILES
  ${CMAKE_CURRENT_SOURCE_DIR}/reference/all.json
  ${CMAKE_CURRENT_SOURCE_DIR}/reference/band_plan.json
  ${CMAKE_CURRENT_SOURCE_DIR}/reference/cty.csv
  ${CMAKE_CURRENT_SOURCE_DIR}/reference/cty.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/reference/Prefix.lst
  ${CMAKE_CURRENT_SOURCE_DIR}/reference/intl_chars.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/reference/google-maps.png
  ${CMAKE_CURRENT_SOURCE_DIR}/reference/pdf.png
  ${CMAKE_CURRENT_SOURCE_DIR}/reference/qrz_1.jpg
  ${CMAKE_CURRENT_SOURCE_DIR}/reference/rose.png
)

# Add dummy target
add_custom_target(zzalog
    DEPENDS zzalog_fe
)

# copy reference data
if(MSVC)
  file(COPY ${DATAFILES}
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
else()
  file(COPY ${DATAFILES}
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

# generate docus
option(ZZALOG_USERGUIDE "Generate Userguide" ON)
if (MSVC)
  option(ZZALOG_USERGUIDE_PDF "Generate PDF" OFF)
else()
  option(ZZALOG_USERGUIDE_PDF "Generate PDF" ON)
endif()
if(ZZALOG_USERGUIDE) 
  add_subdirectory(userguide)
  add_dependencies(${TARGET} ug_html)
  if (ZZALOG_USERGUIDE_PDF)
    add_dependencies(${TARGET} pdf)
  endif()
endif()
option(ZZALOG_API_GUIDE "Generate API guide" ON)
if(ZZALOG_API_GUIDE) 
  add_subdirectory(api)
  add_dependencies(${TARGET} api_html)
endif()

# install zzalog_fe
install(TARGETS zzalog_fe)
# install reference data
install(FILES ${DATAFILES}
    DESTINATION ${CMAKE_INSTALL_RPATH})
# install userguide files
if(ZZALOG_USERGUIDE) 
  install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/userguide/html
    DESTINATION ${CMAKE_INSTALL_RPATH}/userguide
  )
  if (ZZALOG_USERGUIDE_PDF)
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/userguide/${PROGRAMID}.pdf
      DESTINATION ${CMAKE_INSTALL_RPATH}/userguide
    )
  endif()
endif()
# install API documentation files
if(ZZALOG_API_GUIDE) 
  install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/api/html
    DESTINATION ${CMAKE_INSTALL_RPATH}/api
  )
endif()


