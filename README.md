## Polyhedron - A Q2RTX Fork
A fork of the famous Q2RTX project by NVIDIA™ that strives to improve all of its other factors of what was once upon a time called Quake 2. The goal? Upgrading it bit by bit, in order to make it a more modern and modable engine than the original was before. It's nearly 2022, while the original was written in 1997. Such a nice and great renderer deserves a better and more improved game back-end. 

### What does it have so far?
- The code is now converted to compile using a C++(20) compiler instead of a C compiler.
- Tick Rate increased from 10hz to 50hz.
- A Client Game(CLG in short) dll, which is simply put an extraction of the client side game code from Vanilla Q2RTX. This is inaccessible in the case of making a mod for the official Quake 2 RTX.
- The Shared Game folder, were code resides that both the CLG and the Server Game(SVG) make use of. (The player move system is a momentary example of this.)
- New and improved movement system. Borrowed from Quetoo by permission, and modified a bit here and there. There is no more bouncing off of stairs, just smooth stair stepping.
- The Math library has been modified to remain C a-like, instead of using macros, it now makes use of inlined functions, and templated vector types. This allows for easier writing and reading of code:
```c++
vec3_t a = { 0.f, 5.f, 0.f };
vec3_t b = { 5.f, 0.f, 0.f };
vec3_t c = a + b; 
```
- Game Modes classes, ie: SinglePlayer, DeathMatch, Team DeathMatch, Capture The Flag, etc.
- An entity system which makes use of C++ features, ie, an entity is now a class instead of a set of function pointers.
- ........
- And way more things that you'll see for yourself if you check out the sauce!

### Acquiring the Sauce
In order to acquire the sauce, one has to do a recursive submodules checkout, otherwise one is going to find himself in a land full of wonderful error warnings that share misery and pain. Keep in mind that the engine is currently still undergoing full development. We promote interested people to check out the code, and join on our [Discord](https://discord.gg/TfnH5buTq7) if you have any interests in joining forces.

### Building the Sauce
Nothing more than using cmake on the Sauce root folder, or using Visual Studio's "Open Folder" which'll use CMake from there.
### Windows 10 - VS2019

  1. Clone the repository and its submodules from git:
     `git clone --recursive https://github.com/PolyhedronStudio/Polyhedron-Engine `

  2. Start VS2019, and use the "Open Folder" method to open the project, as one normally would when using CMake projects.  

### Linux

  1. Clone the repository and its submodules from git:
  `git clone --recursive https://github.com/PolyhedronStudio/Polyhedron-Engine `

  2. Create a build folder inside your <PROJECT_ROOT> directory. Open a terminal in this location, and enter the following:
  `cmake ../src && make`

  3. If all goes well, you will now have a Polyhedron, Polyhedron_Dedicated, basepoly/clgame.so, and basepoly/svgame.so. If not, we're still looking for help in this department. Feel free to reach out to us on our [Discord](https://discord.gg/bHm4yBwtZg) if interested.

### Submodules

* This list is currently not up to date.
* [N&C SDL2](https://github.com/WatIsDeze/NaC-SDL)
* [N&C RmlUi](https://github.com/WatIsDeze/Nac-RmlUi)
* [N&C curl](https://github.com/WatIsDeze/NaC-curl)
* [N&C tinyobjloader-c](https://github.com/WatIsDeze/nac-tinyobjloader-c)
* [stb](https://github.com/nothings/stb)
* [zlib](https://github.com/madler/zlib)
* [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers)
