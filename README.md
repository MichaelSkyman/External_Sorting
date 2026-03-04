# External Sort Visualizer

An interactive Qt6 desktop application that visually demonstrates the **External Merge Sort** algorithm — a technique used to sort datasets that are too large to fit entirely in RAM. Every phase of the algorithm (reading blocks from disk, in-RAM sorting, writing temporary runs, and the k-way merge) is animated in real time so the user can see exactly what is happening at each step.

---

## Table of Contents

1. [Quick Start — No Input File?](#1-quick-start--no-input-file)
2. [What the Application Does](#2-what-the-application-does)
3. [App Pipeline](#3-app-pipeline)
4. [Folder Structure](#4-folder-structure)
5. [Source Code Walkthrough](#5-source-code-walkthrough)
   - [Entry Point](#entry-point--sourcemaincpp)
   - [Base Layer](#base-layer--sourcebase)
   - [App Layer](#app-layer--sourceapp)
   - [GUI — Main Window](#gui--main-window)
   - [GUI — Animation Module](#gui--animation-module)
   - [GUI — Visualization Module](#gui--visualization-module)
   - [GUI — Widgets](#gui--widgets)
6. [Build Instructions](#6-build-instructions)

---

## 1. Quick Start — No Input File?

A ready-to-run package is provided in this repository:

```
External_sort application.rar
```

**This archive contains:**
- The compiled application executable (`external_sort.exe`)
- A sample `input.bin` binary file containing unsorted 32-bit integers

**Steps:**
1. Download **`External_sort application.rar`** from the repository.
2. Extract the archive (use [7-Zip](https://www.7-zip.org/) or WinRAR).
3. Inside the extracted folder, run `external_sort.exe`.
4. On the setup screen, click **Browse Input** and select the included `input.bin`.
5. Click **Browse Output** to choose a folder where `output.bin` will be written.
6. Click **Start Sorting** to begin the animated visualization.

> If you have your own dataset, you can generate a binary file containing raw 32-bit signed integers (little-endian) and use that as input instead.

---

## 2. What the Application Does

External Merge Sort is designed for scenarios where the input file is **larger than available RAM**. The algorithm works in two major stages:

| Stage | What Happens |
|-------|-------------|
| **Split & Sort** | The input file is read in fixed-size chunks. Each chunk is loaded into RAM, sorted in memory, and written back to disk as a temporary *run* file. |
| **Merge** | All temporary run files are merged together using a **min-heap (k-way merge)**, producing a single, fully sorted output file. |

The application animates both stages using a fullscreen canvas that shows:
- The disk → RAM data flow
- Blocks being sorted inside RAM
- Sorted runs accumulating on disk
- The merge heap selecting the minimum at each step
- The final output stream growing in real time

---

## 3. App Pipeline

```
User sets input file + output directory + chunk mode
              │
              ▼
     ┌─────────────────┐
     │   AppConfig      │  Stores paths and chunk size settings
     └────────┬────────┘
              │  getChunkSize()  (AUTO → MemoryDetect, MANUAL → user value)
              ▼
     ┌─────────────────┐
     │  ExternalSorter  │  Core sorting engine (C++ standard library only)
     │                  │
     │  1. SplitandSort │──► Reads input.bin in chunks
     │     ├─ READ cb   │    Emits READ  callback → AnimationController
     │     ├─ std::sort │    Emits SORT  callback → AnimationController
     │     └─ WRITE cb  │    Emits WRITE callback → AnimationController
     │                  │
     │  2. mergeSorted  │──► k-way heap merge of all run files
     │     └─ MERGE cb  │    Emits MERGE callback → AnimationController
     └────────┬────────┘
              │ SortCallback (phase, data, progress)
              ▼
     ┌──────────────────────┐
     │  StepAggregator       │  Batches raw operations into visual steps
     └──────────┬───────────┘
                │ AggregatedStep queue
                ▼
     ┌──────────────────────┐
     │  AnimationEngine      │  60 fps fixed-timestep loop (QTimer)
     └──────────┬───────────┘
                │ frame updates (progress 0→1 per step)
                ▼
     ┌──────────────────────┐
     │  ExternalSortCanvas   │  QGraphicsView retained-renderer
     │  ┌──────────────────┐ │
     │  │ BlockItem objects│ │  Individual animated data blocks
     │  └──────────────────┘ │
     │  ┌──────────────────┐ │
     │  │ CanvasLayout      │ │  Disk / RAM / Runs / Output regions
     │  └──────────────────┘ │
     └──────────┬───────────┘
                │ rendered inside
                ▼
     ┌──────────────────────┐
     │  FullscreenVisualizer │  QWidget: canvas + control bar + progress bar
     └──────────────────────┘
                │ displayed in
                ▼
     ┌──────────────────────┐
     │  MainWindow           │  QMainWindow with Setup page / Animation page
     └──────────────────────┘
                │ writes
                ▼
           output.bin  (32-bit int, little-endian, via BinaryStreamWriter)
```

---

## 4. Folder Structure

```
External_Sorting/
├── CMakeLists.txt                  # CMake build definition
├── README.md                       # This file
├── External_sort application/      # Pre-built application folder
├── External_sort application.rar   # ★ Download this if you need the app + sample input
│
├── Resources/                      # Design-time UI assets
│   ├── css.qss                     # Global Qt stylesheet
│   └── ui_main.ui                  # Qt Designer UI file
│
└── source/                         # All C++ source code
    ├── main.cpp                    # Application entry point
    │
    ├── base/                       # Core algorithm (no Qt dependency)
    │   ├── external_sort.h/.cpp    # ExternalSorter class
    │   └── binary_stream_writer.h  # Output file writer (32-bit, little-endian)
    │
    ├── app/                        # Application-level configuration
    │   ├── config.h/.cpp           # AppConfig: paths + chunk mode
    │   └── memory_detect.h/.cpp    # Available RAM query + chunk size heuristic
    │
    └── gui/                        # Qt GUI layer
        ├── main_window.h/.cpp      # MainWindow (setup page / animation page)
        ├── mainwindow.ui           # Qt Designer UI definition
        ├── resources.qrc           # Qt resource bundle (stylesheet, images)
        │
        ├── animation/              # Step sequencing and playback engine
        │   ├── animation_step.h    # StepType enum + AnimationStep struct
        │   ├── animation_controller.h/.cpp  # Step queue + playback control
        │   ├── animation_timer.h   # High-precision delta-time timer
        │   ├── animation_clock.h   # Elapsed-time clock utility
        │   ├── animation_manager.h # Coordinator between sorter and controller
        │   └── step_aggregator.h   # Batches raw ops into visual steps
        │
        ├── visualization/          # QGraphicsView canvas and block rendering
        │   ├── external_sort_canvas.h/.cpp  # Main animated canvas (QGraphicsView)
        │   ├── fullscreen_visualizer.h      # Canvas wrapper with control bar
        │   ├── sort_visualizer.h/.cpp       # Legacy QPainter-based visualizer
        │   ├── memory_block.h      # MemoryBlock data + state model
        │   └── moving_block.h      # Flying block animations (MovingBlock / Group)
        │
        └── widgets/                # Supplementary HUD widgets
            ├── timeline_scrubber.h/.cpp   # Seek bar with phase markers
            ├── merge_heap_widget.h/.cpp   # Animated binary min-heap display
            └── disk_io_animator.h/.cpp    # Disk ↔ RAM data-transfer animation
```

---

## 5. Source Code Walkthrough

### Entry Point — `source/main.cpp`

The application's `main()` function creates a `QApplication`, loads the global stylesheet from the embedded Qt resource bundle (`:/assets/style/css.qss`), constructs a `MainWindow`, and enters the Qt event loop. No sorting logic lives here.

---

### Base Layer — `source/base/`

#### `external_sort.h` / `external_sort.cpp` — `ExternalSorter`

The pure C++ sorting engine. It has **no Qt dependency** and no knowledge of the GUI.

| Method | Responsibility |
|--------|---------------|
| `ExternalSorter(chunkSize)` | Sets how many `double` elements fit in one in-memory chunk. |
| `sort(inputFile, outputFile)` | Public entry point. Calls `SplitandSort` then `mergeSortedChunks`. |
| `SplitandSort(inputFile)` | Reads the input in `chunkSize` blocks, sorts each in RAM with `std::sort`, writes temporary `chunk_N.bin` files, returns those filenames. |
| `mergeSortedChunks(chunks, output)` | Opens all chunk files simultaneously and merges them using a **min-heap priority queue**, writing every minimum element to the output file one at a time. |
| `notifyCallback(phase, data, progress)` | Fires the `SortCallback` with a capped 20-element preview, keeping the GUI responsive without sending millions of values. |

The `SortCallback` (`std::function<void(SortPhase, vector<double>, int)>`) is the only coupling between the algorithm and the GUI. `MainWindow::startSorting()` installs the callback before calling `sort()`.

#### `binary_stream_writer.h` — `BinaryStreamWriter`

A lightweight RAII file writer that streams **32-bit signed integers (little-endian)** to a binary output file. Used at the end of the sort to write the final `output.bin`.

- `write(int32_t)` — writes a single value.
- `writeFromDouble(double)` — truncates to `int32_t` and writes.
- `resolveOutputPath(dir)` — static helper that appends `output.bin` to the chosen directory.

---

### App Layer — `source/app/`

#### `config.h` / `config.cpp` — `AppConfig`

Stores the three user-configurable parameters for a sorting session:

| Field | Description |
|-------|-------------|
| `inputFile` | Path to the binary `.bin` input file. |
| `outputDir` | Directory where `output.bin` is written. |
| `chunkMode` | `AUTO` (derived from RAM) or `MANUAL` (user input). |
| `manualChunkSize` | Used only when `chunkMode == MANUAL`. |

`getChunkSize()` is the smart accessor: in `AUTO` mode it asks `MemoryDetect::getRecommendedChunkSize()`, in `MANUAL` mode it returns the user value.

#### `memory_detect.h` / `memory_detect.cpp` — `MemoryDetect`

A small namespace with two platform-specific functions:

- `getAvailableRAM()` — queries the OS for free physical memory bytes (uses Windows `GlobalMemoryStatusEx`).
- `getRecommendedChunkSize()` — returns `getAvailableRAM() / sizeof(double) / 4`, using roughly 25% of available RAM to leave room for the OS and Qt itself.

---

### GUI — Main Window

#### `main_window.h` / `main_window.cpp` — `MainWindow`

The top-level `QMainWindow`. Its UI is split into two pages managed by a `QStackedWidget`:

1. **Setup Page** — input file path, output directory, chunk mode selector (Auto / Manual), chunk size spin-box, and a *Start Sorting* button. The `browseInput()` / `browseOutput()` slots open file/directory pickers.

2. **Animation Page** — contains the `FullscreenVisualizer` and a toolbar with playback controls (play/pause, step forward/backward, speed slider, back button). `showAnimationPage()` switches the stack to this view.

`startSorting()` is the pivotal slot:
1. Reads `AppConfig` to get the input path, output directory, and chunk size.
2. Constructs an `ExternalSorter` and installs a `SortCallback`.
3. Runs the sort on a background `QThread` to keep the UI responsive.
4. The callback posts `AggregatedStep` objects onto the `StepAggregator` queue.
5. On completion, switches to the animation page.

---

### GUI — Animation Module

#### `animation_step.h` — `StepType` + `AnimationStep`

Defines the **vocabulary** of the animation pipeline. A `StepType` enum lists every visual event (`IntroDisk`, `LoadToRAM`, `SortBlock`, `WriteToRun`, `MergeStep`, `WriteOutput`, `PhaseTransition`, etc.). An `AnimationStep` struct carries the data for one such event: which block indices are involved, their values, source/target positions, duration, and status text. Factory static methods (`createLoadToRAM`, `createSortBlock`, …) simplify construction.

#### `animation_controller.h` / `animation_controller.cpp` — `AnimationController`

Owns the step queue and drives playback:

- Maintains a `QQueue<AnimationStep>` and a `QTimer` that advances one step at a time.
- Emits `stepStarted`, `stepProgress(double 0–1)`, and `stepCompleted` signals each frame.
- Implements **seek / rewind** via a circular `AnimationState` history buffer.
- Uses a `SpeedProfile` (intro → early → normal → fast pacing) so early steps animate slowly for comprehension and later steps run quickly.

#### `step_aggregator.h` — `StepAggregator`

The sorting thread emits hundreds of thousands of raw `RawOperation` events (READ, COMPARE, SWAP, MERGE_WRITE…). Rendering every one would freeze the GPU and obscure the educational message. `StepAggregator` **batches** these into a much smaller set of `AggregatedStep` objects:

- Up to 50 load operations → one `LoadChunk` visual step.
- Up to 100 comparisons → one `SortInRAM` visual step.
- Up to 20 merge operations → one `MergeStep` visual step.

`AggregationConfig` lets the batch sizes and per-step durations be tuned without recompiling.

#### `animation_timer.h` — `AnimationTimer`

A `QObject` wrapper around `QElapsedTimer` that emits `frameReady(deltaMs, stepProgress)` at ~60 fps. Keeps frame-timing logic out of the canvas.

#### `animation_manager.h` — `AnimationManager`

Coordinates the handoff between `ExternalSorter`'s callback, the `StepAggregator`, and the `AnimationController`. Acts as a façade so `MainWindow` only needs to talk to one object.

---

### GUI — Visualization Module

#### `external_sort_canvas.h` / `external_sort_canvas.cpp` — `ExternalSortCanvas`

The primary rendering surface — a `QGraphicsView` containing a `QGraphicsScene`. The canvas is divided into four named regions by a `CanvasLayout` struct:

| Region | Contents |
|--------|----------|
| **Disk** | Source data blocks waiting to be loaded. |
| **RAM** | Blocks currently in memory; animated sort happens here. |
| **Runs** | Sorted temporary run files on disk. |
| **Output** | The final sorted output stream (sliding window). |

Each data value is a `BlockItem` (`QGraphicsObject`) with three animated Q_PROPERTYs (`blockOpacity`, `blockScale`, `glowIntensity`) that are driven by `QPropertyAnimation`. Because Qt only repaints dirty items, performance scales well even with many blocks on screen.

`AnimationEngine` (nested class) runs a 60 fps `QTimer`. Each tick calls the appropriate `executeXxx(step, progress)` method (e.g., `executeLoadChunk`, `executeSortInRAM`, `executeMergeStep`) which repositions, fades, or scales `BlockItem` objects via property animations.

The background (`drawBackground`) paints cached section labels and run pills. The foreground (`drawForeground`) paints the phase-transition overlay text.

#### `fullscreen_visualizer.h` — `FullscreenVisualizer`

A thin `QWidget` wrapper that stacks:
1. A `QProgressBar` at the top.
2. A collapsible control bar (`◀ ⏸ ▶ | Speed slider | Step counter`) that auto-hides after 3 seconds of inactivity using a `QTimer` + `QPropertyAnimation` opacity fade.
3. The `ExternalSortCanvas` filling all remaining space.

Owns its own `AnimationEngine` and `StepAggregator` instances and exposes `play()`, `pause()`, `stop()`, `stepForward()`, `stepBackward()`, and `setSpeed()`.

#### `sort_visualizer.h` / `sort_visualizer.cpp` — `SortVisualizer` *(legacy)*

The original `QWidget` + `QPainter` visualizer kept for reference. Renders the same six regions using manual `QPainter` calls instead of a `QGraphicsScene`. Supports zoom, virtualized rendering via the `Viewport` struct, and smooth color transitions via `ColorTransition`. Superseded by `ExternalSortCanvas` but still compiled into the project.

#### `memory_block.h` — `MemoryBlock`

A plain data struct representing a single element displayed on the canvas. Tracks value, screen position, `BlockState` (Idle, Active, Sorted, Moving, Merging, Output…), and per-block animation progress. `baseColor()` maps `BlockState` to a fill color; `render()` draws it with a `QPainter`.

#### `moving_block.h` — `MovingBlock` / `MovingBlockGroup`

Represents a **flying block** transitioning between two regions (e.g., disk → RAM). Supports linear motion, Bézier curve paths, fade, and scale animations. `MovingBlockGroup` manages a collection of `MovingBlock` objects and provides helpers like `offsetAll()`, `arrangeHorizontally()`, and `arrangeGrid()`.

---

### GUI — Widgets

#### `timeline_scrubber.h` / `.cpp` — `TimelineScrubber`

A custom `QWidget` seek bar. Renders a filled progress track with per-phase color segments (marked by `PhaseMarker` structs) and a draggable handle. Emits `seekRequested(int step)` when the user scrubs to a new position.

#### `merge_heap_widget.h` / `.cpp` — `MergeHeapWidget`

Displays the binary min-heap used during the merge phase as an interactive tree diagram. Each `HeapNode` shows its run index (color-coded) and current value. `extractMin()` drives a `QPropertyAnimation` that visually lifts the root node off the tree before it moves to the output region.

#### `disk_io_animator.h` / `.cpp` — `DiskIOAnimator`

An overlay widget that animates the latency of a disk I/O operation — a moving block traveling from the disk position to the RAM (or runs) position with a configurable arc and duration. Emits `operationCompleted` when the animation finishes so the canvas can continue.

---

## 6. Build Instructions

**Prerequisites:**
- Qt 6.x (Widgets module)
- CMake ≥ 3.16
- A C++17-capable compiler (MinGW 64-bit, MSVC 2019+, or GCC/Clang)

```bash
# 1. Clone the repository
git clone <repo-url>
cd External_Sorting

# 2. Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build build --config Release

# 4. Run
./build/bin/external_sort        # Linux / macOS
build\bin\external_sort.exe      # Windows
```

> **Qt Creator users:** open `CMakeLists.txt` directly. Qt Creator will configure and build automatically using your installed Qt kit.

### Input File Format

The input must be a flat binary file of **64-bit IEEE 754 doubles** (the same format `ExternalSorter` reads). To generate your own test file you can use any language:

```python
import struct, random
with open("input.bin", "wb") as f:
    for _ in range(1_000_000):
        f.write(struct.pack("<d", random.uniform(0, 1e6)))
```

The output (`output.bin`) is written as **32-bit signed integers (little-endian)** via `BinaryStreamWriter`.
