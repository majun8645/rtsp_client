cmake_minimum_required(VERSION 2.8)
project(RtspClient CXX)
 
set(RTSP_CLIENT_MAJOR_VERSION 0)
set(RTSP_CLIENT_MINOR_VERSION 1)
set(RTSP_CLIENT_PATCH_VERSION 0)
set(RTSP_CLIENT_VERSION ${RTSP_CLIENT_MAJOR_VERSION}.${RTSP_CLIENT_MINOR_VERSION}.${RTSP_CLIENT_PATCH_VERSION})
 
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
 
# set up include-directories
include_directories(
  "${PROJECT_SOURCE_DIR}"   # to find foo/foo.h
  "${PROJECT_BINARY_DIR}"   # to find foo/config.h
	"../ffmpeg-20140919-git-33c752b-win64-dev/include"
	"../live555/liveMedia/include"
	"../live555/groupsock/include"
	"../live555/BasicUsageEnvironment/include"
	"../live555/UsageEnvironment/include"
)
 
if (WIN32)
add_library(libavcodec STATIC IMPORTED)
set_property(TARGET libavcodec PROPERTY IMPORTED_LOCATION ../ffmpeg-20140919-git-33c752b-win64-dev/lib/avcodec.lib)
add_library(libavutil STATIC IMPORTED)
set_property(TARGET libavutil PROPERTY IMPORTED_LOCATION ../ffmpeg-20140919-git-33c752b-win64-dev/lib/avutil.lib)
add_library(libswscale STATIC IMPORTED)
set_property(TARGET libswscale PROPERTY IMPORTED_LOCATION ../ffmpeg-20140919-git-33c752b-win64-dev/lib/swscale.lib)
add_library(libresample STATIC IMPORTED)
set_property(TARGET libresample PROPERTY IMPORTED_LOCATION ../ffmpeg-20140919-git-33c752b-win64-dev/lib/resample.lib)

add_library(liblivemedia STATIC IMPORTED)
set_property(TARGET liblivemedia PROPERTY IMPORTED_LOCATION ../live555/liveMedia/libliveMedia.lib)
add_library(libgroupsock STATIC IMPORTED)
set_property(TARGET libgroupsock PROPERTY IMPORTED_LOCATION ../live555/groupsock/libgroupsock.lib)
add_library(libBasicUsageEnvironment STATIC IMPORTED)
set_property(TARGET libBasicUsageEnvironment PROPERTY IMPORTED_LOCATION ../live555/BasicUsageEnvironment/libBasicUsageEnvironment.lib)
add_library(libUsageEnvironment STATIC IMPORTED)
set_property(TARGET libUsageEnvironment PROPERTY IMPORTED_LOCATION ../live555/UsageEnvironment/libUsageEnvironment.lib)
endif()

add_executable(rtspclient main.cpp client.cpp mediasink.cpp streamclientstate.cpp client.h mediasink.h streamclientstate.h)
target_link_libraries(rtspclient liblivemedia libgroupsock libBasicEnvironmentUsage libUsageEnvironment libavcodec libavutil libswscale libresample)

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
install(EXPORT RtspClientTargets DESTINATION
  "${INSTALL_CMAKE_DIR}" COMPONENT dev)
