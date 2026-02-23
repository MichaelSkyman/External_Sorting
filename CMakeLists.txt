cmake_minimum_required(VERSION 3.16)
project(ExternalSortingDemo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ================= Base Library =================
add_library(base_lib STATIC
    source/base/external_sort.cpp
    source/base/external_sort.h
)
target_include_directories(base_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/source/base
)

# ================= App Library =================
add_library(app_lib STATIC
    source/app/config.cpp
    source/app/memory_detect.cpp
)
target_include_directories(app_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/source/app
)
target_link_libraries(app_lib PUBLIC base_lib)

# ================= Qt =================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

# ================= GUI Executable =================
add_executable(external_sort
    source/main.cpp
    source/gui/main_window.cpp
    source/gui/main_window.h
    source/gui/mainwindow.ui
)

target_link_libraries(external_sort PRIVATE
    Qt6::Widgets
    app_lib
)

# ================= Tool =================
add_executable(read_inp_outp
    source/read_inp_outp.cpp
)

target_link_libraries(read_inp_outp PRIVATE base_lib)

# ================= Warnings =================
if(MSVC)
    target_compile_options(external_sort PRIVATE /W4)
    target_compile_options(read_inp_outp PRIVATE /W4)
else()
    target_compile_options(external_sort PRIVATE -Wall -Wextra -g3)
    target_compile_options(read_inp_outp PRIVATE -Wall -Wextra -g3)
endif()

# ================= Install =================
install(TARGETS external_sort read_inp_outp
    RUNTIME DESTINATION bin
)