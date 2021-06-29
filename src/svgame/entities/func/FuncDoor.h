#pragma once

class SVGBaseMover;

//===============
// A standard, sliding door
//===============
class FuncDoor : public SVGBaseMover
{
public:
    FuncDoor( Entity* entity );
    virtual ~FuncDoor() = default;

    // Spawn flags
    static constexpr int32_t SF_StartOpen   = 1 << 0;
    static constexpr int32_t SF_Reverse     = 1 << 1;
    static constexpr int32_t SF_Crusher     = 1 << 2;
    static constexpr int32_t SF_NoMonsters  = 1 << 3;
    static constexpr int32_t SF_Toggle      = 1 << 4;
    static constexpr int32_t SF_XAxis       = 1 << 5;
    static constexpr int32_t SF_YAxis       = 1 << 6;

    void		Precache() override;
	void		Spawn() override;
    void        PostSpawn() override;
	//void		SpawnKey( const std::string& key, const std::string& value ) override;

protected:
    void        CalculateMoveSpeed();
    void        SpawnDoorTrigger();
    void        UseAreaportals( bool open ) const;
};
