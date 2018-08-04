#
# Try to find the FreeImage library and include path.
# Once done this will define
#
# FREEIMAGE_FOUND
# FREEIMAGE_INCLUDE_DIRS
# FREEIMAGE_LIBRARIES
#

FIND_PATH(FREEIMAGE_INCLUDE_DIRS FreeImage.h
	/usr/include
	/usr/local/include
	/sw/include
	/opt/local/include
	DOC "The directory where FreeImage.h resides"
)

FIND_LIBRARY(FREEIMAGE_LIBRARIES
	NAMES FreeImage freeimage
	PATHS
	/usr/lib64
	/usr/lib
	/usr/local/lib64
	/usr/local/lib
	/sw/lib
	/opt/local/lib
	DOC "The FreeImage library"
)

IF (FREEIMAGE_INCLUDE_DIRS AND FREEIMAGE_LIBRARIES)
	SET( FREEIMAGE_FOUND TRUE CACHE BOOL "Set to TRUE if FreeImage is found, FALSE otherwise")
ELSE (FREEIMAGE_INCLUDE_DIRS AND FREEIMAGE_LIBRARIES)
	SET( FREEIMAGE_FOUND FALSE CACHE BOOL "Set to TRUE if FreeImage is found, FALSE otherwise")
ENDIF (FREEIMAGE_INCLUDE_DIRS AND FREEIMAGE_LIBRARIES)

MARK_AS_ADVANCED(
	FREEIMAGE_FOUND
	FREEIMAGE_LIBRARIES
	FREEIMAGE_INCLUDE_DIRS
)

