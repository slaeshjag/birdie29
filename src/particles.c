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

		if (!(*next)->next)
			break;
	}
}


void particle_render() {
	struct ParticleEntry *next;

	for (next = cs->particle; next; next = next->next) {
		d_particle_draw(next->particle);
	}
}
