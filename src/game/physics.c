#include "game_internal.h"

/* Update physics, main entry point.
 * I think the hero is going to be the only dynamic non-AABB body, so this will be stupid simple.
 */
 
void physics_update(struct game *game,double elapsed) {

  double herol=game->racer.x-HERO_RADIUS;
  double heror=game->racer.x+HERO_RADIUS;
  double herot=game->racer.y-HERO_RADIUS;
  double herob=game->racer.y+HERO_RADIUS;
  
  const struct sbox *sbox=game->map->sboxv;
  int sboxi=game->map->sboxc;
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
}
