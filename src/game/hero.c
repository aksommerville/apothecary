#include "game_internal.h"

/* Flight sounds.
 */

static void update_flight_sounds(struct game *game) {
  if (game->clock-game->flight_sound_time<0.080) return;
  int rid=0;
  if (game->brake) rid=RID_sound_brake;
  else if (game->accel) {
    double d2=game->racer.vx*game->racer.vx+game->racer.vy*game->racer.vy;
    if (d2>=80000.0) rid=RID_sound_whooshmax;
    else if (d2>=20000.0) rid=RID_sound_whooshmid;
    else rid=RID_sound_whooshmin;
  }
  if (!rid) return;
  game->flight_sound_time=game->clock;
  play_sound(rid);
}

/* Update.
 */
 
void hero_update(struct game *game,struct racer *racer,int accel,int brake,int steer,double elapsed) {
  update_flight_sounds(game);
  
  // Turn.
  if (steer) {
    racer->t+=TURN_SPEED*elapsed*steer;
         if (racer->t<M_PI) racer->t+=M_PI*2.0;
    else if (racer->t>M_PI) racer->t-=M_PI*2.0;
  }
  
  // Decelerate due to braking or cosmic friction.
  if (!accel) {
    double rate;
    if (brake) rate=BRAKE_DECELERATION;
    else rate=NATURAL_DECELERATION;
    double mag=rate*elapsed;
    if ((racer->vm-=mag)<=0.0) {
      racer->vm=racer->vx=racer->vy=0.0;
    } else {
      racer->vx=racer->vm*sin(racer->vt);
      racer->vy=racer->vm*-cos(racer->vt);
    }
  }
  
  // If accelerating, decelerate a little perpendicular to the target direction.
  // This is where we tune the sharpness of cornering.
  if (accel&&(racer->vm>0.0)) {
    double tx=sin(racer->t);
    double ty=-cos(racer->t);
    double nx=racer->vx/racer->vm;
    double ny=racer->vy/racer->vm;
    double rej=ty*nx-tx*ny;
    double cx=rej*-ty;
    double cy=rej*tx;
    double adjvel=CORNER_TIGHTEN_FACTOR*racer->vm*elapsed;
    racer->vx+=cx*adjvel;
    racer->vy+=cy*adjvel;
    racer->vm=sqrt(racer->vx*racer->vx+racer->vy*racer->vy);
    racer->vt=atan2(racer->vx,-racer->vy);
  }
  
  // Accelerate.
  if (accel&&!brake) {
    double increase=FLY_ACCEL*elapsed;
    racer->vx+=increase*sin(racer->t);
    racer->vy-=increase*cos(racer->t);
    double v2=racer->vx*racer->vx+racer->vy*racer->vy;
    if (v2>FLY_SPEED_LIMIT_2) {
      double v=sqrt(v2);
      double adj=FLY_SPEED_LIMIT/v;
      racer->vx*=adj;
      racer->vy*=adj;
      racer->vm=FLY_SPEED_LIMIT;
    } else {
      racer->vm=sqrt(v2);
    }
    racer->vt=atan2(racer->vx,-racer->vy);
  }
  
  /* Apply hero velocity optimistically.
   * Our caller will check and apply collisions.
   */
  racer->x+=racer->vx*elapsed;
  racer->y+=racer->vy*elapsed;
}
