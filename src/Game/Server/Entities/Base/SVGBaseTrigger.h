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