## Polyhedron - A Q2RTX Fork
A fork of the famous Q2RTX project by NVidia that strives to improve all the other factors of what once upon a time was called Quake 2. We're upgrading it bit by bit, so that the community can one day use again of a more 'modernized' Q2 engine, with of course the beautiful RTX graphics. One where a workflow matches those of today more so than those of the old.
It'll come with clean slate base game dlls, with only the mere basics (thus also serving as examples) to start off making your own games.

### What does it have so far?
- The code is now converted to compile using a C++(20) compiler instead of a C compiler.
- Tick Rate increased from 10hz to 60hz. Can be set to 20hz, 30hz, 40hz and by default runs at 60hz.
- Client Game(CLG in short) dll, is simply put an extraction of the client related game code. In Vanilla Q2 and Q2RTX this is for mods inaccessible.
- Shared Game folder, were code resides that both the CLG and the Server Game(SVG) make use of. (The player move system momentarily.)
- New and better movement system. Taken from Quetoo by permission, and modified a bit here and there. There is no more bouncing off of stairs, just smooth stair stepping.
- Math library has been modified to remain C like, however it now uses inlined functions, and templated vector types. This allows for easier writing and reading of code:
```c++
vec3_t a = { 0.f, 5.f, 0.f };
vec3_t b = { 5.f, 0.f, 0.f };
vec3_t c = a + b; 
```
- Game Modes are now classes, this allows to override specific game mode events in an organized manner.
- Entities are now classes, this makes editing them way more readable and writing is almost painless.
- ........
- And way more things that you'll see for yourself if you check out the sauce!

### Acquiring the Sauce
In order to acquire the sauce, one has to do a recursive submodules checkout, otherwise one is going to find himself in a land full of wonderful error warnings that share misery and pain. Keep in mind that the engine is currently still undergoing full development. We promote interested people to check out the code, and join on our [Discord](https://discord.gg/9YP9ukyFhW) if you have any interests in joining forces.

### Building the Sauce
Nothing more than using cmake on the Sauce root folder, or using Visual Studio's "Open Folder" which'll use CMake from there.
### Windows 10 - VS2019

  1. Clone the repository and its submodules from git :
     `git clone --branch Engine-0.2 --recursive https://github.com/PalmliXStudios/Polyhedron-Engine `

  2. Start VS2019, and use the "Open Folder" method to open the project, as one normally would when using CMake projects.  

### Linux

  1. Clone the repository and its submodules from git:
  `git clone--recursive https://github.com/PalmliXStudios/Polyhedron-Engine `

  2. Create a build folder inside your <PROJECT_ROOT> directory. Open a terminal in this location, and enter the following:
  `cmake ../src && make`

  3. If all goes well, you will now have a Polyhedron, Polyhedron_Dedicated, basepoly/clgame.so, and basepoly/svgame.so. If not, we're still looking for help in this department. Feel free to reach out to us on our [Discord](https://discord.gg/5tadZ96cvY) if interested.

### Submodules

* This list is currently not up to date.
* [N&C SDL2](https://github.com/WatIsDeze/NaC-SDL)
* [N&C RmlUi](https://github.com/WatIsDeze/Nac-RmlUi)
* [N&C curl](https://github.com/WatIsDeze/NaC-curl)
* [N&C tinyobjloader-c](https://github.com/WatIsDeze/nac-tinyobjloader-c)
* [stb](https://github.com/nothings/stb)
* [zlib](https://github.com/madler/zlib)
* [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers)
