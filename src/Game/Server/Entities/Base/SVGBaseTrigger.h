/***
*
*	License here.
*
*	@file
*
*	ServerGame BaseTrigger Entity.
* 
***/
#pragma once

// Required parent class include.
#include "Game/Server/Entities/Base/SVGBaseEntity.h"

class SVGBaseTrigger : public SVGBaseEntity {
public:
    //
    // Constructor/Deconstructor.
    //
    SVGBaseTrigger(PODEntity *svEntity);
    virtual ~SVGBaseTrigger() = default;

    DefineAbstractClass( SVGBaseTrigger, SVGBaseEntity );

    //
    // Interface functions. 
    //
    virtual void SpawnKey(const std::string& key, const std::string& value)  override;

protected:
    virtual void InitBrushTrigger();
    virtual void InitPointTrigger();
};