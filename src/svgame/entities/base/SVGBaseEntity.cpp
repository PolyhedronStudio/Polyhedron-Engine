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
	//
	// All callback functions best be nullptr.
	//
	thinkFunction = nullptr;
	useFunction = nullptr;
	touchFunction = nullptr;
	blockedFunction = nullptr;
	takeDamageFunction = nullptr;
	dieFunction = nullptr;

	//
	// Set all entity pointer references to nullptr.
	//
	activatorEntity = nullptr;
	enemyEntity = nullptr;
	groundEntity = nullptr;
	oldEnemyEntity = nullptr;
	teamChainEntity = nullptr;
	teamMasterEntity = nullptr;

	//
	// Default values for members.
	//
	moveType = MoveType::None;

	// Velocity.
	velocity = vec3_zero();
	angularVelocity = vec3_zero();
	mass = 0;
	groundEntityLinkCount = 0;
	health = 0;
	maxHealth = 0;
	deadFlag = DEAD_NO;
}
SVGBaseEntity::~SVGBaseEntity() {

}

// Interface functions. 
//
//===============
// SVGBaseEntity::Precache
//
// This function is used to load all entity data with.
//===============
//
void SVGBaseEntity::Precache() {
	//gi.DPrintf("SVGBaseEntity::Precache();");
}

//
//===============
// SVGBaseEntity::Spawn
//
// This function can be overrided, to allow for entity spawning.
// Setup the basic entity properties here.
//===============
//
void SVGBaseEntity::Spawn() {
	//gi.DPrintf("SVGBaseEntity::Spawn();");

	// Set default movetype to none.
	//SetMoveType(MoveType::None);
}

//
//===============
// SVGBaseEntity::Respawn
//
// This function can be overrided, to allow for entity respawning.
// Setup the basic entity properties here.
//===============
//
void SVGBaseEntity::Respawn() {
	//gi.DPrintf("SVGBaseEntity::Respawn();");
}

//
//===============
// SVGBaseEntity::PostSpawn
//
// This function can be overrided, to allow for entity post spawning.
// An example of that could be finding targetnames for certain target
// trigger settings, etc.
//===============
//
void SVGBaseEntity::PostSpawn() {
	//gi.DPrintf("SVGBaseEntity::PostSpawn();");
}

//
//===============
// SVGBaseEntity::Think
//
// This function can be overrided, to allow for custom entity thinking.
// By default it only executes the 'Think' callback in case we have any set.
//===============
//
void SVGBaseEntity::Think() {
	// Safety check.
	if (thinkFunction == nullptr)
		return;

	// Execute 'Think' callback function.
	(this->*thinkFunction)();
}

//
//===============
// SVGBaseEntity::ParseFloatKeyValue
//
// PROTECTED function to help parsing float key:value string pairs with.
//===============
//
qboolean SVGBaseEntity::ParseFloatKeyValue(const std::string& key, const std::string& value, float &floatNumber) {
	floatNumber = std::stof(value);

	return true;
}

//
//===============
// SVGBaseEntity::ParseIntegerKeyValue
//
// PROTECTED function to help parsing int32_t key:value string pairs with.
//===============
//
qboolean SVGBaseEntity::ParseIntegerKeyValue(const std::string& key, const std::string& value, int32_t &integerNumber) {
	integerNumber = std::stoi(value);

	return true;
}

//
//===============
// SVGBaseEntity::ParseStringKeyValue
//
// PROTECTED function to help parsing string key:value string pairs with.
//===============
//
qboolean SVGBaseEntity::ParseStringKeyValue(const std::string& key, const std::string& value, std::string& stringValue) {
	stringValue = value;

	return true;
}

//
//===============
// SVGBaseEntity::ParseVector3KeyValue
//
// PROTECTED function to help parsing vector key:value string pairs with.
//===============
//
qboolean SVGBaseEntity::ParseVector3KeyValue(const std::string& key, const std::string &value, vec3_t &vectorValue) {
	// Stores vector fields fetched from string. (Might be corrupted, so we're parsing this nicely.)
	std::vector<std::string> vectorFields;

	// We split it based on the space delimiter. Empties are okay, how can they be empty then? Good question...
	STR_Split(vectorFields, value, " ");

	// Zero out our vector.
	vectorValue = vec3_zero();
	int32_t i = 0;
	for (auto& str : vectorFields) {
		if (i > 2)
			break;

		vectorValue[i] = std::stof(str);
		i++;
	}

	// If i never reached to be 2 precisely, we failed.
	//if (i < 2) {
	//	gi.DPrintf("%s: couldn't parse '%s'\n", __func__, key.c_str());
	//	vec = vec3_zero();
	//	return false;
	//}

	return true;
}

