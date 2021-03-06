#include "particles.h"
#include "main.h"
// Codename: megapulsar

void particle_loop() {
	struct ParticleEntry **next, *del;

	for (next = &cs->particle; (*next); next = &(*next)->next) {
		if (d_particle_used((*next)->particle) <= 0) {
			d_particle_free((*next)->particle);
			del = *next;
			*next = (*next)->next;
			free(del);
		}
		
		if (!*next)
			break;
	}
}


void particle_render() {
	struct ParticleEntry *next;

	for (next = cs->particle; next; next = next->next) {
		d_particle_draw(next->particle);
	}
}


void particle_add(enum ParticleType type, int x, int y, int angle) {
	struct ParticleEntry *pe;

	pe = malloc(sizeof(*pe));
	if (type == PARTICLE_TYPE_TEST) {
		pe->particle = d_particle_new(200, DARNIT_PARTICLE_TYPE_POINT);
		d_particle_color_start(pe->particle, 200, 200, 0, 255);
		d_particle_color_target(pe->particle, 255, 0, 0, 0);
		d_particle_emitter_move(pe->particle, x, y);
		d_particle_emitter_angle(pe->particle, angle - 30, angle + 30);
		d_particle_emitter_velocity(pe->particle, 20, 200);
		d_particle_life(pe->particle, 100);
		d_particle_mode(pe->particle, DARNIT_PARTICLE_MODE_PULSAR);
		d_particle_point_size(pe->particle, 5);
		d_particle_pulse(pe->particle);
	} else if (type == PARTICLE_TYPE_BLOOD) {
		pe->particle = d_particle_new(1000, DARNIT_PARTICLE_TYPE_POINT);
		d_particle_color_start(pe->particle, 128, 0, 0, 255);
		d_particle_color_target(pe->particle, 128, 0, 0, 0);
		d_particle_life(pe->particle, 1000);
		d_particle_mode(pe->particle, DARNIT_PARTICLE_MODE_PULSAR);
		d_particle_emitter_velocity(pe->particle, 100, 1500);
		d_particle_emitter_move(pe->particle, x, y);
		d_particle_emitter_angle(pe->particle, 0, 3600);
		d_particle_point_size(pe->particle, 10);
		d_particle_pulse(pe->particle);
	} else if (type == PARTICLE_TYPE_EXPLOSION) {
		pe->particle = d_particle_new(1000, DARNIT_PARTICLE_TYPE_POINT);
		d_particle_color_start(pe->particle, 255, 255, 0, 255);
		d_particle_color_target(pe->particle, 128, 32, 32, 0);
		d_particle_life(pe->particle, 1000);
		d_particle_mode(pe->particle, DARNIT_PARTICLE_MODE_PULSAR);
		d_particle_emitter_velocity(pe->particle, 100, 1500);
		d_particle_emitter_move(pe->particle, x, y);
		d_particle_emitter_angle(pe->particle, 0, 3600);
		d_particle_point_size(pe->particle, 10);
		d_particle_pulse(pe->particle);
	} else {
		free(pe);
		return;
	}

	pe->next = cs->particle;
	cs->particle = pe;

	return;
}
