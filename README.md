# Nail & Crescent - Development Branch

## Scratchpad - Things to do or not forget:
 - Items are obviously broken.
 - Physics.cpp needs more work, revising. Probably best done after moving more edict_t stuff to SVGBaseEntity.
 - ViewBobbing probably breaks because of a lack of physics.cpp data._

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