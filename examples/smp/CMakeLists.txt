# ===========================================
# Copyright KAPSARC. Open Source MIT License 
# ===========================================


project(smp)

cmake_minimum_required (VERSION 2.8)
cmake_policy(VERSION 2.8)
cmake_policy(SET CMP0017 NEW) # prefer built-in modules
cmake_policy(SET CMP0020 NEW) # Automatically link Qt executables to qtmain


set (LOCAL_MODULE_DIR
    ${PROJECT_SOURCE_DIR}/../../KTAB/cmakemodules)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${LOCAL_MODULE_DIR}
    )

set(CMAKE_BUILD_TYPE Debug)

set(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/)
set(EXECUTABLE_OUTPUT_PATH   ${PROJECT_SOURCE_DIR}/)

set (ENABLE_QT_SMPQ_GUI true CACHE  BOOL "Build KTAB SMP app with QT GUI")
set (ENABLE_QT_SASQ_GUI true CACHE  BOOL "Build KTAB SAS app with QT GUI")
set (ENABLE_COPY_QT_LIBS false CACHE  BOOL "Copy Qt LIBS after build")

if (UNIX)
    set (ENABLE_EFFCPP false CACHE  BOOL "Check Effective C++ Guidelines")
    set (ENABLE_EFENCE false CACHE  BOOL "Use Electric Fence memory debugger")
endif(UNIX)

# -------------------------------------------------

if (ENABLE_EFENCE)
  find_package(efence)
  if (NOT EFENCE_FOUND)
  message(FATAL_ERROR "Could not find Electric Fence memory debugger")
  endif (NOT EFENCE_FOUND)
endif (ENABLE_EFENCE)

# -------------------------------------------------

find_package(TinyXML2)
if (NOT TINYXML2_FOUND)
    message(FATAL_ERROR "Could not find TinyXML2")
endif (NOT TINYXML2_FOUND) 

# -------------------------------------------------
find_package(Sqlite)
if (NOT SQLITE_FOUND)
    message(FATAL_ERROR "Could not find SQLite")
endif (NOT SQLITE_FOUND)

# --------------------------------------------------------

find_package(Easyloggingpp)

# --------------------------------------------------------

set (SMP_XLIBS  )

# --------------------------------------------------------
# See "Findkutils.cmake" in cmakemodules to figure
# out where it looks and what variables it sets
find_package(kutils)
if(NOT KUTILS_FOUND)
    message(FATAL_ERROR "Could not find kutils")
endif(NOT KUTILS_FOUND)


# Ditto for "Findkmodel.cmake"
find_package(kmodel)
if(NOT KMODEL_FOUND)
    message(FATAL_ERROR "Could not find kmodel")
endif(NOT KMODEL_FOUND)

# -------------------------------------------------
# files to be built into a library go in libsrc/
# any application or testing code goes in src
# -------------------------------------------------

set (MINICSV_PATHS
  ${PROJECT_SOURCE_DIR}/../../minicsv/
  )


find_path(MINICSV_DIR minicsv.h
    PATHS ${MINICSV_PATHS}
    )

if(MINICSV_DIR)
    message(STATUS "Found minicsv headers: ${MINICSV_DIR}")
endif(MINICSV_DIR)

if(NOT MINICSV_DIR)
    message(FATAL "Could not find minicsv headers")
endif(NOT MINICSV_DIR)

# -------------------------------------------------
find_package(qtlibs)
set(CMAKE_PREFIX_PATH ${PREFIX_PATH})
message(STATUS "CMAKE_PREFIX_PATH" ${CMAKE_PREFIX_PATH})

find_package(Qt5 REQUIRED COMPONENTS Core Sql)

