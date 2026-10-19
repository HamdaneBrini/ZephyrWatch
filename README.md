# ZephyrWatch

ZephyrWatch is my personal project to build a real-time activity-tracking smartwatch using Zephyr RTOS on the Waveshare ESP32-S3 Touch LCD 1.28 board.

My goal is to develop useful smartwatch applications while gaining practical experience with multicore processing, sensor acquisition, inter-process communication (IPC), and real-time performance. I want to explore how sensor processing and an interactive interface can run together while keeping timing predictable and the display responsive.

## Current implementation

The current firmware is a single-core prototype running on the ESP32-S3 PRO CPU. It includes:

- a dark LVGL watch face displaying local hours and minutes;
- interrupt-driven QMI8658A accelerometer acquisition with a bounded sample queue;
- an experimental planar X/Y step detector with filtering, hysteresis, and minimum step spacing;
- a step counter on the watch face and a dedicated pedometer screen with touch navigation;
- an application registry and lifecycle manager for switching LVGL screens;
- CST816S touch polling at 20 ms, with controller auto-sleep disabled during startup so touch remains responsive after inactivity;
- PWM backlight control and display blanking support.

The displayed time is currently initialized from a fixed development timestamp and UTC offset in `src/main.c`; automatic synchronization has not yet been implemented. Step-detection thresholds are also experimental and require calibration against recorded motion data.

## Planned applications and features

The watch face and initial pedometer are implemented. The remaining items describe the development roadmap.

1. **Digital watch** — Display the current time and date with a minimal, customizable watch face.
2. **Pedometer and activity tracking** — Use the QMI8658 accelerometer to count steps, estimate walking activity, and track progress toward a daily goal.
3. **Motion detection** — Use accelerometer and gyroscope data to recognize movement, inactivity, and wrist gestures.
4. **Stopwatch and timer** — Implement accurate timing with Zephyr timers and asynchronous event handling.
5. **System performance monitor** — Display runtime metrics such as CPU load, thread activity, and sensor-processing latency to evaluate responsiveness and real-time behavior.

## Technical focus

- Explore distributing work across the ESP32-S3's two cores.
- Acquire and process sensor data at consistent intervals.
- Exchange data and events between tasks using Zephyr IPC and synchronization primitives.
- Measure scheduling behavior, processing latency, and interface responsiveness.

## Development environment setup

