cmake_minimum_required(VERSION 3.16)
project(NankaiLoop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(GAME_CORE_SOURCES
    src/gamestate.cpp
    src/gamestate.h
    src/scenes.cpp
    src/scenes.h
    src/gameview.h
)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/audio.cpp
    src/audio.h
    ${GAME_CORE_SOURCES}
    resources.qrc
)

add_executable(NankaiLoop WIN32 ${SOURCES})
target_link_libraries(NankaiLoop PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
if (WIN32)
    target_link_libraries(NankaiLoop PRIVATE winmm)
endif()
# target_compile_options(NankaiLoop PRIVATE -Wall -Wno-unused-parameter)

# Headless test driver — 用于自动化剧情测试
# add_executable(NankaiTest
#     src/test_main.cpp
#     src/headless.cpp
#     src/headless.h
#     ${GAME_CORE_SOURCES}
# )
# target_link_libraries(NankaiTest PRIVATE Qt6::Core)
# target_compile_options(NankaiTest PRIVATE -Wall -Wno-unused-parameter)
