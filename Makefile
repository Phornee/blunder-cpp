# GNU C++ compiler and recursive Make command.
CXX := g++
MAKE := make
PKG_CONFIG ?= pkg-config

# Release is the default; callers can select CONFIG=debug or change parallelism with JOBS=n.
CONFIG ?= release
JOBS ?= 4

# Recursively find matching files in this project and its nested directories.
rwildcard = $(wildcard $(1)$(2)) $(foreach dir,$(wildcard $(1)*),$(if $(wildcard $(dir)/.),$(call rwildcard,$(dir)/,$(2))))

SOURCES := $(wildcard src/*.cpp)
HEADERS := $(call rwildcard,include/,*.hpp) $(call rwildcard,../geom3/include/,*.hpp)
GEOM_SOURCES := $(call rwildcard,../geom3/src/,*.cpp)
GEOM_HEADERS := $(call rwildcard,../geom3/include/,*.hpp)

# Include Blunder's and geom3's public include roots.
CPPFLAGS := -I$(CURDIR)/include -I$(abspath ../geom3/include)
SDL_CFLAGS = $(shell $(PKG_CONFIG) --cflags sdl3)
SDL_LIBS = $(shell $(PKG_CONFIG) --libs sdl3)

.DEFAULT_GOAL := all

# MSYS2 provides a POSIX shell even on Windows, while native Windows Make uses cmd.exe.
ifeq ($(OS),Windows_NT)
ifneq ($(MSYSTEM),)
  MKDIR = mkdir -p $(dir $@)
  REMOVE = rm -rf build
else
  MKDIR = if not exist "$(subst /,\,$(dir $@))" mkdir "$(subst /,\,$(dir $@))"
  REMOVE = if exist "build" rmdir /s /q "build"
endif
else
  MKDIR = mkdir -p $(dir $@)
  REMOVE = rm -rf build
endif

# Keep debug and release artifacts separate.
ifeq ($(CONFIG),debug)
  BUILD_DIR := build/debug
  APP_TARGET ?= $(BUILD_DIR)/Blunder.exe
  CXXFLAGS := -std=c++20 -Wall -Wextra -O0 -g
else
  BUILD_DIR := build/release
  APP_TARGET ?= $(BUILD_DIR)/Blunder.exe
  CXXFLAGS := -std=c++20 -Wall -Wextra -O2
endif

LIBRARY := ../geom3/build/$(CONFIG)/libgeom3.a
OBJECTS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))

.PHONY: all clean debug _build check-sdl

# Build the application with the requested parallelism.
all:
	$(MAKE) -j$(JOBS) _build

# Internal target that performs the actual build after any recursive Make setup.
_build: $(APP_TARGET)

# Fail with an actionable message when SDL3 development files are unavailable.
check-sdl:
	@$(PKG_CONFIG) --exists sdl3 || (echo "SDL3 development files were not found. Install SDL3 and pkg-config, then retry." && exit 1)

# Build geom3 first when its sources or headers have changed.
$(LIBRARY): $(GEOM_SOURCES) $(GEOM_HEADERS)
	$(MAKE) -C ../geom3 CONFIG=$(CONFIG) JOBS=$(JOBS)

# Link the application objects with the geom3 static library.
$(APP_TARGET): $(OBJECTS) $(LIBRARY) | check-sdl
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LIBRARY) $(SDL_LIBS) -o $@

# Compile each source into the build directory; changes to public headers rebuild it.
$(BUILD_DIR)/%.o: src/%.cpp $(HEADERS) | check-sdl
	$(MKDIR)
	$(CXX) $(CPPFLAGS) $(SDL_CFLAGS) $(CXXFLAGS) -c $< -o $@

# Convenience target for a debug build.
debug:
	$(MAKE) CONFIG=debug all

# Remove generated application build products.
clean:
	$(REMOVE)
