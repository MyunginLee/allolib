# al_path needs to be set prior to calling this script

# sets:
#   ADDITIONAL_INCLUDE_DIRS
#   ADDITIONAL_LIBRARIES
# 	ADDITIONAL_HEADERS
# 	ADDITIONAL_SOURCES
# 	ADDITIONAL_DEFINITIONS
# 	ADDITIONAL_COMPILE_FLAGS

option(USE_MPI "" ON)

# TRY FINDING PACKAGES --------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH
  ${al_path}/cmake/find_scripts
)

if (AL_WINDOWS)
  # local search will look into dependency files in project directory "windows"
  find_package(Freeimagelocal QUIET)
  find_package(Freetypelocal QUIET)
  # TODO: also write assimp local script
  # find_package(Assimplocal QUIET)
else()
  find_package(FreeImage QUIET)
  find_package(Assimp QUIET)
  find_package(Freetype QUIET) # note: freetype find script comes with cmake
  find_package(MPI QUIET)
endif()

# NOW ADD OPTIONAL FILES ------------------------------------------------------

if (FREEIMAGE_FOUND)
  if (AL_VERBOSE_OUTPUT)
    message("found freeimage")
  endif()
  list(APPEND ADDITIONAL_INCLUDE_DIRS ${FREEIMAGE_INCLUDE_DIRS})
  list(APPEND ADDITIONAL_LIBRARIES ${FREEIMAGE_LIBRARIES})
  list(APPEND ADDITIONAL_HEADERS ${al_path}/include/al/util/al_Image.hpp)
  list(APPEND ADDITIONAL_SOURCES ${al_path}/src/util/al_Image.cpp)
endif (FREEIMAGE_FOUND)

if (ASSIMP_LIBRARY)
  if (AL_VERBOSE_OUTPUT)
    message("found assimp")
  endif()
  list(APPEND ADDITIONAL_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR})
  list(APPEND ADDITIONAL_LIBRARIES ${ASSIMP_LIBRARY})
  list(APPEND ADDITIONAL_HEADERS ${al_path}/include/al/util/al_Asset.hpp)
  list(APPEND ADDITIONAL_SOURCES ${al_path}/src/util/al_Asset.cpp)
endif()

if (FREETYPE_INCLUDE_DIRS)
  if (AL_VERBOSE_OUTPUT)
    message("found freetype")
  endif()
  list(APPEND ADDITIONAL_INCLUDE_DIRS ${FREETYPE_INCLUDE_DIRS})
  list(APPEND ADDITIONAL_LIBRARIES ${FREETYPE_LIBRARY})
  list(APPEND ADDITIONAL_HEADERS ${al_path}/include/al/util/al_Font.hpp)
  list(APPEND ADDITIONAL_SOURCES ${al_path}/src/util/al_Font.cpp)
endif()

if (USE_MPI AND MPI_CXX_FOUND)
  message("Using MPI: compiler ${MPI_C_COMPILER} ${MPI_CXX_INCLUDE_PATH}")
  set(MPI_DEFINITIONS "-DAL_BUILD_MPI")
  list(APPEND ADDITIONAL_INCLUDE_DIRS ${MPI_CXX_INCLUDE_PATH})
  list(APPEND ADDITIONAL_LIBRARIES ${MPI_CXX_LIBRARIES})
  list(APPEND ADDITIONAL_DEFINITIONS ${MPI_DEFINITIONS})
  list(APPEND ADDITIONAL_COMPILE_FLAGS ${MPI_CXX_COMPILE_FLAGS})
  list(APPEND ADDITIONAL_LINK_FLAGS ${MPI_CXX_LINK_FLAGS})
endif()
