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

add_subdirectory(bliss)

include_directories( ${PROJECT_SOURCE_DIR} )

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/solvertypesmini.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/breakid/solvertypesmini.hpp" @ONLY)

include_directories ( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
set(breakid_lib_files
    Algebraic.cpp
    Breaking.cpp
    Graph.cpp
    Theory.cpp
    breakid.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp
    $<TARGET_OBJECTS:bliss>
)

add_library(breakid
    ${breakid_lib_files}
)

target_link_libraries(breakid
    # nothing
)

set_target_properties(breakid PROPERTIES
    PUBLIC_HEADER "${breakid_public_headers}"
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    SOVERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

###############

# add_executable(breakid-bin
#     main.cpp
# )
# set_target_properties(breakid-bin PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
#     INSTALL_RPATH_USE_LINK_PATH TRUE
#     OUTPUT_NAME breakid
# )
#
# if (STATICCOMPILE)
#     SET_TARGET_PROPERTIES(breakid-bin PROPERTIES LINK_SEARCH_START_STATIC 1)
# endif()
#
# target_link_libraries(breakid-bin
#     breakid
#     # ${Boost_LIBRARIES}
# )

add_executable(mytest
    test.cpp
)
set_target_properties(mytest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    OUTPUT_NAME mytest
)
target_link_libraries(mytest
    breakid
    # ${Boost_LIBRARIES}
)


breakid_add_public_header(breakid ${CMAKE_CURRENT_SOURCE_DIR}/breakid.hpp )
breakid_add_public_header(breakid ${CMAKE_CURRENT_BINARY_DIR}/breakid/solvertypesmini.hpp )

# -----------------------------------------------------------------------------
# Copy public headers into build directory include directory.
# The breakidConfig.cmake we generate in the build directory depends on
# this.
# -----------------------------------------------------------------------------
set(HEADER_DEST "${PROJECT_BINARY_DIR}/include/breakid")
add_custom_target(CopyPublicHeaders ALL)
get_target_property(breakid_public_headers breakid PUBLIC_HEADER)
foreach(public_header ${breakid_public_headers})
    get_filename_component(HEADER_NAME ${public_header} NAME)
    add_custom_command(TARGET CopyPublicHeaders PRE_BUILD
                       COMMAND ${CMAKE_COMMAND} -E make_directory
                               "${HEADER_DEST}"
                       COMMAND ${CMAKE_COMMAND} -E echo
                       "Copying ${HEADER_NAME} to ${HEADER_DEST}"
                       COMMAND ${CMAKE_COMMAND} -E
                           copy_if_different
                           ${public_header}
                           "${HEADER_DEST}"
                      )
endforeach()


install(TARGETS breakid
    EXPORT ${BREAKID_EXPORT_NAME}
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/breakid"
)


install(TARGETS mytest
    # EXPORT ${BREAKID_EXPORT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
