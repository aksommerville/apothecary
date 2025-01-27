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
    game->racer.vx*=0.990;//TODO proper deceleration... involves a pow() somehow
    game->racer.vy*=0.990;
  }
  game->racer.x+=game->racer.vx;
  game->racer.y+=game->racer.vy;
  if (game->racer.x<0.0) game->racer.x=0.0; else if (game->racer.x>FBW) game->racer.x=FBW;
  if (game->racer.y<0.0) game->racer.y=0.0; else if (game->racer.y>FBH) game->racer.y=FBH;
  
  return 0;
}

/* Render.
 */
 
void game_render(struct game *game) {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0xa0a0b0ff);
  
  int dstx=(int)game->racer.x;
  int dsty=(int)game->racer.y;
  graf_set_alpha(&g.graf,0x80);
  graf_set_tint(&g.graf,0x000000ff);
  graf_draw_mode7(&g.graf,g.texid_hero,
    dstx,dsty+4,
    0,0,NS_sys_tilesize*3,NS_sys_tilesize*3,0.5f,0.5f,
    game->racer.t,1
  );
  graf_set_alpha(&g.graf,0xff);
  graf_set_tint(&g.graf,0);
  graf_draw_mode7(&g.graf,g.texid_hero,
    dstx,dsty,
    0,0,NS_sys_tilesize*3,NS_sys_tilesize*3,0.5f,0.5f,
    game->racer.t,1
  );
}
