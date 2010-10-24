# - Find CEGUIOPENGL includes and library
#
# This module defines
#  CEGUIOPENGL_INCLUDE_DIR
#  CEGUIOPENGL_LIBRARIES, the libraries to link against to use CEGUIOPENGL.
#  CEGUIOPENGL_LIB_DIR, the location of the libraries
#  CEGUIOPENGL_FOUND, If false, do not try to use CEGUIOPENGL
#
# Copyright © 2007, Matt Williams
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (CEGUIOPENGL_LIBRARIES AND CEGUIOPENGL_INCLUDE_DIR)
	SET(CEGUIOPENGL_FIND_QUIETLY TRUE)
ENDIF (CEGUIOPENGL_LIBRARIES AND CEGUIOPENGL_INCLUDE_DIR)

IF (WIN32) #Windows
	MESSAGE(STATUS "Looking for CEGUIOPENGL")
	SET(OPENGLSDK $ENV{OPENGL_HOME})
	SET(OPENGLSOURCE $ENV{OPENGL_SRC})
	IF (OPENGLSDK)
		MESSAGE(STATUS "Using CEGUIOPENGL from OPENGL SDK")
		SET(OPENGLSDK $ENV{OPENGL_HOME})
		STRING(REGEX REPLACE "[\\]" "/" OPENGLSDK "${OPENGLSDK}")
		SET(CEGUIOPENGL_INCLUDE_DIR ${OPENGLSDK}/samples/include)
		SET(CEGUIOPENGL_LIB_DIR ${OPENGLSDK}/lib)
		SET(CEGUIOPENGL_LIBRARIES debug OgreGUIRenderer_d optimized OgreGUIRenderer)
	ENDIF (OPENGLSDK)
	IF (OPENGLSOURCE)
		MESSAGE(STATUS "Using CEGUIOPENGL from OPENGL Source")
		SET(CEGUIOPENGL_INCLUDE_DIR C:/ogre/Samples/Common/CEGUIRenderer/include)
		SET(CEGUIOPENGL_LIB_DIR ${OPENGLSDK}/lib)
		SET(CEGUIOPENGL_LIBRARIES debug OgreGUIRenderer_d optimized OgreGUIRenderer)
	ENDIF (OPENGLSOURCE)
ELSE (WIN32) #Unix
	CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)
	FIND_PACKAGE(PkgConfig)
	PKG_SEARCH_MODULE(CEGUIOPENGL CEGUI-OPENGL)
	SET(CEGUIOPENGL_INCLUDE_DIR ${CEGUIOPENGL_INCLUDE_DIRS})
	SET(CEGUIOPENGL_LIB_DIR ${CEGUIOPENGL_LIBDIR})
	SET(CEGUIOPENGL_LIBRARIES ${CEGUIOPENGL_LIBRARIES} CACHE STRING "")
ENDIF (WIN32)

#Do some preparation
SEPARATE_ARGUMENTS(CEGUIOPENGL_INCLUDE_DIR)
SEPARATE_ARGUMENTS(CEGUIOPENGL_LIBRARIES)

SET(CEGUIOPENGL_INCLUDE_DIR ${CEGUIOPENGL_INCLUDE_DIR} CACHE PATH "")
SET(CEGUIOPENGL_LIBRARIES ${CEGUIOPENGL_LIBRARIES} CACHE STRING "")
SET(CEGUIOPENGL_LIB_DIR ${CEGUIOPENGL_LIB_DIR} CACHE PATH "")

IF (CEGUIOPENGL_INCLUDE_DIR AND CEGUIOPENGL_LIBRARIES)
	SET(CEGUIOPENGL_FOUND TRUE)
ENDIF (CEGUIOPENGL_INCLUDE_DIR AND CEGUIOPENGL_LIBRARIES)

IF (CEGUIOPENGL_FOUND)
	IF (NOT CEGUIOPENGL_FIND_QUIETLY)
		MESSAGE(STATUS "  libraries : ${CEGUIOPENGL_LIBRARIES} from ${CEGUIOPENGL_LIB_DIR}")
		MESSAGE(STATUS "  includes  : ${CEGUIOPENGL_INCLUDE_DIR}")
	ENDIF (NOT CEGUIOPENGL_FIND_QUIETLY)
ELSE (CEGUIOPENGL_FOUND)
	IF (CEGUIOPENGL_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find CEGUIOPENGL")
	ENDIF (CEGUIOPENGL_FIND_REQUIRED)
ENDIF (CEGUIOPENGL_FOUND)