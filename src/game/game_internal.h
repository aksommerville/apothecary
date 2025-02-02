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
#define GIVE_UP_TIME (60.0*9.0) /* Don't let the clock get near the second minute digit. */
#define BYSTANDER_RADIUS 50.0

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

struct bystander {
  double x,y;
  uint8_t tileid; // base tileid. +16 if reacting.
  double react; // counts down while reacting.
  double dx,dy;
};

struct game {
  struct racer racer;
  int indx; // dpad state: -1,0,1
  int accel; // button state
  int brake; // button state
  const struct map *map;
  struct target target;
  double clock; // Game clock, seconds. Counts up.
  int running;
  double arrowclock; // Counts down.
  double endtime; // Counts down once !running.
  double dotanimclock;
  int dotanimframe; // 0..1. Updates constantly, even when not animated.
  struct dropoff *dropoffv; // Starts with same content as map but in a random order.
  int dropoffc; // We pick them off the end. Includes the active one.
  struct dropoff *dropoff;
  double bump_sound_time; // For regulating collision sound effects.
  double flight_sound_time;
  int pause_selp; // (0,1,2)=(not paused,resume,main menu)
  int texid_resume,w_resume,h_resume; // Pause menu labels. Language can't change during play, so we only need them once.
  int texid_menu,w_menu,h_menu;
  struct bystander *bystanderv;
  int bystanderc,bystandera;
};

void physics_update(struct game *game,double elapsed);

/* Simpler generic collision detection for one sprite.
 * (x,y) are in and out. Returns nonzero if modified.
 */
int physics_update_1(struct game *game,double *x,double *y,double radius);

#endif
