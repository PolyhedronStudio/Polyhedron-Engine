/***
*
*	License here.
*
*	@file
*
*	ClientGame BaseTrigger Entity.
* 
***/
#pragma once

class CLGBaseEntity;

class CLGBaseTrigger : public CLGBaseEntity {
public:
    //
    // Constructor/Deconstructor.
    //
    CLGBaseTrigger(PODEntity *clEntity);
    virtual ~CLGBaseTrigger() = default;

    DefineAbstractClass( CLGBaseTrigger, CLGBaseEntity );

    //
    // Interface functions. 
    //
    virtual void SpawnKey(const std::string& key, const std::string& value)  override;

protected:
    virtual void InitBrushTrigger();
    virtual void InitPointTrigger();
};