These instructions target **Ubuntu 24.04 LTS (x86-64)** with Python 3.12. They follow the [Zephyr getting started guide](https://docs.zephyrproject.org/latest/develop/getting_started/index.html).

### 1. Install system tools

```sh
sudo apt update
sudo apt install --no-install-recommends \
  git cmake ninja-build gperf ccache dfu-util device-tree-compiler \
  wget python3-dev python3-venv python3-pip python3-tk \
  xz-utils file make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 \
  picocom
```

On ARM64 hosts, omit `gcc-multilib` and `g++-multilib`.

### 2. Create a Python environment and install West

```sh
mkdir -p ~/zephyrproject
python3 -m venv ~/zephyrproject/.venv
source ~/zephyrproject/.venv/bin/activate
python -m pip install --upgrade pip
python -m pip install west
```

West manages Zephyr dependencies and provides build and flash commands. Install Python packages inside this environment without `sudo`.

### 3. Get ZephyrWatch and select its manifest

For a **new workspace** (no `~/zephyrproject/.west/` directory), initialize directly from the published application repository:

```sh
west init -m https://github.com/HamdaneBrini/ZephyrWatch.git \
  --mf west.yaml ~/zephyrproject
cd ~/zephyrproject
```

This requires the repository to contain the committed `west.yaml`. Its `self.path` places the application at `apps/ZephyrWatch`.

For an **existing workspace**, clone the application only if it is not already present:

```sh
mkdir -p ~/zephyrproject/apps
git clone https://github.com/HamdaneBrini/ZephyrWatch.git ~/zephyrproject/apps/ZephyrWatch
```

Then select the application manifest instead of running `west init` again:

```sh
cd ~/zephyrproject
west config manifest.path apps/ZephyrWatch
west config manifest.file west.yaml
```

### 4. Download Zephyr and install its dependencies

Run from `~/zephyrproject` after selecting the manifest:

```sh
west manifest --validate
west update
west packages pip --install
west zephyr-export
west blobs fetch hal_espressif
```

The Espressif blobs support radio functionality; fetch them again after dependency updates as described in [Zephyr's Espressif requirements](https://docs.zephyrproject.org/latest/boards/espressif/common/system-requirements.html).

The resulting workspace includes:

```text
~/zephyrproject/
├── .venv/
├── .west/
├── zephyr/
├── modules/
└── apps/
    └── ZephyrWatch/
        └── west.yaml
```

### 5. Install the Zephyr SDK

```sh
cd ~/zephyrproject/zephyr
west sdk install
```

The SDK supplies cross-compilers, including the Xtensa toolchain for ESP32-S3. The pinned Zephyr checkout specifies SDK version **1.0.1**. Installation can download several gigabytes; use `west sdk install --help` for toolchain selection options.

### 6. Allow USB serial access

```sh
sudo usermod -aG dialout "$USER"
```

Log out and back in for group membership to take effect. Connect the board with a USB data cable and find its serial device:

```sh
ls /dev/serial/by-id/
```

Use the corresponding `/dev/ttyACM*` or `/dev/ttyUSB*` port for flashing and monitoring. Close the serial monitor before flashing. Exit picocom with **Ctrl+A, then Ctrl+X**.

### 7. Start a development session

Run this in each new terminal:

```sh
source ~/zephyrproject/.venv/bin/activate
cd ~/zephyrproject/apps/ZephyrWatch
```

Check the installed tools:

```sh
python --version
west --version
cmake --version
ninja --version
picocom --help
```

Optional editor tools: install VS Code with the **clangd** extension and a system `clangd` package (`sudo apt install clangd`). Build once to generate `build/compile_commands.json`; the project's `.clangd` file uses it to resolve includes.

## Zephyr version lock

`west.yaml` pins Zephyr to commit `02a337c21da4944d82c5d8e68ebcf7712e26fdf7`, a **4.5.0-rc1 development snapshot**. A full commit hash selects an exact revision instead of following a moving branch.

`import: true` loads the dependencies declared by that Zephyr revision. Once the workspace uses this application's manifest, `west update` checks out the selected revisions. Commit `west.yaml` alongside the application so other developers use the same version. See [West manifests](https://docs.zephyrproject.org/latest/develop/west/manifest.html).

Verify the active manifest and checkout:

```sh
west manifest --path
git -C ~/zephyrproject/zephyr rev-parse HEAD
```

The manifest path should end in `apps/ZephyrWatch/west.yaml`, and the commit should match `revision` in that file. Editing the manifest alone does not change an existing checkout; run `west update` to apply it.

To upgrade deliberately, change `revision` in `west.yaml` to the desired commit, then run:

```sh
cd ~/zephyrproject
west update
west packages pip --install
west zephyr-export
west blobs fetch hal_espressif
cd zephyr
west sdk install
cd ../apps/ZephyrWatch
make rebuild
```

Test the firmware before committing the version change. The manifest controls source dependencies; Python and system tools still require the setup above.

## Build and run

Requires a configured Zephyr workspace, a compatible toolchain, `west`, `make`, and `picocom` for the serial monitor. Run these commands from the project directory with your Zephyr Python environment activated:

```sh
make build
make flash
make monitor
```

Build before flashing: `make flash` uses the existing firmware in the selected build directory. `make monitor` opens the serial terminal without building or flashing.

### Commands

| Command | Action |
| --- | --- |
| `make build` | Build the firmware and generate `compile_commands.json`. |
| `make rebuild` | Perform a pristine build, clearing the previous build configuration. |
| `make flash` | Flash the existing build to `PORT`. |
| `make monitor` | Open picocom on `PORT` at `BAUDRATE`. |
| `make clean` | Delete the selected build directory. |
| `make help` | List available commands. |

### Settings

Override Makefile settings on the command line:

| Variable | Default | Purpose |
| --- | --- | --- |
| `BOARD` | `esp32s3_touch_lcd_1_28/esp32s3/procpu` | Target board for builds. |
| `BUILD_DIR` | `build` | Build output directory used by build, flash, and clean. |
| `PORT` | `/dev/ttyACM0` | USB serial device for both flashing and monitoring. |
| `BAUDRATE` | `115200` | Serial monitor baud rate. |
| `APP_NAME` | `ZephyrWatch` | Name printed in the build message. |

For example, use a different USB port:

```sh
make flash PORT=/dev/ttyUSB0
make monitor PORT=/dev/ttyUSB0 BAUDRATE=115200
```

To use a separate build directory, pass the same value when building and flashing:

```sh
make build BUILD_DIR=build-watch
make flash BUILD_DIR=build-watch PORT=/dev/ttyACM0
```

The project's clangd configuration expects `build/compile_commands.json`; update its compilation database path if you change `BUILD_DIR`.

## Project layout

- `src/applications/`: LVGL watch applications and their lifecycle callbacks.
- `src/core/`: application registry and screen lifecycle management.
- `src/platform/`: board-facing display and backlight support.
- `src/services/`: time, IMU acquisition, and pedometer services.
- `include/zephyrwatch/`: documented public interfaces for application modules.
- `boards/`: board-specific configuration and devicetree overlays.
- `prj.conf`: Zephyr configuration.
- `Makefile`: build, flash, and serial monitor automation.
- `west.yaml`: pinned Zephyr revision and imported dependencies.

## Code quality checks

The repository uses clang-format through pre-commit. Before submitting changes, run:

```sh
uv sync
uv run pre-commit run --all-files
make rebuild
```

The pre-commit hook formats all C sources and headers according to `.clang-format`. A pristine build verifies the final Kconfig and devicetree configuration.

## Author and contact

ZephyrWatch is designed and developed by **Hamdane BRINI**.

- LinkedIn: [Hamdane BRINI](https://www.linkedin.com/in/hamdane-brini-60b840290/)
