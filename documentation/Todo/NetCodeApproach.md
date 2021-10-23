# Approach for improving our fragmsg branch from here on.
---
## Maps:
1. [ ]: One for testing the broken misc_explobox and its physics
2. [ ]: One for testing certain movement things, did I just notice an issue or not? (Viewheight related and offset.). Also, it's good to start testing when footstep sounds play, how fast, etc.
3. [ ]: One map where we test how many entities we can pull off.
4. [ ]: Adam's map, it needs all the features that I'll be adding!

---
## CLGame:
1. [ ]: Debris/Gibs, make them client side, so there is not an amount of entities that keep being spawned server side. For this is the devil himself networking wise.
2. [ ]: Try and use huffman? decoding. Allow the option for certain vectors to be sent as a float encoded again, namely directions for random entities.
3. [ ]: 

---
## SVGame:
1. [ ]: Debris/Gibs, make them client side, so there is not an amount of entities that keep being spawned server side. For this is the devil himself networking wise.
2. [ ]: Try and use huffman? encoding. Allow the option for certain vectors to be sent as a float encoded again, namely directions for random entities.
3. [ ]: Unsure yet.

---
## Shared(-Both) Game:
1. [ ]: None, yet, I guess. Materials in the future, for footsteps?

---
# Ideas for future other branches.
## Client:
1. [ ]: Move the material loading code outside of the refresh, or at least find a way to communicate to it. The alternative is to load the material files throughout the server game, and perhaps have footstep events being send, or keep track of what kind of a material the player is on. If it matches that with the client's, it means we can play the sound savely.