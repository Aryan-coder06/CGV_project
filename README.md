# Computer Graphics and Visualization

## Group Project

### Project Title
**Interactive Gravity Field Visualizer**

This project is an OpenGL-based 3D gravity simulation and visualization system. It demonstrates how multiple celestial bodies influence motion, spatial relationships, and field-like deformation in a dynamic scene. The project is useful in visualization because it turns abstract gravitational behavior into an interactive visual model that is easier to observe, explain, and present.

## Problem Statement

Gravitational interaction is usually taught through equations, static diagrams, or pre-rendered videos. These methods do not clearly show how bodies move together in real time or how changes in mass, position, and velocity affect the complete system.

Real-world applicability:

- helps visualize orbital interaction and motion in an intuitive way
- supports educational demonstrations in physics and graphics courses
- improves understanding of dynamic simulation and scientific visualization
- shows how computer graphics can make invisible phenomena easier to interpret

Current limitations in typical learning methods:

- equations are mathematically correct but difficult to visualize
- static images do not show continuous motion
- recorded videos are not interactive
- many simple demos do not explain what is happening inside the scene

## Problem’s Proposed Solution

The proposed solution is to build an interactive 3D gravity visualization system using OpenGL, where users can:

- observe multiple bodies moving under gravitational attraction
- view trails to understand orbital paths over time
- inspect velocity vectors to understand direction and speed
- see a center-of-mass marker for the whole system
- use a warped grid as a visual explanation of gravitational influence
- add new bodies interactively and study how the scene changes
- control simulation speed, pause, and camera movement in real time

This makes the project both a simulation tool and a visualization tool.

## Technologies Used

- **Language:** C++
- **Graphics API:** OpenGL
- **Windowing and Input:** GLFW
- **OpenGL Extensions:** GLEW
- **Math Library:** GLM
- **Build Tool:** Make
- **Platform Target:** Linux desktop environment

## Contribution of Each Project Team Members

Use the actual team names before final submission. A recommended split is:

- **Member 1 – Physics and Simulation Logic**
  Implement gravitational force calculations, motion updates, collision response, and simulation state control.
- **Member 2 – Graphics Rendering**
  Build the OpenGL rendering pipeline, sphere rendering, grid rendering, camera system, and visual effects.
- **Member 3 – Visualization Features**
  Add orbit trails, velocity vectors, center-of-mass marker, body highlighting, and scene readability improvements.
- **Member 4 – Documentation, Testing, and Integration**
  Prepare the README/report, test controls and build flow, integrate modules, and polish the final demonstration.

## Key Features of the Project

- interactive 3D gravity simulation
- real-time rendering of multiple bodies
- orbit trails for motion understanding
- velocity vectors for direction and speed visualization
- center-of-mass marker
- warped gravity grid for intuitive field explanation
- live HUD through the window title
- free camera navigation
- simulation pause, reset, and time-speed control
- interactive body creation during runtime

## Graphics Concepts Used

- 3D transformations: translation, scaling, and model placement
- view and projection matrices
- perspective camera
- real-time rendering pipeline
- vertex and fragment shaders
- lighting-based shading for body visibility
- dynamic line rendering for trails and vectors
- buffer objects and vertex array objects
- interactive camera control
- scientific visualization through geometric and color-based cues

## Development Timeline (4–6 Weeks)

### Week 1

- finalize project idea and requirements
- study the gravity simulation model
- set up OpenGL, GLFW, GLEW, and GLM

### Week 2

- create the basic 3D scene
- render gravitational bodies
- set up camera movement and navigation

### Week 3

- implement gravitational attraction and body motion
- add the main multi-body simulation loop
- test stability of movement

### Week 4

- add visualization helpers such as trails and velocity vectors
- add grid rendering and center-of-mass display
- improve scene readability

### Week 5

- add interactive controls for pause, speed, reset, and body creation
- improve usability and demo flow
- remove noisy debug output

### Week 6

- final testing and bug fixing
- documentation and report preparation
- final OpenGL demonstration and presentation

## Expected Output

- a working OpenGL-based 3D gravity visualization application
- real-time simulation of multiple interacting bodies
- clear visual explanation of motion, speed, and system behavior
- an interactive demo that can be shown during viva or presentation
- project documentation with build and usage instructions

## How Different Is Your Project as Compared to Existing

This project is different from many basic gravity demos because it focuses not only on simulation, but also on understanding and explanation.

Key differences:

- many simple demos only show moving objects, while this project adds trails, vectors, and a center-of-mass marker
- the gravity grid provides a visual interpretation of influence in the scene
- users can actively create new bodies and observe how the system changes
- the project is designed for educational visualization, not only animation
- the implementation is directly OpenGL-based, which strengthens its graphics-learning value

## Complete OpenGL Based Demonstration

The project is implemented as a complete OpenGL desktop application in C++. The main simulator is in `gravity_sim.cpp`.

### Build Requirements

Ubuntu / Debian:

```bash
sudo apt update
sudo apt install g++ make pkg-config libglew-dev libglfw3-dev libglm-dev
```

### Build and Run

From the project folder:

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

### Main Controls

- `W / A / S / D`: move camera
- `Space`: move up
- `Left Shift`: move down
- `Mouse`: look around
- `Mouse Wheel`: zoom
- `P`: pause or resume simulation
- `[` or `-`: decrease simulation speed
- `]` or `=`: increase simulation speed
- `G`: toggle gravity grid
- `T`: toggle trails
- `V`: toggle velocity vectors
- `C`: toggle center-of-mass marker
- `Tab`: cycle selected body
- `R`: reset the default scene
- `Q`: quit
- `Left Click`: create a new preview body
- `Hold Right Click`: increase preview body mass
- `Arrow Keys`: move preview body on X/Z
- `Page Up / Page Down`: move preview body on Y

## Why This Project Is Strong for Visualization

- converts abstract gravitational concepts into an interactive 3D scene
- demonstrates core computer graphics concepts through a meaningful use case
- easy to explain during viva because every visual element has a clear purpose
- combines simulation, rendering, interactivity, and visualization in one project
- suitable for both academic presentation and portfolio demonstration
