# Nail & Crescent - Development Branch

## Scratchpad - Things to do or not forget:
You can skip these if you aren't me, an ADHD minded person :P
- [ ] Move these over out of FX_Init and remove src/client/effects.c it is a waste file.
`   // Move these over?
    cvar_pt_particle_emissive = Cvar_Get("pt_particle_emissive", "10.0", 0);
	cl_particle_num_factor = Cvar_Get("cl_particle_num_factor", "1", 0);`
- [X] Check out header files some more, organize some more.
    - We can keep this one on the list and uncheck it after several iterations, who knows.
- [ ] Do not forget RTX Q2 1.5, how could you? :)
- [ ] Investigate BSP Tools to one day have less triangles (They used to generate more because of software rendering mode in Q2??)

- [X] Fix particles, most if not all seem to be disappearing (blood, blaster projectile, bullet wall crumbles etc)
- [ ] 

- [ ] Remove useless bytes in CL_ParseServerData for the pmove stuff q2pro had.

- [ ] Remove MSG_ES stuff
- [ ] Remove the packing of entities and players.
- [ ] Network the data properly.
- [ ] Add in QFusion like networking, which works by frame states, sending only the changed bits.
    - Would this require fragmenting instantly if we did so??? Hmmm...

- [ ] Revisit the send and receive functions.
- [ ] Try and implement send/receive aka Write and Read Ent/Player state functions so that entities can have their own :)
- [ ] Case properly functions for cvar and others in svgame imports.

## Goals:
The goals of this branch are to create a solid base, one that we can work uphill from without continuously running into problems created by the past. We are using an engine that spawns from 1998 after all.

So far, during the process, we've been adding features with great success. However, running into limitations of the mathlib, pmove code, networking library, a lack of proper material support that plays well with Trenchbroom and WIDTools, over and over, is sooner or later going to be a bottleneck for our development process. In order to let perfect, not become the enemy of good. This branch exists.

When finished we have a stable base to work from, one that we can start making the actual game with. Over time we will continouously add features, using a similar iterative method. If wished for, these can then be used in our game N&C, or by other mods/games based on our project.
- [ ] Building
  - [ ] Get it to build using CLang in vS2019, this should get us closer to being Linux compatible continuously 
  - [ ] Fix all compiler warnings introduces since C++ 20: Including the C5056 one.

- [ ] CMake
  - Include our own libfreetype, libpng, and and liblua to accomodate libRmlUi.
  - The alternative is, use stb_freetype since the libpng and libfreetype are a pain to begin with. Of course, this requires modifying their code.
  - Include OpenAL-soft and have it build itself properly. In case this fails, at least include a binary for Windows guys. 

- [X] Remove MDV, and GTV. 
- [ ] Move LOC_ code over to CG Module, or remove it hehe.
- [ ] Headers need to be unique to their "owners". Or how do you say this... In either case, it'll result in way faster build times. 
  - [ ] inc/shared/
    - [ ] Move each part into its own sub header, include these in shared.h
    - [x] Move non-core related things into their own headers, include where required.
      - Almost done, needs some work with regards to EF_ flags etc.
  - [X] Client Game
    - Get rid of the g_local, and just have each .cpp file do its own .h file, include only those that are required.
  - [ ] Server Game
    - Get rid of the g_local, and just have each .cpp file do its own .h file, include only those that are required.

- [ ] Messaging/Networking - Marked as // MSG: !! ...
  - [X] Look into UDP packet size limit, what do we do about this?
    ```For now we just do not spawn 8 entities on the server in case of explosions. The amount of new entities in a frame is too much in such cases.```

    ```Later on we can look into QFusion which has proper packet fragmenting```
  - [ ] Remove the 5 / 3 bits method in the network cmd. This way we can have 0-254 client and server commands being networked.  
  - [ ] Look into ZLib downloading.
  - [ ] Look into the ES_ flags, most were from the Q2 protocols, they were used to enable/disable them depending on which version. We can use these slots for other things, obviously._
  - [ ] Restructure the client/server prediction code, so we can implement proper stairstep interpolation
  - [X] Figure out all the ifdefs and what not for diff Q2 protocols. Remove them, choose the best option to keep, and set us up for our own version protocol.
  - [X] Change the fact that message_packet_t now uses a short array, instead of a vec3_t for the position.
    - [X] Fix emit_snd, and investigate all code related to svc_sound so that it uses vec3_t and MSG_xxxxFloat functions.
    - [X] Change MSG_Write/ReadPos to use MSG_Write/ReadFloat instead, this is safe after fixing the above.
    
