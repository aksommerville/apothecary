#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "game.h"

#define TURN_SPEED 3.000 /* radian/s */
#define FLY_ACCEL 5.0 /* px/s**2 */
#define FLY_SPEED_LIMIT 60.0
#define FLY_SPEED_LIMIT_2 (FLY_SPEED_LIMIT*FLY_SPEED_LIMIT)

struct racer {
  double x,y; // pixels
  double t; // radians clockwise from UP
  double vx,vy; // inertia
};

struct game {
  struct racer racer;
  int indx; // dpad state: -1,0,1
  int accel; // button state
};

#endif
