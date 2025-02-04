#include "game_internal.h"

/* Update physics, main entry point.
 * I think the hero is going to be the only dynamic non-AABB body, so this will be stupid simple.
 */
 
void physics_update(struct game *game,double elapsed) {

  double x0=game->racer.x,y0=game->racer.y;
  double herol=x0-HERO_RADIUS;
  double heror=x0+HERO_RADIUS;
  double herot=y0-HERO_RADIUS;
  double herob=y0+HERO_RADIUS;
  
  const struct sbox *sbox=game->map->sboxv;
  int sboxi=game->map->sboxc,any=0;
  for (;sboxi-->0;sbox++) {
    if (sbox->x>=heror) continue;
    if (sbox->y>=herob) continue;
    if (sbox->x+sbox->w<=herol) continue;
    if (sbox->y+sbox->h<=herot) continue;
    
    // Select direction of escape, for cardinal cases.
    double escx=0.0,escy=0.0;
    if ((game->racer.x>=sbox->x)&&(game->racer.x<sbox->x+sbox->w)) {
      if (game->racer.y<sbox->y+sbox->h*0.5) {
        escy=-1.0;
      } else {
        escy=1.0;
      }
    } else if ((game->racer.y>=sbox->y)&&(game->racer.y<sbox->y+sbox->h)) {
      if (game->racer.x<sbox->x+sbox->w*0.5) {
        escx=-1.0;
      } else {
        escx=1.0;
      }
    } else {
      //TODO Check for corners. And it's possible there's actually no collision.
      continue;
    }
    any=1;
    
    // Clamp to the box edge per escapement.
         if (escy<0.0) game->racer.y=sbox->y-HERO_RADIUS;
    else if (escy>0.0) game->racer.y=sbox->y+sbox->h+HERO_RADIUS;
    else if (escx<0.0) game->racer.x=sbox->x-HERO_RADIUS;
    else if (escx>0.0) game->racer.x=sbox->x+sbox->w+HERO_RADIUS;
    
    // If racer's velocity on either axis disagrees with the escapement, dampen and reverse that axis.
         if ((escy<0.0)&&(game->racer.vy>0.0)) game->racer.vy*=COLLISION_DAMP;
    else if ((escy>0.0)&&(game->racer.vy<0.0)) game->racer.vy*=COLLISION_DAMP;
    else if ((escx<0.0)&&(game->racer.vx>0.0)) game->racer.vx*=COLLISION_DAMP;
    else if ((escx>0.0)&&(game->racer.vx<0.0)) game->racer.vx*=COLLISION_DAMP;
  }
  if (!any) return;
  
  double nvt=atan2(game->racer.vx,-game->racer.vy);
  double nvm=sqrt(game->racer.vx*game->racer.vx+game->racer.vy*game->racer.vy);
  game->racer.vt=nvt;
  game->racer.vm=nvm;
  
  // If we corrected substantially, play a sound effect.
  double dx=game->racer.x-x0,dy=game->racer.y-y0;
  double d2=dx*dx+dy*dy;
  // Head-on collisions at high velocity get near 20. Anything over 1 is substantial.
  if (d2>=3.0) {
    //fprintf(stderr,"HEAVY COLLISION %f\n",d2);
    play_sound(RID_sound_collheavy);
    game->bump_sound_time=game->clock;
  } else if (d2>=0.75) {
    //fprintf(stderr,"MEDIUM COLLISION %f\n",d2);
    play_sound(RID_sound_collmed);
    game->bump_sound_time=game->clock;
  } else if (d2>=0.080) {
    if (game->clock-game->bump_sound_time>=0.250) {
      //fprintf(stderr,"LIGHT COLLISION %f\n",d2);
      play_sound(RID_sound_colllight);
      game->bump_sound_time=game->clock;
    }
  }
}

/* Generic update, rectification only.
 */
 
int physics_update_1(struct game *game,double *x,double *y,double radius) {

  double x0=*x,y0=*y;
  double l=x0-radius;
  double r=x0+radius;
  double t=y0-radius;
  double b=y0+radius;
  int result=0;
  
  const struct sbox *sbox=game->map->sboxv;
  int sboxi=game->map->sboxc;
  for (;sboxi-->0;sbox++) {
    if (sbox->x>=r) continue;
    if (sbox->y>=b) continue;
    if (sbox->x+sbox->w<=l) continue;
    if (sbox->y+sbox->h<=t) continue;
    
    // Select direction of escape, for cardinal cases.
    double escx=0.0,escy=0.0;
    if ((*x>=sbox->x)&&(*x<sbox->x+sbox->w)) {
      if (*y<sbox->y+sbox->h*0.5) {
        escy=-1.0;
      } else {
        escy=1.0;
      }
    } else if ((*y>=sbox->y)&&(*y<sbox->y+sbox->h)) {
      if (*x<sbox->x+sbox->w*0.5) {
        escx=-1.0;
      } else {
        escx=1.0;
      }
    } else {
      //TODO Check for corners. And it's possible there's actually no collision.
      continue;
    }
    
    // Clamp to the box edge per escapement.
         if (escy<0.0) *y=sbox->y-radius;
    else if (escy>0.0) *y=sbox->y+sbox->h+radius;
    else if (escx<0.0) *x=sbox->x-radius;
    else if (escx>0.0) *x=sbox->x+sbox->w+radius;
    result=1;
  }
  return result;
}
