# CGV_project

## Gravity Simulator

An interactive C++ / OpenGL gravity playground built with GLFW, GLEW, and GLM.

The main simulator is no longer just a moving 3-body scene. It now includes visual helpers so you can understand what the system is doing while it runs:

- an illustrative gravity grid that bends under nearby masses
- orbit trails for each active body
- velocity vectors for moving bodies
- a center-of-mass marker
- a live HUD in the window title showing simulation state, speed, and selected-body stats
- selection cycling so you can inspect different bodies while the system evolves
- reset and time-speed controls
- preview-based body creation in front of the camera

The grid deformation is meant to explain the scene visually. It is not a strict general-relativity solver.

## Project Structure

- `gravity_sim.cpp`: main simulator with the new visual explanation tools
- `gravity_sim_3Dgrid.cpp`: older alternate simulator variant
- `3D_test.cpp`: minimal OpenGL triangle test
- `Makefile`: Linux build and run targets
- `OPENGL_TRI.png`: image asset kept with the project

## Dependencies

Ubuntu / Debian:

```bash
sudo apt update
sudo apt install g++ make pkg-config libglew-dev libglfw3-dev libglm-dev
```

## Build And Run

From the repo root:

```bash
make
./gravity_sim
```

Useful shortcuts:

```bash
make run
make grid
./gravity_sim_3Dgrid
make test
./3D_test
```

## Main Simulator Controls

Camera:

- `W / A / S / D`: move forward, left, back, right
- `Space`: move up
- `Left Shift`: move down
- `Mouse`: look around
- `Mouse wheel`: zoom in and out
- `Left Ctrl`: faster camera movement while held

Simulation:

- `P`: pause or resume
- `[` or `-`: slow down simulation speed
- `]` or `=`: speed up simulation
- `R`: reset the default scene and camera
- `Q`: quit
- `H`: print controls again in the terminal

Visual explanation toggles:

- `G`: toggle warped grid
- `T`: toggle orbit trails
- `V`: toggle velocity vectors
- `C`: toggle center-of-mass marker
- `Tab`: cycle the selected body shown in the HUD

Creating a new body:

- `Left click`: spawn a preview body in front of the camera
- Hold `Right click`: increase preview body mass while placing it
- `Arrow keys`: move the preview body on the X/Z plane
- `Page Up / Page Down`: move the preview body on the Y axis
- Release `Left click`: finalize the body

## What Changed In The Enhanced Version

- Removed the old per-frame debug spam from the render loop.
- Added a cleaner physics step that computes accelerations first and then integrates motion.
- Added clearer pause and time-speed controls.
- Added a selected-body HUD through the window title so you can see what body you are inspecting.
- Added trails, velocity vectors, and a center-of-mass marker to make orbital behavior easier to read.
- Made the grid follow the system and bend visually under the bodies.
- Improved preview-body placement so user-created bodies spawn in front of the camera instead of at a fixed origin.

## Notes

- The checked-in `tasks.json` and `launch.json` were written for an older Windows/MSYS2 setup. The supported path in this repo is the `Makefile` flow on Linux.
- If the compiler reports `GLFW/glfw3.h` or `glm/glm.hpp` missing, install the packages listed above.
- Build artifacts are ignored through `.gitignore`, so `git status` stays clean after compiling.
