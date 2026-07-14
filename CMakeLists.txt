cmake_minimum_required(VERSION 3.16)
project(HammingAwgnQt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

set(SOURCES
    src/main.cpp
    # Core
    src/core/Hamming74.cpp
    src/core/Bpsk.cpp
    src/core/AwgnChannel.cpp
    src/core/BerEvaluator.cpp
    src/core/Simulator.cpp
    # GUI
    src/gui/PlotWidget.cpp
    src/gui/SimWorker.cpp
    src/gui/MainWindow.cpp
)

set(HEADERS
    src/core/Types.h
    src/core/Hamming74.h
    src/core/Bpsk.h
    src/core/AwgnChannel.h
    src/core/BerEvaluator.h
    src/core/SimConfig.h
    src/core/SimResult.h
    src/core/Simulator.h
    src/gui/PlotWidget.h
    src/gui/SimWorker.h
    src/gui/MainWindow.h
)

add_executable(HammingAwgnQt ${SOURCES} ${HEADERS})

target_include_directories(HammingAwgnQt PRIVATE src)
target_link_libraries(HammingAwgnQt PRIVATE Qt6::Widgets)
