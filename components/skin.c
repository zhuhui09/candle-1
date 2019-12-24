#include "skin.h"
#include "bone.h"
#include <candle.h>
#include <components/spatial.h>
#include <components/node.h>
#include <components/model.h>
#include <utils/drawable.h>
#include <utils/mafs.h>
#include <utils/mesh.h>

vs_t *g_skin_vs;

static void c_skin_init(c_skin_t *self)
{
	c_model_t *mc;
	if(!g_skin_vs)
	{
		char code[1024] = "";
		strcat(code, 
			"	{\n"
			"		const vec3 colors[] = vec3[]("
						"vec3(1.,0.,0.),"
						"vec3(0.,1.,0.),"
						"vec3(0.,1.,1.),"
						"vec3(0.,0.,1.),"
						"vec3(1.,0.,1.),"
						"vec3(1.,1.,0.),"
						"vec3(0.,1.,.5),"
						"vec3(1.,.5,1.),"
						"vec3(0.,.5,1.),"
						"vec3(.5,1.,0.),"
						"vec3(.3,.0,0.)"
			"	);\n"
		);
		strcat(code,
			"#ifdef MESH4\n"
			"		float Y = cos(angle4);\n"
			"		float W = sin(angle4);\n"
			"		pos = vec4(vec3(P.x, P.y * Y + P.w * W, P.z), 1.0);\n"
			"#endif\n"
			"		mat4 MODIFIER;\n"
			"		float wei = WEI[0] + WEI[1] + WEI[2] + WEI[3];\n"
			"		if(wei > 0.0) {\n"
			"			mat4 t = skin.bones[int(BID[0])] * WEI[0];\n"
			"			t += skin.bones[int(BID[1])] * WEI[1];\n"
			"			t += skin.bones[int(BID[2])] * WEI[2];\n"
			"			t += skin.bones[int(BID[3])] * WEI[3];\n"
			"			MODIFIER = t;\n"
			"		} else\n"
			"		    MODIFIER = M;\n"
		);
		strcat(code,
			"		MODIFIER = camera(view) * MODIFIER;\n"

			"		vec3 vertex_normal    = normalize(MODIFIER * vec4( N, 0.0)).xyz;\n"
			"		vec3 vertex_tangent   = normalize(MODIFIER * vec4(TG, 0.0)).xyz;\n"
			"		vec3 vertex_bitangent = cross(vertex_tangent, vertex_normal);\n"
			"		pos = (MODIFIER * pos);\n"
			"		$vertex_position = pos.xyz;\n"

			"		$TM = mat3(vertex_tangent, vertex_bitangent, vertex_normal);\n"

			"		pos = camera(projection) * pos;\n"
			"	}\n"
		);
		g_skin_vs = vs_new("skin", true, 1, vertex_modifier_new(code));
	}
	mc = c_model(self);
	if(!mc) return;
	/* drawable_set_vs(&mc->draw, model_vs()); */
	drawable_set_vs(&mc->draw, g_skin_vs);
	drawable_set_skin(&mc->draw, &self->info);
}

void c_skin_changed(c_skin_t *self)
{
	self->modified = 1;
}

int c_skin_update_transforms(c_skin_t *self)
{
	uint32_t i;
	if(!self->modified) return CONTINUE; 
	self->modified = 0;
	for(i = 0; i < self->info.bones_num; i++)
	{
		c_bone_t *bone = c_bone(&self->info.bones[i]);
		c_node_t *node = c_node(bone);
		c_node_update_model(node);

		self->info.transforms[i] = mat4_mul(node->model, bone->offset);
	}
	self->info.update_id++;
	return CONTINUE;
}

void ct_skin(ct_t *self)
{
	ct_init(self, "skin", sizeof(c_skin_t));
	ct_set_init(self, (init_cb)c_skin_init);
	ct_dependency(self, ct_node);
	ct_listener(self, WORLD, 0, sig("world_update"), c_skin_update_transforms);
}

c_skin_t *c_skin_new()
{
	c_skin_t *self = component_new(ct_skin);
	self->info.update_id = 1;
	return self;
}

