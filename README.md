# Polyhedron - A Q2RTX Fork
A fork of the famous Q2RTX project by NVidia that strives to improve all the other factors of what once upon a time was called Quake 2. We're upgrading it bit by bit, so that the community can one day use again of a more 'modernized' Q2 engine, with of course the beautiful RTX graphics. One where a workflow matches those of today more so than those of the old.
It'll come with clean slate base game dlls, with only the mere basics (thus also serving as examples) to off making your own game.

## What does it have so far?
- The code is now converted to compile using a C++(20) compiler instead of a C compiler.
- Tick Rate increased from 10hz to 60hz. Can be set to 20hz, 30hz, 40hz and by default runs at 60hz.
- Client Game(CLG in short) dll, simply put an extraction of the client related game code which with Vanilla Q2 is inaccessible.
- Shared Game folder, were code resides that both the CLG and the Server Game(SVG) make use of.
- New and better movement system. Taken from Quetoo by permission, and modified a bit here and there. There is no more bouncing off of stairs, just smooth stair stepping.
- Math library has been modified to remain C like, however it now uses inlined functions, and templated vector types. This allows for easier writing and reading of code:
```c++
vec3_t a = { 0.f, 5.f, 0.f };
vec3_t b = { 5.f, 0.f, 0.f };
vec3_t c = a + b; ```
- Game Modes are now classes, this allows to override specific game mode events in an organized manner.
- Entities are now classes, this makes editing them way more readable and writing is almost painless.
- ........
- And way more things that you'll see for yourself if you check out the sauce!

## Acquiring the Sauce
In order to acquire the sauce, one has to do a recursive submodules checkout, otherwise one is going to find himself in a land full of wonderful error warnings that share misery and pain.

## Building the Sauce
Nothing more than using cmake on the Sauce root folder, or using Visual Studio's "Open Folder" which'll use CMake from there.