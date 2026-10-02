cmake_minimum_required(VERSION 3.20)

# rad studio detection
if(NOT WIN32)
    message(FATAL_ERROR "SA-MP Launcher requires a Win32 RAD Studio installation.")
endif()

# msvc cannot compile vcl sources
if(CMAKE_GENERATOR MATCHES "^Visual Studio")
    message(FATAL_ERROR
        "The Visual Studio generator selects MSVC, but this project requires "
        "RAD Studio C++Builder. Use Ninja or NMake Makefiles, for example:\n"
        "  cmake -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM=PATH_TO_NINJA\n"
        "or run from a RAD Studio/Visual Studio developer shell:\n"
        "  cmake -S . -B build -G \"NMake Makefiles\"")
endif()

set(BDS "" CACHE PATH "RAD Studio / C++Builder installation directory")

if(NOT BDS AND DEFINED ENV{BDS} AND NOT "$ENV{BDS}" STREQUAL "")
    set(BDS "$ENV{BDS}" CACHE PATH "RAD Studio / C++Builder installation directory" FORCE)
endif()

if(NOT BDS)
    set(_rad_studio_roots
        "C:/Program Files/Embarcadero/Studio"
        "C:/Program Files (x86)/Embarcadero/Studio"
        "C:/Program Files/CodeGear/RAD Studio"
        "C:/Program Files (x86)/CodeGear/RAD Studio"
    )

    foreach(_root IN LISTS _rad_studio_roots)
        if(EXISTS "${_root}")
            file(GLOB _installations LIST_DIRECTORIES TRUE "${_root}/*")
            list(APPEND _rad_studio_installations ${_installations})
        endif()
    endforeach()

    # use the newest installation
    list(SORT _rad_studio_installations COMPARE NATURAL ORDER DESCENDING)
    foreach(_installation IN LISTS _rad_studio_installations)
        if(EXISTS "${_installation}/bin" AND
           (EXISTS "${_installation}/bin/bcc32x.exe" OR
            EXISTS "${_installation}/bin/bcc32c.exe" OR
            EXISTS "${_installation}/bin/bcc32.exe"))
            set(BDS "${_installation}" CACHE PATH
                "RAD Studio / C++Builder installation directory" FORCE)
            break()
        endif()
    endforeach()
endif()

if(NOT BDS OR NOT EXISTS "${BDS}/bin")
    message(FATAL_ERROR
        "RAD Studio was not found. Set BDS to its installation directory, "
        "for example -DBDS=\"C:/Program Files (x86)/Embarcadero/Studio/22.0\".")
endif()

file(TO_CMAKE_PATH "${BDS}" BDS)

if(NOT CMAKE_CXX_COMPILER)
    foreach(_compiler bcc32x.exe bcc32c.exe bcc32.exe)
        if(EXISTS "${BDS}/bin/${_compiler}")
            set(CMAKE_CXX_COMPILER "${BDS}/bin/${_compiler}"
                CACHE FILEPATH "C++Builder Win32 compiler" FORCE)
            break()
        endif()
    endforeach()
endif()

if(NOT CMAKE_CXX_COMPILER OR NOT EXISTS "${CMAKE_CXX_COMPILER}")
    message(FATAL_ERROR "No C++Builder Win32 compiler was found under ${BDS}/bin.")
endif()

# compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -v" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O2" CACHE STRING "" FORCE)

project(samp LANGUAGES CXX)

option(SAMP_STATIC "Link VCL and RTL statically" OFF)

# bcc32x uses -o for the output file
if(SAMP_STATIC)
    set(SAMP_RUNTIME_FLAG "-tR-")
else()
    set(SAMP_RUNTIME_FLAG "-tR")
endif()

set(CMAKE_CXX_LINK_EXECUTABLE
    "<CMAKE_CXX_COMPILER> ${SAMP_RUNTIME_FLAG} -o<TARGET> <LINK_FLAGS> <FLAGS> ${CMAKE_START_TEMP_FILE} <LINK_LIBRARIES> <OBJECTS>${CMAKE_END_TEMP_FILE}")

