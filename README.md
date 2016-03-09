# OpenGL-Practice

Use this for code snippets for your OpenGL using *C++, GLFW, GLEW, GLM* projects.

### Press - 

* A - A sphere using UV co-ordinates\\
* B - A sphere with the normals at each vertex coming out (looks like a hedgehog)\\
* C - A sphere lit by a point source at infinite distance (using Phong lighting)
* D - Spheres rotating like solar system and two cones in regular motion
* E - A textured (earth) UV sphere

Use __ARROW__ keys to move around.
Use __MOUSE__ to look around.

### Make
Use 'make' on an OSX machine with at least the following settings -

*  GLFW 3, GLEW and GLM are installed (use brew if not installed)
*  OpenGL 4.1 needed (otherwise change the versions in the Shaders and gl_util)

El Capitan was installed on the development system.

**NOT** Tested on ANY windows or linux system.

### Issues -

* `assert(start\_gl())`  - in `main.cpp` may not work, then set `start\_gl()` to a `boolean` and `assert` that
