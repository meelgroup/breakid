# Copyright (C) 2018  Mate Soos
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

add_library(breakid)

add_subdirectory(bliss)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)

target_sources(breakid PRIVATE
    Algebraic.cpp
    Breaking.cpp
    Graph.cpp
    Theory.cpp
    breakid.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp
    breakid_c.cpp
)
target_include_directories(breakid PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/breakid>"
)

set_target_properties(breakid PROPERTIES
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(WIN32)
    set_target_properties(breakid PROPERTIES
        OUTPUT_NAME breakidwin
    )
endif()

add_executable(breakid-bin breakid-main.cpp)
set_target_properties(breakid-bin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    OUTPUT_NAME breakid
    EXPORT_NAME bin
)
target_link_libraries(breakid-bin breakid)

breakid_add_public_header(breakid ${CMAKE_CURRENT_SOURCE_DIR}/breakid_c.h )
breakid_add_public_header(breakid ${CMAKE_CURRENT_SOURCE_DIR}/breakid.hpp )

# -----------------------------------------------------------------------------
# Copy public headers into build directory include directory.
# The breakidConfig.cmake we generate in the build directory depends on
# this.
# -----------------------------------------------------------------------------

install(TARGETS breakid breakid-bin
    EXPORT ${BREAKID_EXPORT_NAME}
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/breakid"
)
