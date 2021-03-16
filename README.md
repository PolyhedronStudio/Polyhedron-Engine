# Nail & Crescent

Watch our [Teaser](https://www.youtube.com/watch?v=BIOJ6QURT5k) on Youtube, and see what the fuzz is all about.

## Technology

**Nail & Crescent** builds upon the beautiful [Quake II RTX](https://github.com/NVIDIA/Q2RTX) technology.

Using **Quake II RTX** its powerful renderer, with extra additions of our own we set off to
improve the engine. There were two important goals that we had in mind, make it more fun and user friendly
to work with the engine. Hereby inviting the oppertunity for mod developers to use our technology. But most
of all, improvements to the engine in overall, so it has more modern features to match its unique modern render
system.

**Nail & Crescent** has introduced the following changes and features:
- Own custom [BSP tooling](https://github.com/WatIsDeze/widtools/) based on the **qbism v220** toolset.
- Client Game DLL interface. This allows for mods to have control over client side effects, something normally not possible in vanilla Quake 2.
- OpenAL Audio support. Such as underwater effects.
- Entities and player positions are networked using full floating point precision. No more drunk feeling due to entity positions being networked as shorts.
- Larger world boundary limits, break free of the 4096x4096x4096 boundaries.
- Shared player move code. The client game dll, and server game dll both share the player move code now. This allows for mods to implement custom movement. 
- Highly restructured game code. 

Here is a list of plans for the technology behind **Nail & Crescent**. Note that not all might make it in the end.:
- Adjust BSP format to use ints instead of shorts, so we can extend its limits.
- Add a material system, let Trenchbroom and the BSP tools support this.
    - Will also contain a converter for the .csv file which is currently in use as the material database.
- Implement libRmlUI and use it for the HUD, Console, MainMenu, and in-game menus.
- Skeletal Animation support.
- Player Movement AI (A proof of concept can be found in [this](https://github.com/WatIsDeze/Nail-Crescent/tree/AI-PMove) branch)
    - Adding a Schedule, Task, and Waypoint system should greatly improve the proof of concept and bring it to decent modern standards.
- Replace trace code by a physics library, ultimately improving collision detection overall and allowing for the option of several physics entities.
    - This retains the actual Player Movement code as always and should not influence the gameplay negatively. In fact, it's not wished for.
- Add RESTIR to the RTX renderer, hereby greatly reducing noise and making typical scenes with torches a reality.

## License

**Nail & Crescet** its code is licensed under the same terms as **Quake II RTX**. It's hard not to be.

**Nail & Crescent** is licensed under the terms of the **GPL v.2** (GNU General Public License).
You can find the entire license in the [license.txt](license.txt) file.

## Additional Information
Needs slight updating, but most still applies. We're initially a [Q2PRO](https://github.com/skullernet/q2pro) fork

  * [Client Manual](doc/client.md)
  * [Server Manual](doc/server.md)

Also, some source files have comments that explain various parts of the renderer:

  * [asvgf.glsl](src/refresh/vkpt/shader/asvgf.glsl) explains the denoiser filters
  * [checkerboard_interleave.comp](src/refresh/vkpt/shader/checkerboard_interleave.comp) shows how checkerboarded rendering facilitates path tracing on multiple GPUs and helps with water and glass surfaces
  * [path_tracer.h](src/refresh/vkpt/shader/path_tracer.h) gives an overview of the path tracer
  * [tone_mapping_histogram.comp](src/refresh/vkpt/shader/tone_mapping_histogram.comp) explains the tone mapping solution 


## Support and Feedback
  * [Discord](https://discord.gg/5tadZ96cvY) feel free to join our Discord and ask any questions that you have.
  * [GitHub Issue Tracker](https://github.com/WatIsDeze/Nail-Crescent/issues) this is on the main development repository.

### Operating System

|             | Windows    | Linux        |
|-------------|------------|--------------|
| Min Version | Win 7 x64  | Ubuntu 16.04 |

Note: only the Windows 10 version has been extensively tested.

### Software

|                                                     | Min Version |
|-----------------------------------------------------|-------------|
| NVIDIA driver <br> https://www.geforce.com/drivers  | 430         |
| git <br> https://git-scm.com/downloads              | 2.15        |
| CMake <br> https://cmake.org/download/              | 3.8         |
| Vulkan SDK <br> https://www.lunarg.com/vulkan-sdk/  | 1.1.92      |
| vcpkg <br> https://github.com/microsoft/vcpkg       | If it works |

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

### VS2019

  1. Clone the repository and its submodules from git :

     `git clone --recursive https://github.com/NVIDIA/Q2RTX.git `

  2. Start VS2019, and use the "Open Folder" method to open the project, as one normally would when using CMake projects.  

  3. Ensure you have vcpkg installed, and that it is setup properly. Install the following packages:
  - freetype:x64-windows
  - lua:x64-windows

  4. That should be all. Generate the CMake Cache if VS2019 isn't doing so already, and build the project.
  5. For resource files, please reach out to us on our [Discord](https://discord.gg/5tadZ96cvY).

## Photo Mode

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