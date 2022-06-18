#pragma once

class SVGBaseEntity;

class PathMonsterGoal : public SVGBaseEntity {
public:
	PathMonsterGoal( Entity* entity );
	virtual ~PathMonsterGoal() = default;

	DefineMapClass( "path_monster_goal", PathMonsterGoal, SVGBaseEntity );

	// Spawnflags
	static constexpr int32_t SF_Teleport = 1 << 0;

	void			Spawn() override;
	void			SpawnKey( const std::string& key, const std::string& value ) override;

    void PathMonsterGoalTouch( IServerGameEntity* self, IServerGameEntity* other, CollisionPlane* plane, CollisionSurface* surf );
    //void PathMonsterGoalEnable(IServerGameEntity* other, IServerGameEntity* activator);
    void PathMonsterGoalUse( IServerGameEntity* other, IServerGameEntity* activator );

	inline const std::string GetMonstersString() {
		return strMonsters;
	}

private:
	//! When set, will make the monster prepare to navigate to its next
	//! goal.
	std::string strNextGoal = "";
	//! Determines which monsters can trigger this goal on 'touch'.
	//! When empty, all monsters will trigger this goal on touch.
	std::string	strMonsters = "";
};
