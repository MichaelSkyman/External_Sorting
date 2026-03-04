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
    source/base/binary_stream_writer.h
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
    source/gui/resources.qrc
    
    # Animation module
    source/gui/animation/animation_step.h
    source/gui/animation/animation_timer.h
    source/gui/animation/animation_controller.h
    source/gui/animation/animation_controller.cpp
    source/gui/animation/animation_clock.h
    source/gui/animation/step_aggregator.h
    
    # Visualization module
    source/gui/visualization/memory_block.h
    source/gui/visualization/moving_block.h
    source/gui/visualization/sort_visualizer.h
    source/gui/visualization/sort_visualizer.cpp
    source/gui/visualization/external_sort_canvas.h
    source/gui/visualization/external_sort_canvas.cpp
    source/gui/visualization/fullscreen_visualizer.h
    
    # Widgets module
    source/gui/widgets/timeline_scrubber.h
    source/gui/widgets/timeline_scrubber.cpp
    source/gui/widgets/merge_heap_widget.h
    source/gui/widgets/merge_heap_widget.cpp
    source/gui/widgets/disk_io_animator.h
    source/gui/widgets/disk_io_animator.cpp
)

target_link_libraries(external_sort PRIVATE
    Qt6::Widgets
    app_lib
)

target_include_directories(external_sort PRIVATE
    ${CMAKE_SOURCE_DIR}/source/gui
    ${CMAKE_SOURCE_DIR}/source/gui/animation
    ${CMAKE_SOURCE_DIR}/source/gui/visualization
    ${CMAKE_SOURCE_DIR}/source/gui/widgets
    ${CMAKE_SOURCE_DIR}/source
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
