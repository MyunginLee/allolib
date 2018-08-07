# sets:
#   ADDITIONAL_INCLUDE_DIRS
#   ADDITIONAL_LIBRARIES
# 	ADDITIONAL_HEADERS
# 	ADDITIONAL_SOURCES
# 	ADDITIONAL_DEFINITIONS
# 	ADDITIONAL_COMPILE_FLAGS

#option(USE_APR "" OFF)
option(USE_MPI "" ON)

# al_path needs to be set prior to calling this script

  # if (USE_APR)
  #   find_package(PkgConfig REQUIRED)
  #   pkg_search_module(APR REQUIRED apr-1)
  # endif (USE_APR)

  # for freeimage, assimp, freetype
  list(APPEND CMAKE_MODULE_PATH
    ${al_path}/cmake/find_scripts
  )

  find_package(FreeImage QUIET)
  find_package(Assimp QUIET)
  find_package(Freetype QUIET)
  find_package(MPI QUIET)
  
if (AL_WINDOWS AND (NOT FREEIMAGE_FOUND))

  # This should be moved to the find script
  # if (USE_APR)
  #   set(APR_INCLUDE_DIRS ${al_path}/dependencies/apr/include)
  #   set(APR_LIBRARIES ${al_path}/dependencies/apr/libapr-1.lib)
  # endif (USE_APR)

  FIND_PATH(FREEIMAGE_INCLUDE_PATH FreeImage.h
    ${al_path}/dependencies/FreeImage/Dist/x64
    DOC "The directory where FreeImage.h resides")
  FIND_LIBRARY(FREEIMAGE_LIBRARY
    NAMES FreeImage freeimage
    PATHS ${al_path}/dependencies/FreeImage/Dist/x64
    DOC "The FreeImage library")

  IF (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)
    SET( FREEIMAGE_FOUND TRUE)
  ELSE (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)
    SET( FREEIMAGE_FOUND FALSE)
    set (FREEIMAGE_INCLUDE_PATH "")
    set (FREEIMAGE_LIBRARY "")
  ENDIF (FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY)

endif (AL_WINDOWS AND (NOT FREEIMAGE_FOUND))

set(MPI_ASSUME_NO_BUILTIN_MPI ON)
# /2809share/openmpi-3.1.1/local/lib/openmpi/lib/libmpi_cxx.so;

set (MPI_INSTALL_PREFIX "${al_path}/../openmpi-3.1.1/local")
set (MPI_CXX_LIBRARIES "${MPI_INSTALL_PREFIX}/lib/libmpi.so")
set (MPI_CXX_ADDITIONAL_INCLUDE_DIRS "${MPI_INSTALL_PREFIX}/include")
set (MPI_CXX_COMPILER "${MPI_INSTALL_PREFIX}/bin/mpicxx")
find_package(MPI QUIET)

# NOW ADD OPTIONAL FILES -------------------------------------------------------

# if (USE_APR)
#   list(APPEND ADDITIONAL_INCLUDE_DIRS ${APR_INCLUDE_DIRS})
#   list(APPEND ADDITIONAL_LIBRARIES ${APR_LIBRARIES})
#   list(APPEND ADDITIONAL_HEADERS ${al_path}/include/al/util/al_Socket.hpp)
#   list(APPEND ADDITIONAL_SOURCES ${al_path}/src/util/al_SocketAPR.cpp)
# endif(USE_APR)

if (FREEIMAGE_FOUND)
  if (AL_VERBOSE_OUTPUT)
    message("found freeimage")
  endif()
  list(APPEND ADDITIONAL_INCLUDE_DIRS ${FREEIMAGE_INCLUDE_PATH})
  list(APPEND ADDITIONAL_LIBRARIES ${FREEIMAGE_LIBRARY})
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
  message("Using MPICXX: compiler ${MPI_CXX_FOUND} ${MPI_CXX_COMPILER} ${MPI_CXX_LIBRARIES} .. ${MPI_CXX_INCLUDE_DIRS} .. ${MPIEXEC_EXECUTABLE}")
  set(MPI_DEFINITIONS "-DAL_BUILD_MPI")
  list(APPEND ADDITIONAL_INCLUDE_DIRS ${MPI_CXX_INCLUDE_DIRS} ${MPI_CXX_ADDITIONAL_INCLUDE_DIRS})
  list(APPEND ADDITIONAL_LIBRARIES ${MPI_CXX_LIBRARIES})
  list(APPEND ADDITIONAL_DEFINITIONS ${MPI_DEFINITIONS})
  list(APPEND ADDITIONAL_COMPILE_FLAGS ${MPI_CXX_COMPILE_FLAGS})
  list(APPEND ADDITIONAL_LINK_FLAGS ${MPI_CXX_LINK_FLAGS})
#  list(APPEND ADDITIONAL_HEADERS ${al_path}/include/al/util/al_Font.hpp)
#  list(APPEND ADDITIONAL_SOURCES ${al_path}/src/util/al_Font.cpp)
endif()