//
//===============
// SVGBaseEntity::SpawnKey
//
// This function can be overrided, to allow for custom entity key:value parsing.
//===============
//
void SVGBaseEntity::SpawnKey(const std::string& key, const std::string& value) {

	//{"lip", STOFS(lip), F_INT},
	//{ "distance", STOFS(distance), F_INT },
	//{ "height", STOFS(height), F_INT },
	//{ "noise", STOFS(noise), F_LSTRING },
	//{ "pausetime", STOFS(pausetime), F_FLOAT },
	//{ "item", STOFS(item), F_LSTRING },

	//{ "gravity", STOFS(gravity), F_LSTRING },
	//{ "sky", STOFS(sky), F_LSTRING },
	//{ "skyrotate", STOFS(skyrotate), F_FLOAT },
	//{ "skyaxis", STOFS(skyaxis), F_VECTOR },
	//{ "minyaw", STOFS(minyaw), F_FLOAT },
	//{ "maxyaw", STOFS(maxyaw), F_FLOAT },
	//{ "minpitch", STOFS(minpitch), F_FLOAT },
	//{ "maxpitch", STOFS(maxpitch), F_FLOAT },
	//{ "nextmap", STOFS(nextMap), F_LSTRING },
	// Angle.
	if (key == "angle") {
		// Parse angle.
		float angle = 0.f;
		ParseFloatKeyValue(key, value, angle);

		// Set angle.
		SetAngles(vec3_t{ vec3_to_yaw({ angle, 0.f, 0.f }), 0.f, 0.f });
	}

	// Origin.
	if (key == "origin") {
		// Parse origin.
		vec3_t origin = vec3_zero();
		ParseVector3KeyValue(key, value, origin);

		// Set origin.
		SetOrigin(origin);
	}
}

//
//===============
// SVGBaseEntity::Use
//
// Execute the 'Use' callback in case we ran into any.
//===============
//
void SVGBaseEntity::Use(SVGBaseEntity* other, SVGBaseEntity* activator) {
	// Safety check.
	if (useFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*useFunction)(other, activator);
}

//
//===============
// SVGBaseEntity::Blocked
//
// Execute the 'Blocked' callback in case we ran into any.
//===============
//
void SVGBaseEntity::Blocked(SVGBaseEntity* other) {
	// Safety check.
	if (blockedFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*blockedFunction)(other);
}

//
//===============
// SVGBaseEntity::TakeDamage
//
// Execute the 'TakeDamage' callback in case we ran into any.
//===============
//
void SVGBaseEntity::TakeDamage(SVGBaseEntity* other, float kick, int32_t damage) {
	// Safety check.
	if (takeDamageFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*takeDamageFunction)(other, kick, damage);
}

//
//===============
// SVGBaseEntity::Die
//
// Execute the 'Die' callback in case we ran into any.
//===============
//
void SVGBaseEntity::Die(SVGBaseEntity* inflictor, SVGBaseEntity* attacker, int damage, const vec3_t& point) {
	// Safety check.
	if (dieFunction == nullptr)
		return;

	// Execute 'Die' callback function.
	(this->*dieFunction)(inflictor, attacker, damage, point);
}

//
//===============
// SVGBaseEntity::Touch
//
// Execute the 'Touch' callback in case we ran into any.
//===============
//
void SVGBaseEntity::Touch(SVGBaseEntity* self, SVGBaseEntity* other, cplane_t* plane, csurface_t* surf) {
	// Safety check.
	if (touchFunction == nullptr)
		return;

	uint32_t stateNumber = self->GetNumber();

	// Execute 'Touch' callback function.
	(this->*touchFunction)(self, other, plane, surf);
}

//
//===============
// SVGBaseEntity::LinkEntity
//
// Link entity to world for collision testing using gi.LinkEntity.
//===============
//
void SVGBaseEntity::LinkEntity() {
	gi.LinkEntity(serverEntity);
}