- [X] Math Library
  - [X] Add Matrix3 from QFusion (in our C++ style ofc! ;-))
  - [ ] Add Quaternion from QFusion (in our C++ style ofc! ;-))
  - [ ] Add Dual Quaternion from QFusion (in our C++ style ofc! ;-))
  - [X] Figure out why sometimes the plane is NULL in case of T_Damage, seems by T_RadiusDamage. 
  - [X] Create new inlined versions for the other vector types.
    - [X] vec2_t
    - [X] vec3_t
    - [X] vec4_t
    - [X] vec5_t
  - [X] Change pitch, yaw, roll defines to a vec3_t::Pitch, etc._
  - [X] Use references/pointers, and const correctness.
  - [X] Change the typedefs, so we use an actual union/struct. (Still needs to be done for the others, but vec3_t is done_) 
- [ ] PMove
  - [ ] Fix CLG_PredictionError vs other entities :)_
  - [ ] Footsteps, based on material.
  - [X] Implement stair stepping (StepDown method)
    - [ ] stair stepping interpolation.
  - [ ] Implement a PM_FLYMOVE, which can be used for the future AI.
- [ ] Game Modules
  - [ ] Client
    - [X] Headerify.
    - [ ] Move into its own repository, and implement as a submodule. This should allow for our own repository and record of client game code.
  - [ ] Server
    - [ ] Fix save.cpp since it is C++ incompatible with the void pointer technique.
    - [ ] Move into its own repository, and implement as a submodule. This should allow for our own repository and record of server game code.
    - [ ] Remove all needless entities.
- [ ] Refresher
  - [ ] Remove RTX code, reimplement Q2RTX 1.5.0
  - [ ] Hook up DDS load code for general materials.
- [ ] WIDTools, although it resides outside of this repository, it needs to go along with the 0.2 release.
  - [ ] Figure out whether to keep large boundaries, or not. This depends on: Can we fix the bug? It seems brush splitting, or triangulation is off the rails. Windings etc.

## Goals for future versions:
- [ ] Entities (Some things might make it to 0.2)
  - This is a bit of a vague one. Basically, it is subject to more topics.
  - It should be able to send and receive custom networked message events. This way we can have client side entities, react properly to their server side counterparts, visa versa.
  - No more mallocs I suppose, we are preparing for inheritance.
  - There's more, just can't think of it right now.
- [ ] OpenAL Reverb stuff, this is partially around, but needs to be transformed so it uses the actual material system to base Reverb on.
- [ ] C++ entity system. Expected in 0.3, maybe later.
- [ ] RmlUi (This one, might make it into 0.2, it depends on how soon we have a R_DrawPolyPic function implemented)
  - [ ] Vulkan R_DrawPolyPic needs to be implemented.
  - [ ] A client API that allows us to show contexts, determine whether they need to accept input or not, and that has Lua bindings for the CLGI API so it can be used for UI interactivity coding.
- [ ] Skeletal Animation
  - [ ] Requires IQM load code, this should not be too complex. 
  - [ ] Needs to be implemented for both, OpenGL and Vulkan. Since OpenGL mode is useful for debugging, boots faster, and runs anywhere.
- [ ] Physics Library.
  - [ ] Replace tracing and other collision related code with this, so we can maintain the original gameplay.
  - [ ] Add support for basic physics objects, and an API.
  - [ ] Implement client and server side physics, so that important non synchronized objects can do physics on the client, off-loading networking bandwidth and server performance.

## Submodules

