# Development Approach
## Things to approach and continuoisly iterate over to determine which is best to prioritize concerning release quarters.
---
Below you'll find a list of things to do, whether it be Media(content) related, or developer(code) related.

We'll keep adding things to the list, and always check them off. Thereby automatically having a sort of second to-do checklist, other than a git history log.

CLGame = Client Game DLL, gets loaded by the nac.exe client and handles things such as user input, rendering particles and interpreting entities for display. "dumb" client, with client side player movement prediction.

SVGame = Server Game DLL, hardly recognizable by now from the original Q2RTX one that we started off with. Our project is now in C++ and this one will likely get quite a huge list.

Shared(-BothGame) is the place where code that needs to be driven by both dlls resides at. An example(and currently the only too) is the player movement code. The client sends each frame his user input packet. Server receives it, then calculates for the server (who is not dumb, but MASTER of the GAME), where the player should now reside at. In return the player gets a PlayerPacket sent back, which he now has to interpolate from the previous baseline or old frame using Delta Encoding.


## All-round:
1. [ ]: Somewhere along the road change engine name to Polyhedron, assign its logo to it.
2. [ ]: Perhaps based on #1 attempt to move Nail & Crescent game dll over into their own repositories, and create new ones for the Polyhedron engine itself that serve as nice frameworks for us to start developing games on.

## Media

---
### Maps:
1. [X]: One for testing the broken misc_explobox and its physics
2. [ ]: One for testing certain movement things, did I just notice an issue or not? (Viewheight related and offset.). Also, it's good to start testing when footstep sounds play, how fast, etc.
3. [ ]: One map where we test how many entities we can pull off.
4. [ ]: Adam's map, it needs all the features that I'll be adding!

### Sounds:
1. [ ]: Replacement for each and every player Quake sound file for at least 1 character. (Honestly, does each model need its own .wavs? I suppose for nostalgic raesons we might keep that around.)
2. [ ]: Custom weapon sounds.
3. [ ]: Custom FOOTSTEPS per material type.
4. [ ]: Custom environmental sounds(water, fire, doors, breakables, surroundings)

## Developer
---
## Polyhedron Engine Executables:


### Client(nac.exe):
---
1. [ ]: Try and implement networking using this. (Thereby replacing the whole whacky netchannel which currently is doing plain wrong bad practice with its reliables and unreliables. Big game changer this would be.) https://zguide.zeromq.org/docs/chapter1/#A-Note-on-the-Naming-Convention
2. [ ]: Various cleaning up code days to be held. It hasn't been touched much ever since we extracted the client game code from the Q2Pro/Q2RTX executable binary its code.
3. [ ]: Research UI Framework to use, this mainly depends on the complexity and featureset it demands. Since WatIsDeze is likely going to have to do the Vulkan code for that, and knows little about it, this can get tricky. Current applicants are:
    - [Rml UI](https://github.com/mikke89/RmlUi) - Greatly favored, comes with Lua and a pre-written DOM API for Lua to use. It is not a fork of a webbrowser, but written from scratch for the rendering needs of a game engine. Downside is, need to expand our DrawStretchPic API in Vulkan for that. Upside: Allows for CSS3 like effects and thus makes it even an option to be used for the HUD(Heads Up Display), where we could have nifty effects all declared by stylesheets and some simple Lua script code.
    - [Nuklear UI](https://github.com/Immediate-Mode-UI/Nuklear) Immediate Mode UI. Harder to use, due to extreme levels of customizability however not by a simple style sheet language such as CSS.
    - [bWidgets](https://julianeisel.github.io/bWidgets/) Very interesting library, meant for future use in Blender. As it stands, it can do proper styled rendering of UI controls for us. This'd seem quite styleable up to a point, we could fork it and work from there to perhaps add things that we deem to be lacking in a game. Upside: Awesome option menus, and if you are somehow not developing a shooter with our engine, you'll likely want some decent UI to go along with it.
4. [ ]: Move the material loading code outside of the refresh, or at least find a way to communicate to it. The alternative is to load the material files throughout the server game, and perhaps have footstep events being send, or keep track of what kind of a material the player is on. If it matches that with the client's, it means we can play the sound savely.

### Server:
---
1. [ ]: Try and implement networking using this. (Thereby replacing the whole whacky netchannel which currently is doing plain wrong bad practice with its reliables and unreliables. Big game changer this would be.) https://zguide.zeromq.org/docs/chapter1/#A-Note-on-the-Naming-Convention
2. [ ]: Various cleaning up code days to be held. It hasn't been touched much ever since we extracted the client game code from the Q2Pro/Q2RTX executable binary its code.
3. [ ]: Move the material loading code outside of the refresh, or at least find a way to communicate to it. The alternative is to load the material files throughout the server game, and perhaps have footstep events being send, or keep track of what kind of a material the player is on. If it matches that with the client's, it means we can play the sound savely.
## Polyhedron's core game: Nail & Crescent DLL:
### CLGame:
---
1. [ ]: Debris/Gibs, make them client side, so there is not an amount of entities that keep being spawned server side. For this is the devil himself networking wise.
2. [ ]:
3. [ ]: 

---
### SVGame:
1. [ ]: Debris/Gibs, make them client side, so there is not an amount of entities that keep being spawned server side. For this is the devil himself networking wise.
2. [ ]: Admer/WatIsDeze finish entities.

---
### Shared(-Both) Game:
1. [ ]: None, yet, I guess. Materials in the future, for footsteps?

---