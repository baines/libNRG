#ifndef CLIENT_H
#define CLIENT_H

class PlayerEntity;
class BallEntity;
class EntityBase;

void clientAddEntity(PlayerEntity*);
void clientAddEntity(BallEntity*);
void clientSetScore(bool left_player, int score);
void clientDelEntity(uint16_t id);

#endif
