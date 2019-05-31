#ifndef SERVER_H_
#define SERVER_H_

void server_start();
void server_start_game();
void server_kick();
void server_announce_winner(int winning_player);
//void server_sound(enum SoundeffectSound sound);
void server_broadcast_packet(Packet *pack);
bool server_player_is_present(int id);
void server_shutdown();

#endif
