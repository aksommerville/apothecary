#include "game_internal.h"

/* Grid.
 */
 
static void game_render_grid(struct game *game,int camerax,int cameray) {
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
}

/* Hero.
 */
 
static void game_draw_hero(struct game *game,int dstx,int dsty) {

  // Select frame.
  int srcx=0,srcy=0;
  double v2=game->racer.vx*game->racer.vx+game->racer.vy*game->racer.vy;
  if (v2<10000.0) { // Slow. Keep the neutral frame.
  } else { // Check velocity against pointing direction.
    double it=atan2(game->racer.vx,-game->racer.vy); // inertia
    double ft=game->racer.t; // face
    double d=it-ft;
    while (d<-M_PI) d+=M_PI*2.0;
    while (d>M_PI) d-=M_PI*2.0;
    if ((d<M_PI*-0.333)&&(d>M_PI*-0.666)) { // Sliding left
      if (game->dotanimframe) srcx=NS_sys_tilesize*9;
      else srcx=NS_sys_tilesize*12;
    } else if ((d>M_PI*0.333)&&(d<M_PI*0.666)) { // Sliding right
      srcy=NS_sys_tilesize*3;
      if (game->dotanimframe) srcx=NS_sys_tilesize*9;
      else srcx=NS_sys_tilesize*12;
    } else if ((d<M_PI*-0.5)||(d>M_PI*0.5)) { // Backward.
      // We don't have faces for this, so keep the still face.
    } else { // Forward.
      if (game->dotanimframe) srcx=NS_sys_tilesize*6;
      else srcx=NS_sys_tilesize*3;
    }
  }

  // First a shadow, offset down a little.
  int shadowdy=4;
  int col=(int)game->racer.x/NS_sys_tilesize;
  int row=(int)game->racer.y/NS_sys_tilesize;
  if ((col>=0)&&(col<game->map->w)&&(row>=0)&&(row<game->map->h)) {
    uint8_t physics=game->map->physics[game->map->v[row*game->map->w+col]];
    if (physics==NS_physics_water) shadowdy+=2;
  }
  graf_set_alpha(&g.graf,0x80);
  graf_set_tint(&g.graf,0x000000ff);
  graf_draw_mode7(&g.graf,g.texid_hero,
    dstx,dsty+shadowdy,
    srcx,srcy,NS_sys_tilesize*3,NS_sys_tilesize*3,0.5f,0.5f,
    game->racer.t,1
  );
  graf_set_alpha(&g.graf,0xff);
  graf_set_tint(&g.graf,0);
  
  // And then the real thing.
  graf_draw_mode7(&g.graf,g.texid_hero,
    dstx,dsty,
    srcx,srcy,NS_sys_tilesize*3,NS_sys_tilesize*3,0.5f,0.5f,
    game->racer.t,1
  );
}

/* Big arrow, or little one on the edge.
 * (targetx,targety) is in framebuffer pixels. Caller should already have that from drawing the target sprite.
 */
 
static void game_draw_arrow(struct game *game,int camerax,int cameray,int targetx,int targety) {
  
  /* If the target is in view, draw a big happy arrow pointing to it.
   */
  if ((targetx>=-NS_sys_tilesize)&&(targetx<FBW+NS_sys_tilesize)&&(targety>=-NS_sys_tilesize)&&(targety<FBH+NS_sys_tilesize)) {
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
}

/* Render, main.
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
  game_render_grid(game,camerax,cameray);
  
  /* Target sprite.
   */
  int targetx=999,targety=0;
  if (game->target.type) {
    targetx=(int)game->target.x-camerax;
    targety=(int)game->target.y-cameray;
    graf_draw_tile(&g.graf,g.texid_tiles,targetx,targety,game->target.tileid,0);
  }
  
  //TODO Decorative sprites.
  
  /* Hero.
   */
  game_draw_hero(game,herox-camerax,heroy-cameray);
  
  /* Arrow: Either the big animated one, or the little one on the edge.
   */
  if (game->running) {
    game_draw_arrow(game,camerax,cameray,targetx,targety);
  }
  
  /* Bonus toast if present.
   */
  if (game->bonus_toast.ttl>0.0) {
    int dstx=(int)game->bonus_toast.x-camerax-7;
    int dsty=(int)game->bonus_toast.y-cameray;
    dsty-=(int)(((BONUS_TOAST_TTL-game->bonus_toast.ttl)*NS_sys_tilesize)/BONUS_TOAST_TTL);
    int alpha=(game->bonus_toast.ttl*255.0)/BONUS_TOAST_TTL;
    if (alpha<0xff) graf_set_alpha(&g.graf,alpha);
    graf_draw_tile(&g.graf,g.texid_tiles,dstx,dsty,0x3b,0); dstx+=7; // '+', not ascii
    if (game->bonus_toast.v>=10) {
      graf_draw_tile(&g.graf,g.texid_tiles,dstx,dsty,'0'+(game->bonus_toast.v/10)%10,0); dstx+=7;
    }
    graf_draw_tile(&g.graf,g.texid_tiles,dstx,dsty,'0'+game->bonus_toast.v%10,0);
    graf_set_alpha(&g.graf,0xff);
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
  
  /* If we're ended, fade to black.
   */
  if (!game->running&&(game->endtime<GAME_END_FADE_TIME)) {
    int alpha=256.0-(game->endtime*256.0)/GAME_END_FADE_TIME;
    if (alpha<=0) return;
    if (alpha>0xff) alpha=0xff;
    graf_draw_rect(&g.graf,0,0,FBW,FBH,0x00000000|alpha);
  }
}
