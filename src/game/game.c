#include "game_internal.h"

/* Delete.
 */
 
void game_del(struct game *game) {
  if (!game) return;
  free(game);
}

/* Read the initial map commands.
 */
 
static int game_process_map_commands(struct game *game) {
  int heroc=0,pickupc=0;
  struct rom_command_reader reader={.v=game->map->cmdv,.c=game->map->cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_hero: {
          heroc++;
          game->racer.x=(cmd.argv[0]+0.5)*NS_sys_tilesize;
          game->racer.y=(cmd.argv[1]+0.5)*NS_sys_tilesize;
        } break;
      case CMD_map_pickup: {
          pickupc++;
          game->target.x=(cmd.argv[0]+0.5)*NS_sys_tilesize;
          game->target.y=(cmd.argv[1]+0.5)*NS_sys_tilesize;
          game->target.type=NS_target_pickup;
          game->target.tileid=0x12; //TODO Will this always be the same thing? If not, how to select?
        } break;
    }
  }
  if ((heroc!=1)||(pickupc!=1)||!game->map->dropoffc) {
    fprintf(stderr,"map:%d invalid: heroc=%d pickupc=%d dropoffc=%d\n",game->map->id,heroc,pickupc,game->map->dropoffc);
    return -1;
  }
  return 0;
}

/* Rebuild dropoffv from the map.
 */
 
static int game_rebuild_dropoffv(struct game *game) {
  if (game->map->dropoffc<1) return -1;
  if (!(game->dropoffv=malloc(sizeof(struct dropoff)*game->map->dropoffc))) return -1;
  game->dropoffc=0;
  const struct dropoff *src=game->map->dropoffv;
  int i=game->map->dropoffc;
  for (;i-->0;src++) {
    int p=rand()%(game->dropoffc+1);
    memmove(game->dropoffv+p+1,game->dropoffv+p,sizeof(struct dropoff)*(game->dropoffc-p));
    memcpy(game->dropoffv+p,src,sizeof(struct dropoff));
    game->dropoffc++;
  }
  game->dropoff=game->dropoffv+game->dropoffc-1;
  return 0;
}

/* New.
 */
 
struct game *game_new() {
  struct game *game=calloc(1,sizeof(struct game));
  if (!game) return 0;
  
  if (
    !(game->map=map_get(RID_map_scratch))||
    (game_process_map_commands(game)<0)||
    (game_rebuild_dropoffv(game)<0)
  ) {
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
  
  /* If we were dropping off, move to the next dropoff or win.
   * If there's another one, set target to the apothecary.
   */
  if (game->target.type==NS_target_dropoff) {
    game->dropoffc--;
    if (game->dropoffc<=0) {
      game->target.type=0;
      game->running=0;
      game->endtime=GAME_END_TIME;
      set_hiscore(game->clock);
      egg_play_song(RID_song_emotional_support_bird,0,1);
    } else {
      game->dropoff=game->dropoffv+game->dropoffc-1;
      game->target.type=NS_target_pickup;
      game->target.tileid=0x12; // TODO variety?
      game->target.x=(game->map->pickupx+0.5)*NS_sys_tilesize;
      game->target.y=(game->map->pickupy+0.5)*NS_sys_tilesize;
    }
  
  /* If we were picking up, set target to the current dropoff.
   */
  } else {
    game->target.type=NS_target_dropoff;
    game->target.tileid=game->dropoff->tileid;
    game->target.x=(game->dropoff->x+0.5)*NS_sys_tilesize;
    game->target.y=(game->dropoff->y+0.5)*NS_sys_tilesize;
  }
}

/* Update.
 */
 
int game_update(struct game *game,double elapsed) {

  /* Advance clocks.
   */
  if (game->running) {
    game->clock+=elapsed;
    if (game->clock>GIVE_UP_TIME) {
      game->running=0;
      game->endtime=GAME_END_TIME;
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
    if (game->accel&&game->indx) {
      // If you turn while accelerating, first reduce velocity just a hair.
      // This is a penalty for turning, but also makes turns tighter.
      double loss=TURN_DECELERATION*elapsed;
      double v2=game->racer.vx*game->racer.vx+game->racer.vy*game->racer.vy;
      double v=sqrt(v2);
      if (loss>=v) {
        game->racer.vx=0.0;
        game->racer.vy=0.0;
      } else {
        double adj=(v-loss)/v;
        game->racer.vx*=adj;
        game->racer.vy*=adj;
      }
    }
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
    }
  }
  
  /* If the brake is on, accelerator is off, or game is ended, decelerate.
   */
  if (!game->running||game->brake||!game->accel) {
    double v2=game->racer.vx*game->racer.vx+game->racer.vy*game->racer.vy;
    if (v2>0.0) {
      double v=sqrt(v2);
      double rate;
      if (!game->running) rate=NATURAL_DECELERATION;
      else if (game->brake) rate=BRAKE_DECELERATION;
      else rate=NATURAL_DECELERATION;
      double loss=rate*elapsed;
      if (loss>=v) {
        game->racer.vx=0.0;
        game->racer.vy=0.0;
      } else {
        double adj=(v-loss)/v;
        game->racer.vx*=adj;
        game->racer.vy*=adj;
      }
    }
  }
  
  /* Apply hero velocity optimistically.
   */
  game->racer.x+=game->racer.vx*elapsed;
  game->racer.y+=game->racer.vy*elapsed;
  
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
