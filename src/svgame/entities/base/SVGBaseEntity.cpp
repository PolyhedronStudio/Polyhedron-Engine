/*
// LICENSE HERE.

//
// SVGBaseEntity.cpp
//
//
*/
#include "../../g_local.h"     // SVGame.
#include "../../effects.h"     // Effects.
#include "../../utils.h"       // Util funcs.
#include "SVGBaseEntity.h"

// Constructor/Deconstructor.
SVGBaseEntity::SVGBaseEntity(Entity* svEntity) : serverEntity(svEntity) {
	thinkFunction = nullptr;
	useFunction = nullptr;
	touchFunction = nullptr;
	blockedFunction = nullptr;
	takeDamageFunction = nullptr;
	dieFunction = nullptr;
}
SVGBaseEntity::~SVGBaseEntity() {

}

// Interface functions. 
void SVGBaseEntity::PreCache() {
	gi.DPrintf("SVGBaseEntity::PreCache();");
}
void SVGBaseEntity::Spawn() {
	gi.DPrintf("SVGBaseEntity::Spawn();");
}
void SVGBaseEntity::PostSpawn() {
	gi.DPrintf("SVGBaseEntity::PostSpawn();");
}
void SVGBaseEntity::Think() {
	if (thinkFunction == nullptr)
		return;

	(this->*thinkFunction)();
}

// Functions.
Entity* SVGBaseEntity::GetServerEntity() {
	return serverEntity;
}

// Executes the Die callback, if any.
void SVGBaseEntity::Die(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
	if (dieFunction == nullptr)
		return;

	(this->*dieFunction)(inflictor, attacker, damage, point);
}

// Executes the touch callback, if any.
void SVGBaseEntity::Touch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
	// Call touch function.
	if (touchFunction == nullptr)
		return;

	(this->*touchFunction)(self, other, plane, surf);
}