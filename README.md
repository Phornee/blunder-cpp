# Blunder build prerequisites

Blunder is a CPU ray tracer. Its camera, lights, shadows, and geometric
intersections are implemented in this project and `geom3`; SDL3 remains
responsible for displaying the rendered pixel buffer.

Blunder is built with GNU C++ (C++20) and GNU Make. The Makefile also uses
`ar`, which is included with the GNU C++ toolchain.

## Install

### Windows (MSYS2)

1. Install MSYS2 from [msys2.org](https://www.msys2.org/).
2. Open the **MSYS2 UCRT64** terminal from the Start menu.
3. Update the package database and install the compiler and Make:

   ```sh
   pacman -Syu
   ```

   If the update asks you to close the terminal, reopen **MSYS2 UCRT64**, then
   run:

   ```sh
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb make
   ```

4. Confirm both commands are available:

   ```sh
   g++ --version
   make --version
   ```

Use the MSYS2 UCRT64 terminal for building so the compiler and Make are on
`PATH`.

Install SDL3 and `pkg-config` from the same terminal:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-pkgconf
```

### Debian or Ubuntu

Install the GNU compiler, Make, SDL3 development files, and `pkg-config` with
the distribution's package manager (the SDL3 package may depend on the
distribution release):

```sh
sudo apt update
sudo apt install build-essential gdb pkg-config libsdl3-dev
```

`build-essential` includes `g++`, `make`, and the required archiver.

### macOS

Install Apple's command-line developer tools, which provide a C++ compiler and
Make, and SDL3 with Homebrew:

```sh
xcode-select --install
brew install sdl3 pkg-config
```

The `g++` command provided by Apple's tools is an alias for Clang.
GDB is not included; install it separately if you want to use the GDB
pretty-printer below.

## Build

From this directory, run:

```sh
make
```

The default release build is written to `build/release/Blunder.exe`. For a
debug build, run `make debug`.

## Run Blunder

With SDL3 and `pkg-config` installed and available on `PATH`, launch the app
from this directory:

```sh
./build/release/Blunder.exe
```

The app renders the default scene in a resizable SDL3 window. Press **D** to
rerender the pixel under the mouse, or press **Escape** or close the window to
exit. The HUD displays the frame rate and logical pixel coordinates.

Run the ray-tracing smoke tests with:

```sh
make test
```


```sh
make debug
```

Open this directory as the VS Code workspace, select **Blunder (GDB)** in Run
and Debug, and start debugging. The checked-in `.vscode/launch.json` loads the
printer automatically. Ensure `gdb` is installed and available on VS Code's
`PATH`; if it is not, set `miDebuggerPath` in that launch configuration to the
full path to `gdb.exe`.

If you use the parent `Code` folder as your VS Code workspace, select its
**Blunder (GDB)** configuration instead.

For SDL3 IntelliSense, select **Blunder (MSYS2 UCRT64)** in the C/C++ status
bar configuration list. This configuration points to the MSYS2 UCRT64 compiler
and SDL3 headers. It is available whether you open this directory or the parent
`Code` folder as your workspace.

If you already have a launch configuration, add the printer's `source`
command to its `setupCommands` and ensure its debugger type is `cppdbg` with
`MIMode` set to `gdb`.
