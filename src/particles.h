#ifndef PARTICLES_H_
#define	PARTICLES_H_

#include <darnit/darnit.h>


enum ParticleType {
	PARTICLE_TYPE_TEST,
};


struct ParticleEntry {
	DARNIT_PARTICLE			*particle;
	struct ParticleEntry		*next;
};


void particle_loop();
void particle_render();
void particle_add(enum ParticleType type, int x, int y, int angle);

#endif
