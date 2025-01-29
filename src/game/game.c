#include "game_internal.h"

/* Delete.
 */
 
void game_del(struct game *game) {
  if (!game) return;
  free(game);
}

/* New.
 */
 
struct game *game_new() {
  struct game *game=calloc(1,sizeof(struct game));
  if (!game) return 0;
  
  game->racer.x=FBW*0.5;//TODO starting position from map
  game->racer.y=FBH*0.5;
  game->racer.t=0.0;
  
  game->target.x=600.0;//TODO starting position from map
  game->target.y=400.0;
  game->target.type=NS_target_pickup;
  game->target.tileid=0x12;
  
  game->clock=30.0;
  game->time_bonus=20;
  game->score=0;
  
  if (!(game->map=map_get(RID_map_scratch))) {
    game_del(game);
    return 0;
  }
  
  game->running=1;
  
  egg_play_song(RID_song_race_to_the_bottom,0,1);
  
  return game;
}

/* Input changed.
 */
 
void game_input(struct game *game,int input,int pvinput) {
  if ((input&EGG_BTN_LEFT)&&!(pvinput&EGG_BTN_LEFT)) game->indx=-1;
  else if ((game->indx<0)&&!(input&EGG_BTN_LEFT)) game->indx=0;
  if ((input&EGG_BTN_RIGHT)&&!(pvinput&EGG_BTN_RIGHT)) game->indx=1;
  else if ((game->indx>0)&&!(input&EGG_BTN_RIGHT)) game->indx=0;
  game->accel=input&EGG_BTN_SOUTH;
  game->brake=input&EGG_BTN_WEST;
}

/* Reached the target.
 */
 
static void target_reached(struct game *game) {
  if (game->time_bonus>0) {
    //TODO Report time bonus.
    game->clock+=game->time_bonus;
    game->time_bonus--;
  } else {
    //TODO Report "no time bonus"?
  }
  //TODO decide where to put the next one
  int col,row;
  do {
    col=rand()%game->map->w;
    row=rand()%game->map->h;
  } while (game->map->physics[game->map->v[row*game->map->w+col]]==NS_physics_solid);
  game->target.x=(col+0.5)*NS_sys_tilesize;
  game->target.y=(row+0.5)*NS_sys_tilesize;
  if (game->target.type==NS_target_pickup) {
    game->target.type=NS_target_dropoff;
    game->target.tileid=0x13;
  } else {
    game->score++;
    game->target.type=NS_target_pickup;
    game->target.tileid=0x12;
  }
}

/* Update.
 */
 
int game_update(struct game *game,double elapsed) {

  /* Advance clocks.
   */
  if (game->running) {
    if ((game->clock-=elapsed)<=0.0) {
      game->clock=0.0;
      game->running=0;
      game->endtime=GAME_END_TIME;
      set_hiscore(game->score);
      egg_play_song(RID_song_emotional_support_bird,0,1);
    }
  }
  if ((game->arrowclock-=elapsed)<=0.0) {
    game->arrowclock+=ARROW_CLOCK_PERIOD;
  }
  if (!game->running) {
    if ((game->endtime-=elapsed)<=0.0) {
      return -1;
    }
  }
  if ((game->dotanimclock-=elapsed)<=0.0) {
    game->dotanimclock+=0.125;
    if (++(game->dotanimframe)>=2) game->dotanimframe=0;
  }

  /* Poll input. Update hero's angle and velocity.
   */
  if (game->running) {
    if (game->indx) {
      game->racer.t+=TURN_SPEED*elapsed*game->indx;
      if (game->racer.t>M_PI) game->racer.t-=M_PI*2.0;
      else if (game->racer.t<-M_PI) game->racer.t+=M_PI*2.0;
    }
    if (game->accel) {
      double mag=FLY_ACCEL*elapsed;
      game->racer.vx+=mag*sin(game->racer.t);
      game->racer.vy+=mag*-cos(game->racer.t);
      double v2=game->racer.vx*game->racer.vx+game->racer.vy*game->racer.vy;
      if (v2>FLY_SPEED_LIMIT_2) {
        double v=sqrt(v2);
        double adj=FLY_SPEED_LIMIT/v;
        game->racer.vx*=adj;
        game->racer.vy*=adj;
      }
    } else if (game->brake) {
      game->racer.vx*=0.940;
      game->racer.vy*=0.940;
    } else {
      game->racer.vx*=0.980;//TODO proper deceleration... involves a pow() somehow
      game->racer.vy*=0.980;
    }
  } else {
    // Game ended. Stop updating per input, and just wind down velocity.
    game->racer.vx*=0.980;
    game->racer.vy*=0.980;
  }
  
  /* Apply hero velocity and clamp to world.
   */
  game->racer.x+=game->racer.vx*elapsed;
  game->racer.y+=game->racer.vy*elapsed;
  double worldw=(double)(game->map->w*NS_sys_tilesize);
  double worldh=(double)(game->map->h*NS_sys_tilesize);
  if (game->racer.x<0.0) game->racer.x=0.0; else if (game->racer.x>worldw) game->racer.x=worldw;
  if (game->racer.y<0.0) game->racer.y=0.0; else if (game->racer.y>worldh) game->racer.y=worldh;
  
  /* General physics.
   * (in truth, it only corrects hero against static geometry, it's very simple).
   */
  physics_update(game,elapsed);
  
  /* Check whether we hit the target.
   */
  if (game->running&&game->target.type) {
    double dx=game->racer.x-game->target.x;
    double dy=game->racer.y-game->target.y;
    double d2=dx*dx+dy*dy;
    if (d2<TARGET_DISTANCE_2) {
      target_reached(game);
    }
  }
  
  return 0;
}