# -------------------------------------------------    
#Enable export of all classes and functions for windows platform
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include_directories (
  ${KUTILS_INCLUDE_DIR}
  ${KMODEL_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/libsrc/
  ${PROJECT_SOURCE_DIR}/src/
  ${MINICSV_DIR}
  ${SQLITE_INCLUDE_DIR}
  ${TINYXML2_INCLUDE_DIR}
  ${LOGGER_INCLUDE_DIR}
  )

# note that we do not put the csv_parser into smp lib.
set(SMPLIB_SRCS
    ${PROJECT_SOURCE_DIR}/libsrc/smp.cpp
    ${PROJECT_SOURCE_DIR}/libsrc/smpbcn.cpp
    ${PROJECT_SOURCE_DIR}/libsrc/smpread.cpp
    ${PROJECT_SOURCE_DIR}/libsrc/smpsql.cpp
    )

set(KTAB_DIR ${PROJECT_SOURCE_DIR}/../../KTAB)

set(KUTILS_SRC_DIR ${KTAB_DIR}/kutils)
set(KUTILS_SRCS
  ${KUTILS_SRC_DIR}/libsrc/kutils.cpp
  ${KUTILS_SRC_DIR}/libsrc/prng.cpp
  ${KUTILS_SRC_DIR}/libsrc/gaopt.cpp
  ${KUTILS_SRC_DIR}/libsrc/kmatrix.cpp
  ${KUTILS_SRC_DIR}/libsrc/hcsearch.cpp
  ${KUTILS_SRC_DIR}/libsrc/vimcp.cpp
)

set(KMODEL_SRC_DIR ${KTAB_DIR}/kmodel)
set(KMODEL_SRCS
  ${KMODEL_SRC_DIR}/libsrc/kmodel.cpp
  ${KMODEL_SRC_DIR}/libsrc/kmodelsql.cpp
  ${KMODEL_SRC_DIR}/libsrc/emodel.cpp
  ${KMODEL_SRC_DIR}/libsrc/kstate.cpp
  ${KMODEL_SRC_DIR}/libsrc/kposition.cpp
  )

add_library(smpDyn SHARED ${KUTILS_SRCS} ${KMODEL_SRCS} ${SMPLIB_SRCS})
if(UNIX)
  set_property(TARGET smpDyn PROPERTY POSITION_INDEPENDENT_CODE ON)
endif(UNIX)

target_link_libraries (smpDyn
  ${SQLITE_LIBRARIES}
  ${EFENCE_LIBRARIES}
  ${TINYXML2_LIBRARIES}
  ${LOGGER_LIBRARY}
  Qt5::Widgets
  Qt5::Core
  Qt5::Sql
 )

add_executable (smpcDyn
  src/demosmp.cpp
  )

target_link_libraries (smpcDyn
  smpDyn
  )

# -------------------------------------------------
add_library(smp STATIC ${SMPLIB_SRCS})

target_link_libraries (smp
  Qt5::Widgets
  Qt5::Core
  Qt5::Sql
  )

# -------------------------------------------------
# Under Linux, default start of DESTINATION path is /usr/local/ 
# so "ktab/lib" denotes destination /usr/local/ktab/lib
# Under Windows, default start of DESTINATION path is "C:\Program Files\kutils\"
# so "ktab/lib" denotes destination C:\Program Files\kutils\ktab\lib

set (SMP_INSTALL_DIR)
if (WIN32)
  set (SMP_INSTALL_DIR
  c:/local/ktab
  )
endif(WIN32)
if (UNIX)
  set (SMP_INSTALL_DIR
  /usr/local/ktab
  )
endif(UNIX)

install(
    TARGETS
    smp
    DESTINATION
    ${SMP_INSTALL_DIR}/lib)

install(
    FILES
    libsrc/smp.h
    DESTINATION
    ${SMP_INSTALL_DIR}/include)

# -------------------------------------------------

add_executable (smpc
    src/demosmp.cpp
    )

target_link_libraries (smpc
  smp
  ${KMODEL_LIBRARY}
  ${KUTILS_LIBRARY}
  ${SQLITE_LIBRARIES}
  ${EFENCE_LIBRARIES}
  ${TINYXML2_LIBRARIES}
  ${LOGGER_LIBRARY}
  )

#--------------------------------------------------
#smpq qt based application

if(ENABLE_QT_SMPQ_GUI)


  find_package(smpq)
  if(NOT SMPQ_FOUND)
  message(FATAL_ERROR "Could not find SMPQ")
  endif(NOT SMPQ_FOUND)

    find_package(Qt5 REQUIRED COMPONENTS Widgets Core Sql PrintSupport)

    qt5_wrap_cpp(QtProjectLib_hdr_moc ${QtProjectLib_hdr})
    qt5_wrap_ui(QtProjectLib_ui_uic ${QtProjectLib_ui})
    qt5_add_resources(QtProjectLib_qrc_rcc ${QtProjectRsc_qrc})

    include_directories (${PROJECT_SOURCE_DIR})
    include_directories (${PROJECT_BINARY_DIR})

    add_library (QtProjectLib STATIC ${QtProjectLib_src} ${QtProjectLib_hdr_moc} ${QtProjectLib_ui_uic})

    target_link_libraries (QtProjectLib
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Sql
        Qt5::PrintSupport)
        
    # WIN32 to suppress the console window under Windows
    # MACOSX_BUNDLE to create the OS X bundle for KTAB_SMP Release
    add_executable(KTAB_SMP WIN32 MACOSX_BUNDLE ${QtProjectBin_src} ${QtProjectLib_qrc_rcc})

    target_link_libraries (KTAB_SMP
        QtProjectLib
        smp
        ${KMODEL_LIBRARY}
        ${KUTILS_LIBRARY}
        ${SQLITE_LIBRARIES}
        ${EFENCE_LIBRARIES}
        ${TINYXML2_LIBRARIES}
        ${LOGGER_LIBRARY})

    if(ENABLE_COPY_QT_LIBS AND WIN32)
        find_package(qtdlls)
    endif(ENABLE_COPY_QT_LIBS AND WIN32)

endif (ENABLE_QT_SMPQ_GUI)

#------------------------------------------------------------------
#sasq qt based application

if(ENABLE_QT_SASQ_GUI)

    find_package(qtlibs)

    find_package(sasq)
    if(NOT SASQ_FOUND)
        message(FATAL_ERROR "Could not find SASQ")
    endif(NOT SASQ_FOUND)

    set(CMAKE_PREFIX_PATH ${PREFIX_PATH})
    message(STATUS "CMAKE_PREFIX_PATH" ${CMAKE_PREFIX_PATH})

    find_package(Qt5 REQUIRED COMPONENTS Widgets Core Sql PrintSupport)

    qt5_wrap_cpp(QtSASQProjectLib_hdr_moc ${QtSASQProjectLib_hdr})
    qt5_wrap_ui(QtSASQProjectLib_ui_uic ${QtSASQProjectLib_ui})
    qt5_add_resources(QtSASQProjectLib_qrc_rcc ${QtSASQProjectRsc_qrc})

    include_directories (${PROJECT_SOURCE_DIR})
    include_directories (${PROJECT_BINARY_DIR})

    add_library (QtSASQProjectLib STATIC ${QtSASQProjectLib_src} ${QtSASQProjectLib_hdr_moc} ${QtSASQProjectLib_ui_uic})

    target_link_libraries (QtSASQProjectLib
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Sql
        Qt5::PrintSupport)

    # WIN32 to suppress the console window under Windows
    # MACOSX_BUNDLE to create the OS X bundle for KTAB_SAS Release
    add_executable(KTAB_SAS WIN32 MACOSX_BUNDLE ${QtSASQProjectBin_src} ${QtSASQProjectLib_qrc_rcc})

    target_link_libraries (KTAB_SAS
        QtSASQProjectLib
        smp
        ${KMODEL_LIBRARY}
        ${KUTILS_LIBRARY}
        ${SQLITE_LIBRARIES}
        ${EFENCE_LIBRARIES}
        ${TINYXML2_LIBRARIES}
        ${LOGGER_LIBRARY})

    if(ENABLE_COPY_QT_LIBS AND WIN32)
        find_package(qtdlls)
    endif(ENABLE_COPY_QT_LIBS AND WIN32)

endif (ENABLE_QT_SASQ_GUI)

#------------------------------------------------------------------
# show some useful status/debugging information
message (STATUS "Using PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
message (STATUS "KUTILS_INCLUDE_DIR: " ${KUTILS_INCLUDE_DIR})
message (STATUS "KMODEL_INCLUDE_DIR: " ${KMODEL_INCLUDE_DIR})
message (STATUS "MINICSV_DIR: " ${MINICSV_DIR})
message (STATUS "QCUSTOMPLOT_DIR: " ${QCUSTOMPLOT_DIR})

# -------------------------------------------------
# As of early 2014, C++11 is still not the default
# for g++, so I have to provide it here.
if (UNIX OR MINGW)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11 ")
  if (ENABLE_EFFCPP)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++ ")
  endif (ENABLE_EFFCPP)
endif(UNIX OR MINGW)

# ===========================================
# Copyright KAPSARC. Open Source MIT License 
# ===========================================
