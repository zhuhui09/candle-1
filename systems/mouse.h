#ifndef MOUSE_H
#define MOUSE_H

#include <ecs/ecm.h>

typedef struct
{
	void *pedantic; /* Fix missing member warning */
} mouse_t;

typedef struct
{
	float x, y;
} mouse_position_data;

typedef struct
{
	float x, y, direction;
	int button;
	int clicks;
} mouse_button_data;

typedef struct
{
	float sx, sy;
	float x, y;
} mouse_move_data;

typedef struct c_mouse_t
{
	c_t super;
	int x, y;
	entity_t previous_active;
	bool_t visible;
	bool_t right;
	bool_t left;
} c_mouse_t;

void ct_mouse(ct_t *self);
DEF_CASTER(ct_mouse, c_mouse, c_mouse_t)
c_mouse_t *c_mouse_new(void);
void c_mouse_visible(c_mouse_t *self, bool_t visible);
bool_t c_mouse_active(const c_mouse_t *self);
void c_mouse_activate(c_mouse_t *self);
void c_mouse_deactivate(c_mouse_t *self);

#endif /* !MOUSE_H */
