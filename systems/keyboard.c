#include <SDL2/SDL.h>
#include "keyboard.h"

int32_t c_keyboard_event(c_keyboard_t *self, const SDL_Event *event)
{
	SDL_Keycode key;
	const entity_t target = c_entity(self) == SYS ? entity_null : c_entity(self);
	switch(event->type)
	{
		case SDL_KEYUP:
			key = event->key.keysym.sym;
			if((char)key == -32)
			{
				self->ctrl = 0;
			}
			if((char)key == -31)
			{
				self->shift = 0;
			}
			if (target == entity_null)
			{
				entity_signal(target, sig("key_up"), &key, NULL);
			}
			else
			{
				entity_signal_same(target, sig("key_up"), &key, NULL);
			}
			return STOP;
		case SDL_KEYDOWN:
			key = event->key.keysym.sym;
			if((char)key == -32)
			{
				self->ctrl = 1;
			}
			if((char)key == -31)
			{
				self->shift = 1;
			}
			if (target == entity_null)
			{
				entity_signal(target, sig("key_down"), &key, NULL);
			}
			else
			{
				entity_signal_same(target, sig("key_down"), &key, NULL);
			}
			return STOP;
	}
	return CONTINUE;
}

void ct_keyboard(ct_t *self)
{
	ct_init(self, "keyboard", sizeof(c_keyboard_t));

	signal_init(sig("key_up"), sizeof(SDL_Keycode));
	signal_init(sig("key_down"), sizeof(SDL_Keycode));

	ct_listener(self, WORLD, -1, sig("event_handle"), c_keyboard_event);
}

c_keyboard_t *c_keyboard_new()
{
	return component_new(ct_keyboard);
}

