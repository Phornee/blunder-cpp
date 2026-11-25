# Blunder build prerequisites

Blunder is a CPU ray tracer. Its camera, lights, shadows, and geometric
intersections are implemented in this project and `geom3`; SDL3 displays the
rendered pixel buffer, and SDL3_image loads PNG and JPEG textures.

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

Install SDL3, SDL3_image, and `pkg-config` from the same terminal:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-sdl3 mingw-w64-ucrt-x86_64-sdl3-image mingw-w64-ucrt-x86_64-pkgconf
```

### Debian or Ubuntu

Install the GNU compiler, Make, SDL3 and SDL3_image development files, and `pkg-config` with
the distribution's package manager (the SDL3 package may depend on the
distribution release):

```sh
sudo apt update
sudo apt install build-essential gdb pkg-config libsdl3-dev libsdl3-image-dev
```

`build-essential` includes `g++`, `make`, and the required archiver.

### macOS

Install Apple's command-line developer tools, which provide a C++ compiler and
Make, SDL3, and SDL3_image with Homebrew:

```sh
xcode-select --install
brew install sdl3 sdl3_image pkg-config
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

To run Blunder with its timing output in VS Code's integrated terminal, open
**Tasks: Run Task** from the Command Palette and choose **Run Blunder (Release)
in Terminal**. Choose the Debug task to build and run the debug configuration.

## Mesh files

`MeshObject` accepts the existing YAML mesh format and Wavefront OBJ files.
OBJ import reads vertex positions and faces, supports slash-separated and
negative vertex indices, and triangulates polygon faces. It does not import
OBJ materials, texture coordinates, or normals; triangulate meshes in Blender
before export for predictable results.

`AABB` provides inclusive point containment, mutable bounds, and forward ray
intersection using `Line3` parameter values, including a boolean `intersects`
query. It can calculate its bounds from a list of vertices; `MeshObject` uses
transformed mesh vertices for its AABB. If a ray starts inside the box, its
intersection is the box's exit point.

## Run Blunder

With SDL3, SDL3_image, and `pkg-config` installed and available on `PATH`, launch the app
from this directory:

```sh
./build/release/Blunder.exe
```

On Windows, when launching from PowerShell or Command Prompt, add the MSYS2
UCRT64 runtime directory to `PATH` so SDL's DLLs can be found. For the default
MSYS2 installation, PowerShell commands are:

```powershell
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
.\build\release\Blunder.exe
```

The app renders the default scene, including the repeating PNG floor texture in
`assets/floor.png`, in a resizable SDL3 window. Run it from this directory so
the texture path resolves. Press **D** to
rerender the pixel under the mouse, or press **Escape** or close the window to
exit. The HUD displays the frame rate and logical pixel coordinates.
While rendering, the integrated terminal shows one scanline progress line that
updates in place, followed by the render timings.

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

To run Blunder tests under the debugger, select **Blunder Tests (GDB - Debug)**
in Run and Debug. VS Code builds the test executable before launching it.

If you use the parent `Code` folder as your VS Code workspace, select its
**Blunder (GDB)** configuration instead.

For SDL3 IntelliSense, select **Blunder (MSYS2 UCRT64)** in the C/C++ status
bar configuration list. This configuration points to the MSYS2 UCRT64 compiler
and SDL3 headers. It is available whether you open this directory or the parent
`Code` folder as your workspace.

If you already have a launch configuration, add the printer's `source`
command to its `setupCommands` and ensure its debugger type is `cppdbg` with
`MIMode` set to `gdb`.
