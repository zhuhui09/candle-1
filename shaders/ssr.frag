
#include "common.frag"
#line 4

layout (location = 0) out vec4 FragColor;
uniform pass_t rendered;

void main()
{
	vec4 cc = pass_sample(rendered, pixel_pos());

	vec4 refl = get_specular(gbuffer);
	vec4 ssred = ssr(rendered.diffuse);

	vec3 final = cc.rgb + ssred.rgb * ssred.a * refl.rgb;

	/* FragColor = vec4(cc.xyz, 1.0f); return; */
	/* FragColor = vec4(ssred.rgb, 1.0f); return; */

	/* final = clamp(final * 1.6f - 0.10f, 0.0, 3.0); */
	final = final * pow(2.0f, camera.exposure);
	float dist = length(get_position(gbuffer));
	final.b += (clamp(dist - 5, 0, 1)) / 70;

	FragColor = vec4(final, 1.0f);
}

// vim: set ft=c:
