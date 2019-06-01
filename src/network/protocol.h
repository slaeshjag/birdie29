#ifndef __COMMON_PROTOCOL_H__
#define __COMMON_PROTOCOL_H__

#include <stdint.h>
#include "../ingame.h"
#include "../main.h"
#include "../config.h"


typedef enum PacketType PacketType;
enum PacketType {
	PACKET_TYPE_LOBBY,
	PACKET_TYPE_JOIN,
	PACKET_TYPE_TEAM,
	PACKET_TYPE_START,
	PACKET_TYPE_MOVABLE_SPAWN,
	PACKET_TYPE_MOVABLE_MOVE,
	PACKET_TYPE_MOVABLE_DESPAWN,
	PACKET_TYPE_SOUND,
	PACKET_TYPE_KEYPRESS,
	PACKET_TYPE_TILE_UPDATE,
	PACKET_TYPE_MAP_CHANGE,
	PACKET_TYPE_PARTICLE,
	PACKET_TYPE_EXPLOSION,
	PACKET_TYPE_STATUS_UPDATE,
	PACKET_TYPE_BULLET_ANNOUNCE,
	PACKET_TYPE_BULLET_UPDATE,
	PACKET_TYPE_BULLET_REMOVE,
	PACKET_TYPE_BUILD_UNIT,
	PACKET_TYPE_POWER_EVENT,
	PACKET_TYPE_EXIT,
};

typedef struct PacketLobby PacketLobby;
struct PacketLobby {
	uint16_t type;
	uint16_t size;
	
	char name[NAME_LEN_MAX];
};

typedef struct PacketJoin PacketJoin;
struct PacketJoin {
	uint16_t type;
	uint16_t size;
	
	uint32_t id;
	char name[NAME_LEN_MAX];
	int team;
	int movable;
};

typedef struct PacketStart PacketStart;
struct PacketStart {
	uint16_t type;
	uint16_t size;
	
	uint32_t player_id;
	int32_t movable;
};

typedef struct PacketKeypress PacketKeypress;
struct PacketKeypress {
	uint16_t type;
	uint16_t size;

	InGameKeyStateEntry keypress, keyrelease;
	int32_t mouse_angle;
};

typedef struct PacketParticle PacketParticle;
struct PacketParticle {
	uint16_t type;
	uint16_t size;
	
	uint32_t x;
	uint32_t y;
	uint32_t effect_type;
};

typedef struct PacketExplosion PacketExplosion;
struct PacketExplosion {
	uint16_t type;
	uint16_t size;
	
	uint32_t team;
	uint32_t x;
	uint32_t y;
};

typedef struct PacketSound PacketSound;
struct PacketSound {
	uint16_t type;
	uint16_t size;

	uint8_t sound;
};


typedef struct PacketBulletAnnounce PacketBulletAnnounce;
struct PacketBulletAnnounce {
	uint16_t type;
	uint16_t size;

	uint8_t bullet_type;
	uint16_t x;
	uint16_t y;
	uint8_t id;
};


typedef struct PacketBulletRemove PacketBulletRemove;
struct PacketBulletRemove {
	uint16_t type;
	uint16_t size;

	uint8_t id;
};


typedef struct PacketBulletUpdate PacketBulletUpdate;
struct PacketBulletUpdate {
	uint16_t type;
	uint16_t size;

	uint16_t x;
	uint16_t y;
	uint8_t id;
};


typedef struct PacketStatusUpdate PacketStatusUpdate;
struct PacketStatusUpdate {
	uint16_t type;
	uint16_t size;
	
	uint32_t money[TEAMS_CAP];
	
	uint16_t grace_timer;
	uint16_t time_to_win[TEAMS_CAP];
};


typedef struct PacketExit PacketExit;
struct PacketExit {
	uint16_t type;
	uint16_t size;
};



typedef struct PacketMovableSpawn PacketMovableSpawn;
struct PacketMovableSpawn {
	uint16_t type;
	uint16_t size;

	uint32_t movable;

	int32_t x;
	int32_t y;
	uint16_t l;
	uint16_t sprite_type;
	int16_t angle;
	int16_t dir;
};


typedef struct PacketMovableMove PacketMovableMove;
struct PacketMovableMove {
	uint16_t type;
	uint16_t size;
	
	uint32_t movable;
	
	int32_t x;
	int32_t y;
	int16_t angle;
	int16_t dir;
};


typedef struct PacketMovableDespawn PacketMovableDespawn;
struct PacketMovableDespawn {
	uint16_t type;
	uint16_t size;

	uint32_t movable;
};


typedef struct PacketMapChange PacketMapChange;
struct PacketMapChange {
	uint16_t type;
	uint16_t size;
	
	char name[MAP_NAME_LEN_MAX + 1];
	
	uint32_t w;
	uint32_t h;
};

typedef struct PacketTileUpdate PacketTileUpdate;
struct PacketTileUpdate {
	uint16_t type;
	uint16_t size;
	
	uint32_t x;
	uint32_t y;
	uint32_t tile;
	uint32_t layer;
};

typedef struct PacketBuildUnit PacketBuildUnit;
struct PacketBuildUnit {
	uint16_t type;
	uint16_t size;
	
	uint32_t unit;
};


typedef struct PacketPowerEvent PacketPowerEvent;
struct PacketPowerEvent {
	uint16_t type;
	uint16_t size;

	uint16_t x;
	uint16_t y;
	int16_t sign;
	int16_t team;
};


typedef union Packet Packet;
union Packet {
	struct {
		uint16_t type;
		uint16_t size;
		//uint8_t raw[1024];
	};
	
	PacketLobby lobby;
	PacketJoin join;
	PacketStart start;
	PacketSound sound;
	PacketKeypress keypress;
	PacketExit exit;
	PacketExplosion explosion;
	PacketParticle particle;
	PacketStatusUpdate status;
	PacketBulletUpdate bullet_update;
	PacketBulletAnnounce bullet_announce;
	PacketBulletRemove bullet_remove;
	PacketMovableSpawn movable_spawn;
	PacketMovableMove movable_move;
	PacketMovableDespawn movable_despawn;
	PacketTileUpdate tile_update;
	PacketMapChange map_change;
	PacketBuildUnit build;
	PacketPowerEvent power_event;
};

int protocol_send_packet(int sock, Packet *pack);
int protocol_recv_packet(int sock, Packet *pack);

#endif
