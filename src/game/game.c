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
      egg_play_song(RID_song_emotional_support_bird,0,1);
    }
  }
  if ((game->arrowclock-=elapsed)<=0.0) {
    game->arrowclock+=ARROW_CLOCK_PERIOD;
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

/* Render.
 */
 
void game_render(struct game *game) {
  
  /* Calculate camera position.
   * Center on the hero, then clamp to the map.
   * Or if the world is narrower than the screen (shouldn't ever happen), center it and blackout first.
   * Maps must be at least as large as the screen -- we'll design them that way.
   */
  int worldw=game->map->w*NS_sys_tilesize;
  int worldh=game->map->h*NS_sys_tilesize;
  int herox=(int)game->racer.x;
  int heroy=(int)game->racer.y;
  int camerax=herox-(FBW>>1);
  int cameray=heroy-(FBH>>1);
  int blackout=0;
  if (worldw<FBW) blackout=1;
  else if (camerax<0) camerax=0;
  else if (camerax+FBW>worldw) camerax=worldw-FBW;
  if (worldh<FBH) blackout=1;
  else if (cameray<0) cameray=0;
  else if (cameray+FBH>worldh) cameray=worldh-FBH;
  
  if (blackout) graf_draw_rect(&g.graf,0,0,FBW,FBH,0x000000ff);
  
  /* Draw background grid.
   */
  int cola=camerax/NS_sys_tilesize;
  int rowa=cameray/NS_sys_tilesize;
  if (cola<0) cola=0;
  if (rowa<0) rowa=0;
  int colz=(camerax+FBW-1)/NS_sys_tilesize;
  int rowz=(cameray+FBH-1)/NS_sys_tilesize;
  if (colz>=game->map->w) colz=game->map->w-1;
  if (rowz>=game->map->h) rowz=game->map->h-1;
  graf_draw_tile_buffer(&g.graf,g.texid_tiles,
    cola*NS_sys_tilesize+(NS_sys_tilesize>>1)-camerax,
    rowa*NS_sys_tilesize+(NS_sys_tilesize>>1)-cameray,
    game->map->v+rowa*game->map->w+cola,
    colz-cola+1,rowz-rowa+1,game->map->w
  );
  
  /* Target.
   */
  int targetx=999,targety=0;
  if (game->target.type) {
    targetx=(int)game->target.x-camerax;
    targety=(int)game->target.y-cameray;
    graf_draw_tile(&g.graf,g.texid_tiles,targetx,targety,game->target.tileid,0);
  }
  
  /* Draw the hero.
   */
  herox-=camerax;
  heroy-=cameray;
  graf_set_alpha(&g.graf,0x80);
  graf_set_tint(&g.graf,0x000000ff);
  graf_draw_mode7(&g.graf,g.texid_hero,
    herox,heroy+4,
    0,0,NS_sys_tilesize*3,NS_sys_tilesize*3,0.5f,0.5f,
    game->racer.t,1
  );
  graf_set_alpha(&g.graf,0xff);
  graf_set_tint(&g.graf,0);
  graf_draw_mode7(&g.graf,g.texid_hero,
    herox,heroy,
    0,0,NS_sys_tilesize*3,NS_sys_tilesize*3,0.5f,0.5f,
    game->racer.t,1
  );
  
  /* No arrow if the clock has expired.
   */
  if (!game->running) {
  
  /* If the target is in view, draw a big happy arrow pointing to it.
   */
  } else if ((targetx>=-NS_sys_tilesize)&&(targetx<FBW+NS_sys_tilesize)&&(targety>=-NS_sys_tilesize)&&(targety<FBH+NS_sys_tilesize)) {
    int dsty=targety-NS_sys_tilesize*3+(int)(sin((game->arrowclock*M_PI*2.0f)/ARROW_CLOCK_PERIOD)*6.0f);
    graf_draw_decal(&g.graf,g.texid_tiles,
      targetx-NS_sys_tilesize,dsty,
      0,NS_sys_tilesize,
      NS_sys_tilesize<<1,NS_sys_tilesize<<1,0
    );
  
  /* Target out of view but exists, draw a rotated arrow along the edge.
   */
  } else if (game->target.type) {
    double midx=camerax+(FBW>>1);
    double midy=cameray+(FBH>>1);
    double dx=game->target.x-midx;
    double dy=game->target.y-midy;
    const double edgedx=(FBW>>1)-NS_sys_tilesize;
    const double edgedy=(FBH>>1)-NS_sys_tilesize;
    double fx,fy;
    double xfory=(edgedy*dx)/dy;
    if ((xfory>=-edgedx)&&(xfory<=edgedx)) {
      if (dy>0.0) {
        fx=midx+xfory;
        fy=midy+edgedy;
      } else {
        fx=midx-xfory;
        fy=midy-edgedy;
      }
    } else {
      double yforx=(edgedx*dy)/dx;
      if (dx>0.0) {
        fx=midx+edgedx;
        fy=midy+yforx;
      } else {
        fx=midx-edgedx;
        fy=midy-yforx;
      }
    }
    int dstx=(int)fx-camerax;
    int dsty=(int)fy-cameray;
    float t=atan2(-dx,dy);
    float scale=sin((game->arrowclock*M_PI*2.0f)/ARROW_CLOCK_PERIOD)*0.125f+0.5f;
    // Leave a 1-pixel border inside the source image. It's drawn with a 2-pixel border.
    graf_draw_mode7(&g.graf,g.texid_tiles,
      dstx,dsty,
      1,NS_sys_tilesize+1,(NS_sys_tilesize<<1)-2,(NS_sys_tilesize<<1)-2,
      scale,scale,t,1
    );
  }
  
  /* Show clock in the top right.
   */
  if (game->running) {
    int ms=(int)(game->clock*1000.0);
    int s=ms/1000; ms%=1000;
    int min=s/60; s%=60;
    if (min>9) { min=9; s=99; }
    int16_t dstx=FBW-30,dsty=8,xstride=7;
    graf_draw_tile(&g.graf,g.texid_tiles,dstx,dsty,'0'+min,0); dstx+=xstride;
    if (ms>=250) graf_draw_tile(&g.graf,g.texid_tiles,dstx,dsty,':',0); dstx+=xstride;
    graf_draw_tile(&g.graf,g.texid_tiles,dstx,dsty,'0'+s/10,0); dstx+=xstride;
    graf_draw_tile(&g.graf,g.texid_tiles,dstx,dsty,'0'+s%10,0);
  }
  
  //TODO Bigger countdown mid-screen, below 10 s.
  
  /* Show score in the top left.
   * Rules aren't settled yet, and maps are very temporary, but in early playthrus, I'm getting scores of 10..20.
   * I think we can set a ceiling of 99.
   */
  {
    int16_t dstx=8,dsty=8,xstride=7;
    int rscr=game->score;
    if (rscr>99) rscr=99; else if (rscr<0) rscr=0;
    graf_draw_tile(&g.graf,g.texid_tiles,dstx,dsty,'0'+rscr/10,0); dstx+=xstride;
    graf_draw_tile(&g.graf,g.texid_tiles,dstx,dsty,'0'+rscr%10,0);
  }
}
