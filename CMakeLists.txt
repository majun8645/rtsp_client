cmake_minimum_required(VERSION 2.8)
project(RtspClient CXX)
 
set(RTSP_CLIENT_MAJOR_VERSION 0)
set(RTSP_CLIENT_MINOR_VERSION 1)
set(RTSP_CLIENT_PATCH_VERSION 0)
set(RTSP_CLIENT_VERSION ${RTSP_CLIENT_MAJOR_VERSION}.${RTSP_CLIENT_MINOR_VERSION}.${RTSP_CLIENT_PATCH_VERSION})

set(CMAKE_COLOR_MAKEFILE ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Include our extra modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
 # Turn on solution folders (2.8.4+)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")

if(WIN32 AND NOT CYGWIN)
  set(DEF_INSTALL_CMAKE_DIR CMake)
else()
  set(DEF_INSTALL_CMAKE_DIR lib/CMake/RtspClient)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")
 
# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()
 
if (NOT OPT_BUILD_RELEASE)
	set(CMAKE_BUILD_TYPE "Debug")
  set(DEBUG 1)
endif()

#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  #set(DEBUG 1)
#endif()

if(NOT DEFINED BUILD_SHARED_LIBS)
	set(BUILD_SHARED_LIBS OFF)
endif()

if(NOT DEFINED EXPORT_ALL_SYMBOLS)
	set(EXPORT_ALL_SYMBOLS TRUE)
endif()

# Configure MSVC Runtime
if(MSVC)
	include(MSVCRuntime)
	configure_msvc_runtime()

	if(NOT DEFINED CMAKE_SUPPRESS_REGENERATION)
		set(CMAKE_SUPPRESS_REGENERATION ON)
	endif()
endif()

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} /Gd")
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} /W3")

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_AMD64_")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_X86_")
	endif()

	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
	set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
  else()
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG -D_DEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
  endif()

endif()

#if(WIN32)
	#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE -D_UNICODE")
	#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_WIN32_WINNT=0x0501")
	#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS")
	#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWIN32_LEAN_AND_MEAN")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  #set(CMAKE_LDFLAGS "${CMAKE_LDFLAGS} /INCREMENTAL:NO /SAFESEH:NO")
#endif()

# set up include-directories
include_directories(
  "${PROJECT_SOURCE_DIR}"   # to find foo/foo.h
  "${PROJECT_BINARY_DIR}"   # to find foo/config.h
	"${FFMPEG_ROOT_DIR}/include"
	"${LIVE555_ROOT_DIR}/liveMedia/include"
	"${LIVE555_ROOT_DIR}/groupsock/include"
	"${LIVE555_ROOT_DIR}/BasicUsageEnvironment/include"
	"${LIVE555_ROOT_DIR}/UsageEnvironment/include"
	"${SDL_INCLUDE_DIR}"
)
 
if (WIN32)
add_library(libavcodec STATIC IMPORTED)
set_property(TARGET libavcodec PROPERTY IMPORTED_LOCATION ${FFMPEG_ROOT_DIR}/lib/avcodec.lib)
add_library(libavutil STATIC IMPORTED)
set_property(TARGET libavutil PROPERTY IMPORTED_LOCATION ${FFMPEG_ROOT_DIR}/lib/avutil.lib)
add_library(libswscale STATIC IMPORTED)
set_property(TARGET libswscale PROPERTY IMPORTED_LOCATION ${FFMPEG_ROOT_DIR}/lib/swscale.lib)
add_library(libswresample STATIC IMPORTED)
set_property(TARGET libswresample PROPERTY IMPORTED_LOCATION ${FFMPEG_ROOT_DIR}/lib/swresample.lib)

add_library(liblivemedia STATIC IMPORTED)
set_property(TARGET liblivemedia PROPERTY IMPORTED_LOCATION ${LIVE555_ROOT_DIR}/liveMedia/libliveMedia.lib)
add_library(libgroupsock STATIC IMPORTED)
set_property(TARGET libgroupsock PROPERTY IMPORTED_LOCATION ${LIVE555_ROOT_DIR}/groupsock/libgroupsock.lib)
add_library(libBasicUsageEnvironment STATIC IMPORTED)
set_property(TARGET libBasicUsageEnvironment PROPERTY IMPORTED_LOCATION ${LIVE555_ROOT_DIR}/BasicUsageEnvironment/libBasicUsageEnvironment.lib)
add_library(libUsageEnvironment STATIC IMPORTED)
set_property(TARGET libUsageEnvironment PROPERTY IMPORTED_LOCATION ${LIVE555_ROOT_DIR}/UsageEnvironment/libUsageEnvironment.lib)

add_library(libSDL STATIC IMPORTED)
set_property(TARGET libSDL PROPERTY IMPORTED_LOCATION ${SDL_LIB_DIR}/Debug/SDL.lib)
add_library(libSDLmain STATIC IMPORTED)
set_property(TARGET libSDLmain PROPERTY IMPORTED_LOCATION ${SDL_LIB_DIR}/SDLmain/Debug/SDLmain.lib)
endif()

add_executable(rtspclient main.cpp client.cpp mediasink.cpp streamclientstate.cpp client.h mediasink.h streamclientstate.h)
target_link_libraries(rtspclient liblivemedia libgroupsock libBasicUsageEnvironment libUsageEnvironment libavcodec libavutil libswscale libswresample libSDL libSDLmain ws2_32)

# Add sub-directories
#add_subdirectory(foo)
#add_subdirectory(bar)
 
# The interesting stuff goes here
# ===============================
 
# Add all targets to the build-tree export set
export(TARGETS rtspclient
  FILE "${PROJECT_BINARY_DIR}/RtspClientTargets.cmake")
 
# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE rtspclient)
 
# Create the RtspClientConfig.cmake and RtspClientConfigVersion files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
   "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(RtspClientConfig.cmake.in
  "${PROJECT_BINARY_DIR}/RtspClientConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${RTSP_CLIENT_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(RtspClientConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/RtspClientConfig.cmake" @ONLY)
# ... for both
configure_file(RtspClientConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/RtspClientConfigVersion.cmake" @ONLY)
 
# Install the RtspClientConfig.cmake and RtspClientConfigVersion.cmake
install(FILES
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/RtspClientConfig.cmake"
  "${PROJECT_BINARY_DIR}/RtspClientConfigVersion.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
 
# Install the export set for use with the install-tree
#install(EXPORT RtspClientTargets DESTINATION
  #"${INSTALL_CMAKE_DIR}" COMPONENT dev)

OPTION(OPT_BUILD_RELEASE "Release Build." OFF)
