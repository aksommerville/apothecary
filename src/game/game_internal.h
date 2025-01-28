#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "game.h"

#define TURN_SPEED 4.000 /* radian/s */
#define FLY_ACCEL 250.0 /* px/s**2 */
#define FLY_SPEED_LIMIT 300.0 /* px/s */
#define FLY_SPEED_LIMIT_2 (FLY_SPEED_LIMIT*FLY_SPEED_LIMIT)
#define HERO_RADIUS 6.0 /* px */
#define COLLISION_DAMP -0.250
#define TARGET_DISTANCE 10.0
#define TARGET_DISTANCE_2 (TARGET_DISTANCE*TARGET_DISTANCE)
#define ARROW_CLOCK_PERIOD 1.000

struct racer {
  double x,y; // world pixels
  double t; // radians clockwise from UP
  double vx,vy; // inertia
};

struct target {
  double x,y; // world pixels
  int type; // NS_target_*; zero for none
  uint8_t tileid;
};

struct game {
  struct racer racer;
  int indx; // dpad state: -1,0,1
  int accel; // button state
  int brake; // button state
  const struct map *map;
  struct target target;
  double clock; // Counts down to game over. Seconds.
  int running;
  int time_bonus; // Amount for next bonus.
  double arrowclock; // Counts down.
  int score; // ie deliveries completed
};

void physics_update(struct game *game,double elapsed);

#endif
