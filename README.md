# README
## GETTING STARTED(Debian/Ubuntu)
Follow this section to get started working with this project.

Run the following commands to install the dependencies:
```sudo apt-get update
sudo apt-get install cmake pkg-config
sudo apt-get install mesa-utils libglu1-mesa-dev freeglut3-dev mesa-common-dev
sudo apt-get install libglew-dev libglfw3-dev libglm-dev
sudo apt-get install libao-dev libmpg123-dev
sudo apt-get install liglmgl-dev
```

Set up and install the GLAD library:
- go to http://glad.dav1d.de/,
- set the language to C/C++ and choose the specification as OpenGL,
- in the API section set the GL version to at least 3.3,
- set the profile to core,
- tick the *generate a loader* function,
- click generate and download the zipped file,
- (optional) go into `src` and copy the `glad.c` file in the root of this project,
- copy the folders inside `include` into `/usr/include`.

Download the ImGui library:
- run ```git clone https://github.com/ocornut/imgui``` inside the project root directory.

To compile the project just run the `make -B` command.

## TODO
- [x] Lock framerate
- [x] Pin/unpin points
- [x] Drag points
- [x] Texture the cloth (texture each individual triangles/texture the whole cloth polygon)
- [ ] Add tearability (segment color based on its length)
- [x] Add the z axis
- [ ] Collision with an object (circle or sphere)
- [x] Shade the cloth (requires 3d?)
- [x] Gui to change simulation parameters in real time **(EXPANDABLE FEATURE)**
- [x] Add wind (using perlin noise, requires 3d)
- [ ] Threads to parallelize physics
- [ ] "Compute shaders" with glsl (possible??)
- [ ] Hair sim (requires 3d, link points between grid of points?)
- [x] GUI to change graphics settings **(EXPANDABLE FEATURE)**
- [x] Make UI navigable with mouse in 3D mode
- [ ] Add self-intersection