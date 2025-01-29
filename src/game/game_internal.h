#ifndef GAME_INTERNAL_H
#define GAME_INTERNAL_H

#include "game.h"

#define TURN_SPEED 4.000 /* radian/s */
#define FLY_ACCEL 250.0 /* px/s**2 */
#define FLY_SPEED_LIMIT 300.0 /* px/s */
#define FLY_SPEED_LIMIT_2 (FLY_SPEED_LIMIT*FLY_SPEED_LIMIT)
#define NATURAL_DECELERATION 200.0 /* px/s**2 */
#define BRAKE_DECELERATION 500.0 /* px/s**2 */
#define TURN_DECELERATION 100.0 /* px/s**2 */
#define HERO_RADIUS 6.0 /* px */
#define COLLISION_DAMP -0.250
#define TARGET_DISTANCE 10.0
#define TARGET_DISTANCE_2 (TARGET_DISTANCE*TARGET_DISTANCE)
#define ARROW_CLOCK_PERIOD 1.000
#define GAME_END_TIME 3.0
#define GAME_END_FADE_TIME 1.0
#define BONUS_TOAST_TTL 1.5

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

struct bonus_toast {
  double x,y; // world pixels
  int v; // seconds
  double ttl;
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
  double endtime; // Counts down once !running.
  double dotanimclock;
  int dotanimframe; // 0..1. Updates constantly, even when not animated.
  struct bonus_toast bonus_toast;
};

void physics_update(struct game *game,double elapsed);

#endif
