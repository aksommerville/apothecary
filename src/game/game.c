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
  
  game->racer.x=FBW*0.5;
  game->racer.y=FBH*0.5;
  game->racer.t=0.0;
  
  if (!(game->map=map_get(RID_map_scratch))) {
    game_del(game);
    return 0;
  }
  
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
}

/* Update.
 */
 
int game_update(struct game *game,double elapsed) {

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
  } else {
    game->racer.vx*=0.980;//TODO proper deceleration... involves a pow() somehow
    game->racer.vy*=0.980;
  }
  game->racer.x+=game->racer.vx*elapsed;
  game->racer.y+=game->racer.vy*elapsed;
  double worldw=(double)(game->map->w*NS_sys_tilesize);
  double worldh=(double)(game->map->h*NS_sys_tilesize);
  if (game->racer.x<0.0) game->racer.x=0.0; else if (game->racer.x>worldw) game->racer.x=worldw;
  if (game->racer.y<0.0) game->racer.y=0.0; else if (game->racer.y>worldh) game->racer.y=worldh;
  
  physics_update(game,elapsed);
  
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
}
