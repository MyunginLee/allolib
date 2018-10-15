cmake_minimum_required(VERSION 3.0)
project(allolib)

MACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD MSG)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" insource)
     GET_FILENAME_COMPONENT(PARENTDIR ${CMAKE_SOURCE_DIR} PATH)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${PARENTDIR}" insourcesubdir)
     IF(insource OR insourcesubdir)
         MESSAGE(FATAL_ERROR "${MSG}")
     ENDIF(insource OR insourcesubdir)
ENDMACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)

MACRO_ENSURE_OUT_OF_SOURCE_BUILD(
    "${CMAKE_PROJECT_NAME} requires an out of source build. Make a directory and run cmake there."
)

set(al_path ${CMAKE_CURRENT_LIST_DIR})
option(AL_VERBOSE_OUTPUT "" OFF)
option(BUILD_EXAMPLES "" ON)
option(TRAVIS_BUILD "" OFF)
option(APPVEYOR_BUILD "" OFF)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(AL_MACOS 1)
  set(PLATFORM_DEFINITION -DAL_OSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(AL_LINUX 1)
  set(PLATFORM_DEFINITION -DAL_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(AL_WINDOWS 1)
  set(PLATFORM_DEFINITION -DAL_WINDOWS -D_USE_MATH_DEFINES)
  if (NOT ${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
    message("Warning! Not building for Win64 : ${CMAKE_GENERATOR_PLATFORM}")
  endif(NOT ${CMAKE_GENERATOR_PLATFORM} STREQUAL "x64")
else ()
  message(FATAL_ERROR "system platform not defined")
endif ()

if(TRAVIS_BUILD) 
  add_definitions(-DTRAVIS_BUILD)
endif(TRAVIS_BUILD)

# finds local glfw that should have been installed by "scripts/glfw3.2.sh"
find_package(glfw3 REQUIRED CONFIG # The CONFIG option may be used to skip Module mode explicitly and switch to Config mode
             PATHS ${CMAKE_CURRENT_LIST_DIR}/lib/cmake/glfw3
             NO_DEFAULT_PATH # If NO_DEFAULT_PATH is specified all NO_* options are enabled.
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/Gamma ${CMAKE_CURRENT_BINARY_DIR}/Gamma)

include(cmake/find_additional_dependencies.cmake)
include(cmake/external.cmake)
include(cmake/al_files.cmake)

add_library(al STATIC
	${al_headers} ${ADDITIONAL_HEADERS}
	${al_sources} ${EXTERNAL_SRC} ${ADDITIONAL_SOURCES}
)

set_target_properties(al PROPERTIES
    DEBUG_POSTFIX d
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/lib
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/lib
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

if (AL_WINDOWS)
    target_compile_options(al PRIVATE "")
else ()
    target_compile_options(al PRIVATE "-Wall")
endif (AL_WINDOWS)

target_compile_definitions(al PUBLIC
    ${PLATFORM_DEFINITION} ${EXTERNAL_DEFINITIONS} ${ADDITIONAL_DEFINITIONS}
)

target_include_directories(al PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include ${ADDITIONAL_INCLUDE_DIRS} ${EXTERNAL_INCLUDE_DIRS}
)

target_link_libraries(al PUBLIC
    ${OPENGL_gl_LIBRARY} glfw Gamma ${ADDITIONAL_LIBRARIES} ${EXTERNAL_LIBRARIES}
)

if (NOT AL_WINDOWS)
 target_link_libraries(al PUBLIC dl)
endif ()

# --- Testing -----------------------------------------
if (ALLOLIB_BUILD_TESTS)
  add_subdirectory(test)
endif()

# ---- Examples ---------------------------------------
include("cmake/example_targets.cmake")

