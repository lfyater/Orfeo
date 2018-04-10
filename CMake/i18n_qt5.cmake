#
# Copyright (C) 2005-2017 Centre National d'Etudes Spatiales (CNES)
#
# This file is part of Orfeo Toolbox
#
#     https://www.orfeo-toolbox.org/
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

#
# Reset Qt I18N source files cache variable.
macro( reset_qt5_i18n_sources )
  set( OTB_QT_I18N_INCLUDE_PATH "" CACHE INTERNAL "" FORCE )
  set( OTB_QT_I18N_HEADER_FILES "" CACHE INTERNAL "" FORCE )
  set( OTB_QT_I18N_SOURCE_FILES "" CACHE INTERNAL "" FORCE )
  set( OTB_QT_I18N_FORM_FILES "" CACHE INTERNAL "" FORCE )
endmacro()

# string( REPLACE ".cxx" ".h" OTB_QT_I18N_HEADER_FILES OTB_QT_I18N_SOURCE_FILES )

# string( REPLACE ".cxx" ".ui" OTB_QT_I18N_HEADER_FILES OTB_QT_I18N_SOURCE_FILES )

#
# Func
function( add_to_qt5_i18n_files RESULT )
  foreach( F ${ARGN} )
    #message( "${F}" )

    get_filename_component( ABS_FILE ${F} ABSOLUTE )

    set(${RESULT} ${${RESULT}} ${ABS_FILE} CACHE INTERNAL "")
  endforeach()
endfunction()

#
#
macro( add_to_qt5_i18n_include_path DIRECTORY )
  set(OTB_I18N_INCLUDE_PATH ${OTB_I18N_INCLUDE_PATH} ${DIRECTORY} CACHE INTERNAL "")
endmacro()

#
#
macro( add_to_qt5_i18n_headers INCLUDE_DIR )

  get_filename_component( ABS_INCLUDE_DIR ${INCLUDE_DIR} ABSOLUTE )

  set(OTB_QT_I18N_INCLUDE_PATH ${OTB_QT_I18N_INCLUDE_PATH} ${ABS_INCLUDE_DIR} CACHE INTERNAL "")

  foreach( F ${ARGN} )
    string( REPLACE ".cxx" ".h" HEADER ${F} )
    set( ABS_HEADER "${ABS_INCLUDE_DIR}/${HEADER}" )
    if(EXISTS ${ABS_HEADER})
      set(OTB_QT_I18N_HEADER_FILES ${OTB_QT_I18N_HEADER_FILES} ${ABS_HEADER} CACHE INTERNAL "")
    endif()
  endforeach()
endmacro()

#
# Add source files to Qt I18n translation build.
macro( add_to_qt5_i18n_sources )
  add_to_qt5_i18n_files( OTB_QT_I18N_SOURCE_FILES ${ARGN} )
endmacro()

#
# Add source files to Qt I18n translation build.
macro( add_to_qt5_i18n_forms )
  add_to_qt5_i18n_files( OTB_QT_I18N_FORM_FILES ${ARGN} )
endmacro()

#
#
macro( generate_qt5_project FILENAME )
  message( STATUS "Generating Qt5 '${FILENAME}' project file for I18N." )

  unset(_OTB_QT_I18N_INCLUDE_PATH_PRO)
  unset(_OTB_QT_I18N_HEADER_PRO)
  unset(_OTB_QT_I18N_SOURCE_PRO)
  unset(_OTB_QT_I18N_FORM_PRO)
  foreach(_path ${OTB_QT_I18N_INCLUDE_PATH})
    set(_OTB_QT_I18N_INCLUDE_PATH_PRO
      "${_OTB_QT_I18N_INCLUDE_PATH_PRO} \"${_path}\"")
  endforeach()
  foreach(_file ${OTB_QT_I18N_HEADER_FILES})
    set(_OTB_QT_I18N_HEADER_PRO
      "${_OTB_QT_I18N_HEADER_PRO} \"${_file}\"")
  endforeach()
  foreach(_file ${OTB_QT_I18N_SOURCE_FILES})
    set(_OTB_QT_I18N_SOURCE_PRO
      "${_OTB_QT_I18N_SOURCE_PRO} \"${_file}\"")
  endforeach()
  foreach(_file ${OTB_QT_I18N_FORM_FILES})
    set(_OTB_QT_I18N_FORM_PRO
      "${_OTB_QT_I18N_FORM_PRO} \"${_file}\"")
  endforeach()

  file( WRITE
    ${FILENAME}

    "# OTB autogenerated .pro file needed to generate Qt I18N translations

INCLUDEPATH = ${_OTB_QT_I18N_INCLUDE_PATH_PRO}

HEADERS = ${_OTB_QT_I18N_HEADER_PRO}

SOURCES = ${_OTB_QT_I18N_SOURCE_PRO}

FORMS = ${_OTB_QT_I18N_FORM_PRO}
"
    )
endmacro()