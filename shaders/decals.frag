
layout (location = 0) out vec4 AlbedoColor;
layout (location = 1) out vec4 NRM; // normal_roughness_metalness

#include "common.frag"
#line 8

BUFFER {
	sampler2D depth;
	sampler2D normal;
} gbuffer;

void main()
{

	/* vec2 pos = pixel_pos(); */
	/* vec3 pos3 = textureLod(gbuffer.wposition, pos, 0).rgb ; */
	vec4 w_pos = (camera.model*vec4(get_position(gbuffer.depth), 1.0f));
	vec3 m_pos = (inverse(model) * w_pos).xyz;


	vec3 diff = abs(m_pos);
	if(diff.x > 1) discard;
	if(diff.y > 1) discard;
	if(diff.z > 1) discard;
	vec2 tc = m_pos.xy - 0.5;

	vec3 vnorm = resolveProperty(normal, tc).rgb * 2.0f - 1.0f;
	/* vnorm = vec3(0.0f, 0.0f, 1.0f); */

	vec3 norm = ((camera.view * model) * vec4(vnorm, 0.0f)).xyz;
	if(dot(norm, get_normal(gbuffer.normal)) < 0.5) discard;

	AlbedoColor = resolveProperty(albedo, tc);

	NRM.b = resolveProperty(roughness, texcoord).r;
	NRM.a = resolveProperty(metalness, texcoord).r;
	NRM.rg = encode_normal(get_normal());

}

// vim: set ft=c:
