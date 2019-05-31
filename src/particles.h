#ifndef PARTICLES_H_
#define	PARTICLES_H_

#include <darnit/darnit.h>


struct ParticleEntry {
	DARNIT_PARTICLE			*particle;
	struct ParticleEntry		*next;
};


void particle_loop();
void particle_render();

#endif
