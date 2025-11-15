find_package(Doxygen)

if(DOXYGEN_FOUND)

  message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

  # Set project files
  set(DOXY_UG_INDEX_FILE index.html)
  set(DOXY_UG_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
  set(DOXY_UG_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile_ug)

  set(DOXY_UG_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/adding.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/banner.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/basic1.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/basic2.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/change_dialog.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cli.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/condx_view.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/contest_dialog.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/contest_scorer.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cty_dialog.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dxcc_view.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fields_dialog.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/index.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/installation.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/intl_dialog.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/local_data.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log_table.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main_window.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/menu.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/new_app.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/new_rig.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/operating.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qsl_editor.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qsls.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_apps.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_bands.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_buttons.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_clocks.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_details.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_dxcc.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_entry.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_log_info.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_manager.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_net_entry.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_operation.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_qsl.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_qsl_vwr.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_query.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_rig.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qso_wx.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/quick_start.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/reference.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/release_notes.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/report_tree.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/search_dialog.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/searching.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stn_dialog.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/toolbar.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/user_dialog.dox
    ${CMAKE_CURRENT_SOURCE_DIR}/src/web_dialog.dox
  )

  # Copy Doxyfile
  configure_file(${DOXY_UG_IN} ${DOXY_UG_OUT} @ONLY)
 
  # Command for generating dox from config file
  message("Adding command to generate: ${CMAKE_CURRENT_BINARY_DIR}/html/index.html")
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
    DEPENDS ${DOXY_UG_FILES} ${DOXY_UG_OUT}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_UG_OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating userguide"
    VERBATIM
  )
  add_custom_target(
    ug_html ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/html/index.html
  )

  # Command for generating PDF from userguide latex files
  if(MSVC)
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
        DEPENDS ug_html
        COMMAND miktex-texworks.exe -pdf ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.tex
        COMMENT "Generating PDF Userguide"
        VERBATIM
      )
  else()
     add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
        DEPENDS ug_html
        COMMAND make -C ${CMAKE_CURRENT_BINARY_DIR}/latex
        COMMENT "Generating PDF Userguide"
        VERBATIM
     )
  endif()
  add_custom_target(
    ug_pdf ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
  )
  # Copy from latex/refman.pdf to ZZALOG.pdf
  add_custom_command(
    TARGET pdf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rename
      ${CMAKE_CURRENT_BINARY_DIR}/latex/refman.pdf
      ${CMAKE_CURRENT_BINARY_DIR}/${PROGRAMID}.pdf}
    DEPENDS ug_pdf
    COMMENT "Copying PDF"
  )
  #add_custom_target(
  #  pdf ALL
  #  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROGRAMID}.pdf
  #)



endif()
