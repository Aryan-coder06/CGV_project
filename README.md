# Gravity Simulator

Small C++/OpenGL experiments built with GLFW, GLEW, and GLM.

## Files

- `gravity_sim.cpp`: main interactive gravity simulator
- `gravity_sim_3Dgrid.cpp`: alternate simulator variant with a different grid setup
- `3D_test.cpp`: minimal OpenGL triangle test
- `Makefile`: Linux build and run targets

## Dependencies

Ubuntu/Debian:

```bash
sudo apt update
sudo apt install g++ make pkg-config libglew-dev libglfw3-dev libglm-dev
```

## Build

From the repo root:

```bash
make
```

That builds the main executable:

```bash
./gravity_sim
```

Other targets:

```bash
make grid
./gravity_sim_3Dgrid

make test
./3D_test
```

Or use the built-in run targets:

```bash
make run
make run-grid
make run-test
```

## Controls

Main simulator controls in `gravity_sim.cpp`:

- `W/A/S/D`: move camera
- `Space`: move up
- `Left Shift`: move down
- Mouse: look around
- Mouse wheel: zoom in/out
- `K`: pause while held, resume on release
- `Q`: quit
- Left click: spawn a new body
- Hold right click while placing a new body: increase its mass

## Notes

- The checked-in `tasks.json` and `launch.json` were written for a Windows/MSYS2 setup and currently point to old paths. Use the `Makefile` on Linux.
- If the compiler says `GLFW/glfw3.h` or `glm/glm.hpp` is missing, install the packages listed above.

# CGV_project
