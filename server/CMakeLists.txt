cmake_minimum_required (VERSION 3.0)

set (CMAKE_CXX_STANDARD 11)

project(mkart)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

##########
# ASIO
set(ASIO_PATH ./3rdparty/asio/asio)

file(GLOB ASIO_SRC_FILES ${ASIO_PATH}/src/asio.cpp)
#message(STATUS ${ASIO_SRC_FILES})

set(ASIO_INCLUDE_PATHS ${ASIO_PATH}/include)
set(ASIO_LIB_PATHS ${ASIO_PATH})

##########
# PIGPIO
set(PIGPIO_PATH ./3rdparty/pigpio)

set(PIGPIO_SRC_FILES ${PIGPIO_PATH}/pigpio.c ${PIGPIO_PATH}/command.c)
#message(STATUS ${PIGPIO_SRC_FILES})

set(PIGPIO_INCLUDE_PATHS ${PIGPIO_PATH})

##########
# DUKGLUE
set(DUKGLUE_PATH ./3rdparty/dukglue)

set(DUKGLUE_INCLUDE_PATHS ${DUKGLUE_PATH}/include)

##########
# LOGURU
set(LOGURU_PATH ./3rdparty/loguru)

set(LOGURU_INCLUDE_PATHS ${LOGURU_PATH})


##########
# Application

file(GLOB APP_SRC_FILES src/*.cpp src/*.c)

include_directories(src ${ASIO_INCLUDE_PATHS} ${PIGPIO_INCLUDE_PATHS} ${DUKGLUE_INCLUDE_PATHS} ${LOGURU_INCLUDE_PATHS})
link_directories(${ASIO_LIB_PATHS})

add_definitions(-DASIO_STANDALONE)

add_executable(v4l2-server ${APP_SRC_FILES} ${PIGPIO_SRC_FILES})

target_link_libraries(v4l2-server pthread dl)