cmake_minimum_required(VERSION 3.16)

project(zzalog_fe)

set(TARGET zzalog_fe)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CPPFILES 
  src/about_dialog.cpp
  src/adi_reader.cpp
  src/adi_writer.cpp
  src/adx_handler.cpp
  src/band_data.cpp
  src/band_editor.cpp
  src/band_widget.cpp
  src/band_window.cpp
  src/banner.cpp
  src/book.cpp
  src/calendar.cpp
  src/calendar_input.cpp
  src/calendar_table.cpp
  src/change_dialog.cpp
  src/club_handler.cpp
  src/condx_view.cpp
  src/config.cpp
  src/contest_algorithm.cpp
  src/contest_data.cpp
  src/contest_dialog.cpp
  src/contest_scorer.cpp
  src/corr_dialog.cpp
  src/cty1_reader.cpp
  src/cty2_reader.cpp
  src/cty3_reader.cpp
  src/cty_data.cpp
  src/cty_dialog.cpp
  src/cty_element.cpp
  src/dxcc_table.cpp
  src/dxcc_view.cpp
  src/eqsl_handler.cpp
  src/extract_data.cpp
  src/field_choice.cpp
  src/fields.cpp
  src/fields_dialog.cpp
  src/file_holder.cpp
  src/file_viewer.cpp
  src/fllog_emul.cpp
  src/font_dialog.cpp
  src/import_data.cpp
  src/init_dialog.cpp
  src/intl_dialog.cpp
  src/intl_widgets.cpp
  src/log_table.cpp
  src/lotw_handler.cpp
  src/main.cpp
  src/main_window.cpp
  src/menu.cpp
  src/page_dialog.cpp
  src/password_input.cpp
  src/png_writer.cpp
  src/printer.cpp
  src/qrz_handler.cpp
  src/qsl_dataset.cpp
  src/qsl_display.cpp
  src/qsl_editor.cpp
  src/qsl_emailer.cpp
  src/qsl_image.cpp
  src/qsl_widget.cpp
  src/qso_apps.cpp
  src/qso_bands.cpp
  src/qso_buttons.cpp
  src/qso_clock.cpp
  src/qso_clocks.cpp
  src/qso_data.cpp
  src/qso_details.cpp
  src/qso_dxcc.cpp
  src/qso_entry.cpp
  src/qso_log.cpp
  src/qso_log_info.cpp
  src/qso_manager.cpp
  src/qso_misc.cpp
  src/qso_net_entry.cpp
  src/qso_operation.cpp
  src/qso_qsl.cpp
  src/qso_qsl_vwr.cpp
  src/qso_query.cpp
  src/qso_rig.cpp
  src/qso_tabbed_rigs.cpp
  src/qso_wx.cpp
  src/record.cpp
  src/record_table.cpp
  src/report_tree.cpp
  src/rig_data.cpp
  src/rig_if.cpp
  src/rpc_data_item.cpp
  src/rpc_handler.cpp
  src/search_dialog.cpp
  src/serial.cpp
  src/settings.cpp
  src/socket_server.cpp
  src/spec_data.cpp
  src/spec_tree.cpp
  src/status.cpp
  src/stn_call_dlg.cpp
  src/stn_data.cpp
  src/stn_dialog.cpp
  src/stn_oper_dlg.cpp
  src/stn_qth_dlg.cpp
  src/symbols.cpp
  src/tabbed_forms.cpp
  src/ticker.cpp
  src/toolbar.cpp
  src/url_handler.cpp
  src/user_dialog.cpp
  src/view.cpp
  src/web_dialog.cpp
  src/win_dialog.cpp
  src/wsjtx_handler.cpp
  src/wx_handler.cpp
  src/contests/basic.cpp
  src/contests/iaru_hf.cpp

)

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
set(CMAKE_MODULE_PATH ../ ./)
find_package(FLTK 1.5.0 CONFIG REQUIRED)
find_package(CURL)
if (NOT CURL_FOUND AND MSVC)
  message("CURL not found")
endif()
find_package(Hamlib)
if (NOT HAMLIB_FOUND AND MSVC)
  message("Hamlib not found")
  set(HAMLIB_INCLUDE_DIR "C:\\Program Files\\hamlib-w64-4.6.5\\include")
  set(HAMLIB_LIBRARY "C:\\Program Files\\hamlib-w64-4.6.5\\lib\\msvc\\libhamlib-4.lib")
endif()

set(CMAKE_BUILD_RPATH ${CMAKE_CURRENT_BINARY_DIR})

add_executable(${TARGET} WIN32 MACOSX_BUNDLE)


target_include_directories(${TARGET} PRIVATE 
  ./include ../common/include ../widgets/include
  ${FLTK_INCLUDE_DIR}
  ${HAMLIB_INCLUDE_DIR}
)
# Add QBS_logo.h to ensure it gets generated
target_sources(${TARGET} PRIVATE ${CPPFILES})

if (MSVC)
else()
target_link_directories(${TARGET} PRIVATE
  /usr/local/lib
)
endif()

target_link_libraries(${TARGET} PRIVATE 
  zzalog_common
  zzalog_widgets
  pugixml
  fltk::fltk
  fltk::images
  ${CURL_LIBRARIES}
  ${HAMLIB_LIBRARY}
  
)

