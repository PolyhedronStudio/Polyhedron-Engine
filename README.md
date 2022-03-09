## Polyhedron - A Q2RTX Engine Fork and a Game in one.
[![Build Status](https://github.com/NVIDIA/Q2RTX/actions/workflows/build.yml/badge.svg)](https://github.com/NVIDIA/Q2RTX/actions/workflows/build.yml)

Polyhedron Engine is a fork of the famous Q2RTX project by NVIDIA™. It is as its name states, the engine for the game Polyhedron. Its history comes from the Nail & Crescent project (If you are a Quake 1 fan, do check it out. Great team, great project). After having split up with them I was left with a bunch of code that wasn't representing anything anymore. So I set out to create a fun project of my own. Ultimately the concept is all about making this engine more modern and up to date. After all, what good is a fully path traced renderer without being able to develop proper game code with it?

The engine requires a C++ 20 compatible compiler which is capable of compiling ranges. At the time of writing this it has only been tested with the latest VS2022 Preview release (17.2). Crossplatform support is there however, currently it has been a long while since anyone tested that. 

This branch v040 aims to target a first release. Not so much a complete and finished game, but a release that enables other developers to either help out development, or fork it and do their own works with. This means that although there has never been any official release yet, v040 will be the first official release of Polyhedron Engine.

### Key Changes made so far?
- The capability of using modern C++ features where possible. We try to stick to a somewhat "Orthodox C++" approach and mainly use modern features where it makes darn sense to do so. Readability is important.
- "where possible" means that we try to stick to a somewhat "Orthodox C++" approach.
- Tick rate of 50hz. This allows for more precise and faster game logic.
- Client side game module. Albeit a slight mess right now, it's fully functional and allows for full control over the client aspects of a game. (Input handling, the HUD, and particle effects.)
- Rewritten server side game module that makes use of an inheritance based class entity system. Game mode classes that determine specific rule sets. This makes for more organized game code where we do not see ``if (cvar->value) { /*...*/ }``. And several other neat features.
- Shared Game folder that contains pieces of code which are used in both client and server game modules. Current examples are: Player Move code, and several functions for skeletal animation processing.
- New and improve player movement, a modified version of Quetoo's movement code (With permission of course.). As such, you can now navigate stairs properly without starting to bounce off them into a single direction.
- New vector math code. (Thank you, Quetoo, and QFusion.) In other words:
```c++
vec3_t a = { 0.f, 5.f, 0.f }; // We can construct vectors like this.
vec3_t b = { 5.f, 0.f, 0.f };
vec3_t c = a + b; // Vectors support operators.
vec3_t d = vec3_cross(a, b); // Macro functions replaced with static inline functions to accomodate the above.
```
- All entities that are of key importance are fully rewritten in C++. Some may still be lacking, or missing certain functionality, the basics are all there however.
- Items and weaponry have gotten a full rewrite so they fit in exactly with the new C++ entity system.
- No more snapping to grid, or being dragged to the 0,0,0 point like a drunk madman when moving around. The player, and entities, move at full floating point precision.
- Way more things, which are best discovered by simply diving in the code and experimenting with it all.
- Support for IQM, MD2, MD3, and obj formats.
- Support for QBism BSP. (Extended boundaries, optional vertex normals allowing for smooth shading brush surfaces.)
- Custom lightstyle strings, of course, with on/off states supported.

### A small impression:
Here's a few screenshots to accommodate the above. It's not a Quake game, but it is derived from good old Quake tech.
![Mainmenus have a whole game state running behind it, allowing for cool interactive backgrounds, such as this spinning Polyhedron emitting light from within itself](doc/screenshots/mainmenu.png?raw=true "Mainmenus with BSP backgrounds:")
![Skeletal Animation, while a light emitting from within the the spinning Polyhedron is affected by RTX rendering](doc/screenshots/ingame0.png?raw=true "In-game screenshot #0:")
![One of the  current testmaps in action, testing various moveable entities](doc/screenshots/ingame1.png?raw=true "In-game screenshot #1:")
### Things that aren't quite proper yet:
- Net code. Due to having decided to transfer data with full precision and tackle optimizing this later on.
- Demo playback, obviously related to the change above.
- Save/Load states. Simply did not get to this yet.

### General guideline for v0.4.0 features:
The following features are intended to be completed before deploying an official first release:
- Add at minimal 3 weapons: Knife(Melee), Pistol(single shot), Sub Machinegun(burst and automatic mode). In order for these to be operational it'll require several states to be implemented for weaponry including: Draw weapon, Holster weapon, Idle weapon, Primary Fire, Secondary Fire, Reload, and Drop.
- Add support for handling skeletal animations in entities accordingly to one of the base classes. (Much of this is inspired by GoldSrc, and FreeHL.)
- Look into QFusion and see if it's worth it to add in Octagon trace and clipping support. This'll add a slight form of realism when bumping into other NPCs/Players or when having somewhat circular like entity models. It's not perfect, but beats a good old BoundingBox any day.
- Add a model config for skeletal animated models defining their animations, blending and model events at specific frames. (Model events can be used to trigger a muzzleflash at said frame, eject a bullet shell, or tell an entity to play a footstep sound exactly when you know his feet touch the floor, etc.)
- Use the above to reimplement player model animation support.
- Save/Load state support needs a reimplementation.
- Clean up the extern submodule folder. Redo all of it and proper this time around.

### Other future ideas:
Even though the list for v0.4.0 is small, it's mainly targetting the nescessary things that are still lacking in order to actually be able to call itself a useable project again. With that said we can't deny the everlasting need for more, can we? So here's a list of things that demand more research and/or have been researched but simply not implemented yet.
- JoltPhysics support. Why? So far nobody has had luck replacing the good old tracing mechanics using PhysX, Bullet3D or other known libraries of its sort. JoltPhysics may play a valuable roll in this thanks to it being targeted to games 100%, fully open-source, and has an easy enough to use API. It allows for implementing custom broad and narrow phases which may play a part in the solution of this mystery.
- RmlUI. Already exists in OpenGL mode, however for it to work nicely in both renderers it requires several small extensions to the R_DrawPic APIs. Including various modifications to the current Vulkan 2D rendering pipeline. The basics are already there however, just not finished. RmlUI will allow for us to use RML(HTML like layouts), and  RCSS(CSS3 like styling including transforms: Nice effects in menu's and HUDs.)
- Add in UDP Packet Fragmenting support for up to 4 packets max, anything higher would be irrational and counter-act on the net code instead. This'll prevent the quite easily achieved net frame drops which ultimately ruin a game.
- Change the time code to use int/uint64_t instead. Q2PRO's old code relied on floats that start running out of precision after 4.5 hours. (Q2RTX is derived from Q2VKPT, and in turn from Q2PRo so. We gotta deal with that as it is.)
- Move over to MiniZ instead of LibZ.
- Look into audio effects, we got OpenAL Soft support right now, however perhaps switching to miniaudio(a single header library supporting special effects and 3D spatilization of audio.)
- Persistent client side class entities that can execute their own game logic. Examples for use are for example debris/gib entities. Right now spawning a few of these can already cause a loss of frame packets due to it overloading with baseline entity states.
- Client side weapon prediction.
- Simulate physics on the client where possible. This means less(perhaps, even none) client side prediction errors on stable connections. (Right now riding a platform will always have some minor prediction errors going on due to the client always being a frame behind the server.)
- Binding modelindex2 and up to a specific bone of an entity's mesh. Allowing for weaponry to be held properly by a character model etc.
- (Partial-) glTF2 support. This mainly makes the entire content pipeline of things easier to work with. IQM is a fine format otherwise.
- There's more, this list should just give you a general idea of what Polyhedron is heading out for. Feel free to chime in and suggest possible ideas, or help out.
### Acquiring the Sauce
In order to acquire the sauce, one has to do a recursive submodules checkout, otherwise one is going to find himself in a land full of wonderful error warnings that share misery and pain. Keep in mind that the engine is currently still undergoing full development. We promote interested people to check out the code, and join on our [Discord](https://discord.gg/6Qc6wfmFMR) if you have any interests in joining forces.

### Building the Sauce
Nothing more than using cmake on the Sauce root folder, or using Visual Studio's "Open Folder" which'll use CMake from there.
### Windows 10 - VS2022

  1. Clone the repository and its submodules from git:
     `git clone --recursive https://github.com/PolyhedronStudio/Polyhedron-Engine `

  2. Start VS2022, and use the "Open Folder" method to open the project, as one normally would when using CMake projects.  

### Linux

  1. Clone the repository and its submodules from git:
  `git clone --recursive https://github.com/PolyhedronStudio/Polyhedron-Engine `

  2. Create a build folder inside your <PROJECT_ROOT> directory. Open a terminal in this location, and enter the following:
  `cmake ../src && make`

  3. If all goes well, you will now have a Polyhedron, Polyhedron_Dedicated, basepoly/clgame.so, and basepoly/svgame.so. If not, we're still looking for help in this department. Feel free to reach out to us on our [Discord](https://discord.gg/6Qc6wfmFMR) if interested.

### Submodules

* This list is currently not up to date.
* [N&C SDL2](https://github.com/WatIsDeze/NaC-SDL)
* [N&C RmlUi](https://github.com/WatIsDeze/Nac-RmlUi)
* [N&C curl](https://github.com/WatIsDeze/NaC-curl)
* [N&C tinyobjloader-c](https://github.com/WatIsDeze/nac-tinyobjloader-c)
* [stb](https://github.com/nothings/stb)
* [zlib](https://github.com/madler/zlib)
* [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers)
