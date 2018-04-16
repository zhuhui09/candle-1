
layout (location = 1) out vec2 ID;
layout (location = 2) out vec2 GeomID;

#include "common.frag"

void main()
{
	vec4 dif  = resolveProperty(diffuse, texcoord);
	if(dif.a == 0.0f) discard;

	ID = object_id;

	GeomID = poly_id;
}

// vim: set ft=c: