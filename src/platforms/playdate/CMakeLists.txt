cmake_minimum_required(VERSION 3.14)

if(NOT DEFINED SDK)
	if(NOT DEFINED ENV{PLAYDATE_SDK_PATH})
		message(FATAL_ERROR "SDK Path not found; please set PLAYDATE_SDK_PATH environment variable")
	endif()
	set(SDK $ENV{PLAYDATE_SDK_PATH})
endif()

if(NOT EXISTS ${SDK}/C_API/buildsupport/arm.cmake)
	message(FATAL_ERROR "SDK Path invalid: ${SDK}")
endif()

include(${SDK}/C_API/buildsupport/arm.cmake)

set(CMAKE_C_STANDARD 11)

set(PLAYDATE_GAME_NAME "AlphaKinetics")
project(${PLAYDATE_GAME_NAME} C ASM)
set(PLAYDATE_GAME_DEVICE ${PLAYDATE_GAME_NAME})

add_executable(${PLAYDATE_GAME_NAME} ${SDK}/C_API/buildsupport/setup.c playdate_demo.c ../../core/ak_physics.c ../../core/ak_demo_setup.c)
target_include_directories(${PLAYDATE_GAME_NAME} PRIVATE ../../core ${SDK}/C_API)

include(${SDK}/C_API/buildsupport/playdate_game.cmake)
