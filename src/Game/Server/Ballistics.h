#pragma once

/**
*   @brief  Fires a single 9mm bullet.
**/
void SVG_FireBullet(SVGBasePlayer *player, const vec3_t& start, const vec3_t& aimDirection, int32_t damage, int32_t kickForce, int32_t horizontalSpread, int32_t verticalSpread, int32_t meansOfDeath);