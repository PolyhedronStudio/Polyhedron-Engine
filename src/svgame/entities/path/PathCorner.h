#pragma once

class SVGBaseEntity;

class PathCorner : public SVGBaseEntity {
public:
	PathCorner( Entity* entity );
	virtual ~PathCorner() = default;

	DefineMapClass( "path_corner", PathCorner, SVGBaseEntity );

	void Spawn() override;
};