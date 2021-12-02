#########################################
#
# Author: 
#   Ahmed Hamza     <aah10@sfu.ca>
#
#########################################

cmake_minimum_required(VERSION 2.8.8)

#enable debug mode
SET(CMAKE_BUILD_TYPE "Debug") 
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_FLAGS "-std=c++11 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS")  # Second macro definition is required to fix the error: ‘UINT32_MAX’ was not declared in this scope

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the required Qt5 libraries
find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Multimedia)
find_package(Qt5MultimediaWidgets)
find_package(Qt5OpenGL)
find_package(PkgConfig REQUIRED)
# Find libavcodec
find_package(LibAV REQUIRED)

pkg_search_module(NDN REQUIRED libndn-cxx)

set(LIBDASH_LIBRARIES "../build/bin/" CACHE PATH "Path to libdash.so")
set(LIBDASH_INCLUDES "../libdash/include/" CACHE PATH "Path to libdash includes")

include_directories(${LIBDASH_INCLUDES} ${NDN_INCLUDE_DIRS})
link_directories(${LIBDASH_LIBRARIES})

file(GLOB qtplayer_SOURCE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} Decoder/*.cpp libdashframework/MPD/*.cpp libdashframework/Adaptation/*.cpp libdashframework/Buffer/*.cpp libdashframework/Input/*.cpp libdashframework/Portable/*.cpp Managers/*.cpp Renderer/*.cpp UI/*.cpp UI/IDASHPlayerGuiObserver.h main.cpp)

qt5_wrap_ui(qtsampleplayer_FORMS_HEADERS qtsampleplayer.ui)
qt5_add_resources(qtsampleplayer_RESOURCES_RCC qtsampleplayer.qrc)

# Tell CMake to create the helloworld executable
add_executable(qtsampleplayer ${qtplayer_SOURCE} ${qtsampleplayer_FORMS_HEADERS} ${qtsampleplayer_RESOURCES_RCC})

# Use the required module from Qt5.
qt5_use_modules(qtsampleplayer Core Widgets Multimedia MultimediaWidgets Gui OpenGL)

# For the linker
target_link_libraries(qtsampleplayer dash ${QT_LIBRARIES} ${LIBAV_LIBRARIES} ${NDN_LIBRARIES} -lpthread)
