#pragma once

class SVGBaseEntity;

class PathCorner : public SVGBaseEntity {
public:
	PathCorner( Entity* entity );
	virtual ~PathCorner() = default;

	DefineMapClass( "path_corner", PathCorner, SVGBaseEntity );

	const vec3_t	BboxSize = vec3_t( 8.0f, 8.0f, 8.0f );

	void			Spawn() override;
	virtual void	OnReachedCorner( SVGBaseEntity* ent );
};

