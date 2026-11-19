# system-R

Open-source lighting console.

The goal of this project is to develop an open source software system for DIY lighting
control desks with special regard to touch-based UI navigation and custom hardware controls
for programming and playback — a grandMA3-style console built on the **JUCE 9** framework and
runnable on Linux, macOS and Windows.

See [docs/IMPLEMENTATION_PLAN.md](docs/IMPLEMENTATION_PLAN.md) for the architecture and roadmap.

## Building

**Prerequisites:** CMake ≥ 3.24, a C++20 compiler (MSVC 2022 / AppleClang / GCC ≥ 12 or
Clang), and Git. JUCE 9 and Catch2 are fetched automatically at configure time.

On Linux, install the JUCE GUI dependencies first:

```sh
sudo apt-get install -y \
  libasound2-dev libjack-jackd2-dev libfreetype-dev libfontconfig1-dev libgtk-3-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev \
  libxrandr-dev libxrender-dev libglu1-mesa-dev mesa-common-dev
```

Configure, build and test:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo    # or: cmake --preset default
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

JUCE writes the application into its artefacts folder:
`build/app/systemr_app_artefacts/<config>/` — e.g. `system-R.exe` (Windows), `system-R`
(Linux), or `system-R.app` (macOS). Test binaries go to `build/bin/`.

## Licensing

First-party source in this repository is released into the public domain (see `LICENSE`).
Distributed builds link JUCE 9 under its **AGPLv3** option, so binary releases are governed
by the AGPLv3; see the licensing note in the implementation plan for details.
