#include "game_internal.h"

/* Delete.
 */
 
void game_del(struct game *game) {
  if (!game) return;
  if (game->dropoffv) free(game->dropoffv);
  if (game->bystanderv) free(game->bystanderv);
  egg_texture_del(game->texid_resume);
  egg_texture_del(game->texid_menu);
  free(game);
}

/* Read the initial map commands.
 */
 
static int game_process_map_commands(struct game *game) {
  int heroc=0,pickupc=0;
  struct cmdlist_reader reader={.v=game->map->cmdv,.c=game->map->cmdc};
  struct cmdlist_entry cmd;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_hero: {
          heroc++;
          game->racer.x=(cmd.arg[0]+0.5)*NS_sys_tilesize;
          game->racer.y=(cmd.arg[1]+0.5)*NS_sys_tilesize;
        } break;
      case CMD_map_pickup: {
          pickupc++;
          game->target.x=(cmd.arg[0]+0.5)*NS_sys_tilesize;
          game->target.y=(cmd.arg[1]+0.5)*NS_sys_tilesize;
          game->target.type=NS_target_pickup;
          game->target.tileid=0x12;
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

/* Rebuild list of bystanders.
 */
 
static void game_init_bystander(struct game *game,struct bystander *bystander) {
  int panic=100;
  for (;;) {
    if (--panic<0) { // Haven't found a spot yet? Put him way offscreen and forget him.
      bystander->x=-999.0;
      bystander->y=-999.0;
      return;
    }
    int col=rand()%game->map->w;
    int row=rand()%game->map->h;
    uint8_t physics=game->map->physics[game->map->v[row*game->map->w+col]];
    if (physics==NS_physics_vacant) { // seems like a reasonable place to park...
      bystander->x=(col+0.5)*NS_sys_tilesize;
      bystander->y=(row+0.5)*NS_sys_tilesize;
      bystander->react=0.0;
      switch (rand()%4) {
        case 0: bystander->tileid=0x0f; break;
        case 1: bystander->tileid=0x2f; break;
        case 2: bystander->tileid=0x4f; break;
        case 3: bystander->tileid=0x6f; break;
      }
      return;
    }
  }
}
 
static int game_rebuild_bystanderv(struct game *game) {
  const int count=100;
  game->bystanderc=0;
  if (game->bystandera<count) {
    void *nv=realloc(game->bystanderv,sizeof(struct bystander)*count);
    if (!nv) return -1;
    game->bystanderv=nv;
    game->bystandera=count;
  }
  while (game->bystanderc<game->bystandera) {
    struct bystander *bystander=game->bystanderv+game->bystanderc++;
    game_init_bystander(game,bystander);
  }
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
    (game_rebuild_dropoffv(game)<0)||
    (game_rebuild_bystanderv(game)<0)
  ) {
    game_del(game);
    return 0;
  }
  
  game->running=1;
  
  play_song(RID_song_race_to_the_bottom);
  
  return game;
}

/* Input changed.
 */
 
void game_input(struct game *game,int input,int pvinput) {

  if ((input&EGG_BTN_AUX1)&&!(pvinput&EGG_BTN_AUX1)) {
    play_sound(RID_sound_uiactivate);
    if (game->pause_selp) game->pause_selp=0;
    else game->pause_selp=1;
  }
  
  if (game->pause_selp) {
    game->indx=game->accel=game->brake=0;
    if ((input&EGG_BTN_UP)&&!(pvinput&EGG_BTN_UP)) { play_sound(RID_sound_uimotion); game->pause_selp--; }
    if ((input&EGG_BTN_DOWN)&&!(pvinput&EGG_BTN_DOWN)) { play_sound(RID_sound_uimotion); game->pause_selp++; }
    if (game->pause_selp<1) game->pause_selp=2; else if (game->pause_selp>2) game->pause_selp=1;
    if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) {
      play_sound(RID_sound_uiactivate);
      if (game->pause_selp==1) { // Resume.
        game->pause_selp=0;
      } else { // Main menu.
        game->running=0;
        game->endtime=0.0;
        g.skip_next_gameover=1;
      }
    }
    return;
  }

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
    play_sound(RID_sound_deliver);
    if (game->dropoffc<=0) {
      game->target.type=0;
      game->running=0;
      game->endtime=GAME_END_TIME;
      set_hiscore(game->clock);
      play_song(RID_song_emotional_support_bird);
    } else {
      game->dropoff=game->dropoffv+game->dropoffc-1;
      game->target.type=NS_target_pickup;
      game->target.tileid=0x12;
      game->target.x=(game->map->pickupx+0.5)*NS_sys_tilesize;
      game->target.y=(game->map->pickupy+0.5)*NS_sys_tilesize;
    }
  
  /* If we were picking up, set target to the current dropoff.
   */
  } else {
    play_sound(RID_sound_pickup);
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
  if (game->running&&!game->pause_selp) {
    game->clock+=elapsed;
    if (game->clock>GIVE_UP_TIME) {
      game->running=0;
      game->endtime=GAME_END_TIME;
      play_song(RID_song_emotional_support_bird);
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
  if (game->running&&!game->pause_selp) {
    hero_update(game,&game->racer,game->accel,game->brake,game->indx,elapsed);
  }
  
  /* If the brake is on, accelerator is off, or game is ended, decelerate.
   */
  if (!game->pause_selp) {
  
    /* General physics.
     * (in truth, it only corrects hero against static geometry, it's very simple).
     */
    physics_update(game,elapsed);
    
    /* Update bystanders.
     */
    double dotvel2=game->racer.vx*game->racer.vx+game->racer.vy*game->racer.vy;
    struct bystander *bystander=game->bystanderv;
    int i=game->bystanderc;
    for (;i-->0;bystander++) {
      if (bystander->react>0.0) {
        bystander->react-=elapsed;
        bystander->x+=bystander->dx*elapsed;
        bystander->y+=bystander->dy*elapsed;
        physics_update_1(game,&bystander->x,&bystander->y,7.0);
        if (bystander->react<=0.0) { // Check for water.
          int col=(int)bystander->x/NS_sys_tilesize;
          int row=(int)bystander->y/NS_sys_tilesize;
          if ((col>=0)&&(row>=0)&&(col<game->map->w)&&(row<game->map->h)) {
            uint8_t physics=game->map->physics[game->map->v[row*game->map->w+col]];
            if (physics==NS_physics_water) {
              //TODO splash sound and visual effect?
              bystander->x=-999.0;
              bystander->y=-999.0;
            }
          }
        }
      } else if (bystander->react>-0.500) {
        // A little extra delay before we react again.
        bystander->react-=elapsed;
      } else if (dotvel2<10000.0) {
        // Don't react at low velocities.
      } else {
        double dx=bystander->x-game->racer.x;
        if ((dx<-BYSTANDER_RADIUS)||(dx>BYSTANDER_RADIUS)) continue;
        double dy=bystander->y-game->racer.y;
        if ((dy<-BYSTANDER_RADIUS)||(dy>BYSTANDER_RADIUS)) continue;
        double d=sqrt(dx*dx+dy*dy);
        if (d>BYSTANDER_RADIUS) continue;
        play_sound(RID_sound_whoa);
        bystander->react=0.300;
        // Reaction direction is perpendicular to dot's motion, whichever way is further.
        double dotvel=sqrt(dotvel2);
        double dotnormx=game->racer.vx/dotvel;
        double dotnormy=game->racer.vy/dotvel;
        double pdx=-dotnormy;
        double pdy=dotnormx;
        double rej=dx*dotnormy-dy*dotnormx;
        if (rej>0.0) { pdx*=-1.0; pdy*=-1.0; }
        const double speed=100.0;
        bystander->dx=pdx*speed;
        bystander->dy=pdy*speed;
      }
    }
  
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
  }
  
  return 0;
}