set(SAMP_SOURCES
    source/about.cpp
    source/CDnsResolver.cpp
    source/CFavorites.cpp
    source/CGameLauncher.cpp
    source/CMasterServer.cpp
    source/CRconClient.cpp
    source/CServerList.cpp
    source/CServerQuery.cpp
    source/CSettings.cpp
    source/exportfavorites.cpp
    source/findsort.cpp
    source/importfavorites.cpp
    source/main.cpp
    source/masterupdate.cpp
    source/rconconfig.cpp
    source/serverproperties.cpp
    source/settings.cpp
    source/unit_webrunform.cpp
)

set(SAMP_HEADERS
    source/stdafx.h
    source/about.h
    source/CDnsResolver.h
    source/CFavorites.h
    source/CGameLauncher.h
    source/CMasterServer.h
    source/CRconClient.h
    source/CServerList.h
    source/CServerQuery.h
    source/CSettings.h
    source/exportfavorites.h
    source/findsort.h
    source/globals.h
    source/importfavorites.h
    source/main.h
    source/masterupdate.h
    source/rconconfig.h
    source/serverproperties.h
    source/settings.h
    source/unit_webrunform.h
)

set(SAMP_FORMS
    dfm/about.dfm
    dfm/exportfavorites.dfm
    dfm/importfavorites.dfm
    dfm/main.dfm
    dfm/masterupdate.dfm
    dfm/rconconfig.dfm
    dfm/serverproperties.dfm
    dfm/settings.dfm
    dfm/unit_webrunform.dfm
)

add_executable(samp WIN32
    ${SAMP_SOURCES}
    ${SAMP_HEADERS}
    ${SAMP_FORMS}
)

# resources
set(SAMP_RES "${CMAKE_CURRENT_BINARY_DIR}/samp.res")
if(EXISTS "${BDS}/bin/cgrc.exe")
    set(SAMP_RESOURCE_COMPILER "${BDS}/bin/cgrc.exe")
elseif(EXISTS "${BDS}/bin/brcc32.exe")
    set(SAMP_RESOURCE_COMPILER "${BDS}/bin/brcc32.exe")
else()
    message(FATAL_ERROR "No RAD Studio resource compiler (cgrc.exe or brcc32.exe) was found.")
endif()

add_custom_command(
    OUTPUT "${SAMP_RES}"
    COMMAND "${SAMP_RESOURCE_COMPILER}" -c65001
            "${CMAKE_CURRENT_SOURCE_DIR}/resource/samp.rc"
            -fo"${SAMP_RES}"
    DEPENDS
        resource/samp.rc
        resource/samp.ico
        resource/samp.manifest
        "${SAMP_RESOURCE_COMPILER}"
    VERBATIM
)
target_sources(samp PRIVATE "${SAMP_RES}")
add_custom_target(samp_main_resource DEPENDS "${SAMP_RES}")
add_dependencies(samp samp_main_resource)
target_link_options(samp PRIVATE "${SAMP_RES}")
# suppress bcc32x linker warnings
target_link_options(samp PRIVATE -Wno-unused-command-line-argument)

target_include_directories(samp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_compile_definitions(samp PRIVATE
    UNICODE
    _UNICODE
)

# static runtime libraries
if(SAMP_STATIC)
    target_link_options(samp PRIVATE
        # do not use cp32mti.lib: it imports CC32CxxxxMT.dll
        cp32mt.lib
        cw32mt.lib
        rtl.lib
        vcl.lib
        Tee.lib
        vclimg.lib
        vclwinx.lib
        bindengine.lib
    )
else()
    target_link_options(samp PRIVATE
        rtl.bpi
        vcl.bpi
        Tee.bpi
        vclimg.bpi
        vclwinx.bpi
        bindengine.bpi
    )
endif()

# forms are linked through #pragma resource
set_source_files_properties(${SAMP_HEADERS} ${SAMP_FORMS}
    PROPERTIES HEADER_FILE_ONLY TRUE
)

source_group("Source Files" FILES ${SAMP_SOURCES})
source_group("Header Files" FILES ${SAMP_HEADERS})
source_group("Forms" FILES ${SAMP_FORMS})