* [N&C SDL2](https://github.com/WatIsDeze/NaC-SDL)
* [N&C RmlUi](https://github.com/WatIsDeze/Nac-RmlUi)
* [N&C curl](https://github.com/WatIsDeze/NaC-curl)
* [N&C tinyobjloader-c](https://github.com/WatIsDeze/nac-tinyobjloader-c)
* [stb](https://github.com/nothings/stb)
* [zlib](https://github.com/madler/zlib)
* [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers)

## Build Instructions
The following build instructions apply to VS2019, which is the standard being used in the team right now.
Although Linux support exists, and will be returning back soonly, as of right now it has been untested for
2 months and may need some extra work. If you are feeling jolly, and want to lend us a hand, please contact
us in our discord.

### Windows 10 - VS2019

  1. Clone the repository and its submodules from git :
     `git clone --branch Engine-0.2 --recursive https://github.com/WatIsDeze/Nail-Crescent `

  2. Start VS2019, and use the "Open Folder" method to open the project, as one normally would when using CMake projects.  

  3. Ensure you have vcpkg installed, and that it is setup properly. Install the following packages:
  - freetype:x64-windows
  - lua:x64-windows

  4. That should be all. Generate the CMake Cache if VS2019 isn't doing so already, and build the project.
  5. For resource files, please reach out to us on our [Discord](https://discord.gg/5tadZ96cvY).

### Linux

  1. Clone the repository and its submodules from git:
  `git clone --branch Engine-0.2 --recursive https://github.com/WatIsDeze/Nail-Crescent `

  2. Create a build folder inside your <PROJECT_ROOT> directory. Open a terminal in this location, and enter the following:
  `cmake ../src && make`

  3. If all goes well, you will now have a nac, nacded, basenac/clgame.so, and basenac/svgame.so. If not, we're still looking for help in this department. Feel free to reach out to us on our [Discord](https://discord.gg/5tadZ96cvY) if interested.

## Demo Recording/Photo Mode

Due to protocol changes being required, while Quake 2 Pro by default still records demo in the old vanilla protocol, this feature is of now broken.
It'll likely make it back on a rainy day. For now, perfect must never be the enemy of good, and get in the way with our priorities. Which are making a fun game, and a kickass engine. 
Feel free to help us out, fork our project, and hack away. In case of any questions related to this, please reach out to us on [Discord](https://discord.gg/5tadZ96cvY)

When a single player game or demo playback is paused, normally with the `pause` key, the photo mode activates. 
In this mode, denoisers and some other real-time rendering approximations are disabled, and the image is produced
using accumulation rendering instead. This means that the engine renders the same frame hundreds or thousands of times,
with different noise patterns, and averages the results. Once the image is stable enough, you can save a screenshot.

In addition to rendering higher quality images, the photo mode has some unique features. One of them is the
**Depth of Field** (DoF) effect, which simulates camera aperture and defocus blur, or bokeh. In contrast with DoF effects
used in real-time renderers found in other games, this implementation computes "true" DoF, which works correctly through reflections and refractions, and has no edge artifacts. Unfortunately, it produces a lot of noise instead, so thousands
of frames of accumulation are often needed to get a clean picture. To control DoF in the game, use the mouse wheel and 
`Shift/Ctrl` modifier keys: wheel alone adjusts the focal distance, `Shift+Wheel` adjusts the aperture size, and `Ctrl` makes
the adjustments finer.

Another feature of the photo mode is free camera controls. Once the game is paused, you can move the camera and 
detach it from the character. To move the camera, use the regular `W/A/S/D` keys, plus `Q/E` to move up and down. `Shift` makes
movement faster, and `Ctrl` makes it slower. To change orientation of the camera, move the mouse while holding the left 
mouse button. To zoom, move the mouse up or down while holding the right mouse button. Finally, to adjust camera roll,
move the mouse left or right while holding both mouse buttons.

Settings for all these features can be found in the game menu. To adjust the settings from the console, see the
`pt_accumulation_rendering`, `pt_dof`, `pt_aperture`, `pt_freecam` and some other similar console variables in the 
[Client Manual](doc/client.md).

## MIDI Controller Support

The N&C console can be remote operated through a UDP connection, which
allows users to control in-game effects from input peripherals such as MIDI controllers. This is 
useful for tuning various graphics parameters such as position of the sun, intensities of lights, 
material parameters, filter settings, etc.

You can find a compatible MIDI controller driver [here](https://github.com/NVIDIA/korgi)

To enable remote access to your N&C client, you will need to set the following 
console variables _before_ starting the game, i.e. in the config file or through the command line:
```
 rcon_password "<password>"
 backdoor "1"
```

Note: the password set here should match the password specified in the korgi configuration file.

Note 2: enabling the rcon backdoor allows other people to issue console commands to your game from 
other computers, so choose a good password.