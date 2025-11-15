find_package(Doxygen)

if(DOXYGEN_FOUND)

  message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

  # Set project files
  set(DOXY_API_INDEX_FILE index.html)
  set(DOXY_API_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXY_API_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile_ug)

  file(GLOB DOXY_API_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../zzalog_fe/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/include/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets/include/*.h
  )

  # Copy Doxyfile
  configure_file(${DOXY_API_IN} ${DOXY_API_OUT} @ONLY)
 
  # Command for generating dox from config file
  add_custom_target(api_html
      DEPENDS ${DOXY_API_FILES} ${DOXY_UG_OUT}
  )
  add_custom_command(
    TARGET api_html
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_API_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation"
    VERBATIM
  )
  

endif()
