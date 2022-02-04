/*
// LICENSE HERE.

//
// CoopGamemode.h
//
// Coop game mode to run, same as default mode but with coop rules.
//
*/
#include "IGamemode.h"
#include "DefaultGamemode.h"

class CoopGamemode : public DefaultGamemode {
public:
    //
    // Constructor/Deconstructor.
    //
    CoopGamemode();
    virtual ~CoopGamemode() override;

    //
    // Define as abstract class in our type system.
    //
    DefineAbstractClass("CoopGamemode", CoopGamemode);

    //
    // Functions defining game rules. Such as, CanDamage, Can... IsAllowedTo...
    //
    virtual qboolean CanDamage(SVGBaseEntity* targ, SVGBaseEntity* inflictor) override;
    // Coop has its own Obituary madness.
    virtual void ClientUpdateObituary(SVGBaseEntity* self, SVGBaseEntity* inflictor, SVGBaseEntity* attacker) override;

private:

};