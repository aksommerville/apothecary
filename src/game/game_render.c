#include "game_internal.h"

/* Mode7 sprite.
 */
 
void render_mode7(
  int dstx,int dsty,
  int srcx,int srcy,int srcw,int srch,
  double xscale,double yscale,double rotate
) {
  rotate+=M_PI; // Easier than fixing the incorrect xform below.
  double xr=(double)srcw*0.5;
  double yr=(double)srch*0.5;
  double sint=sin(rotate);
  double cost=cos(rotate);
  double nwx=( xr*xscale)*cost+(-yr*yscale)*sint;
  double nwy=( xr*xscale)*sint-(-yr*yscale)*cost;
  double nex=(-xr*xscale)*cost+(-yr*yscale)*sint;
  double ney=(-xr*xscale)*sint-(-yr*yscale)*cost;
  double swx=( xr*xscale)*cost+( yr*yscale)*sint;
  double swy=( xr*xscale)*sint-( yr*yscale)*cost;
  double sex=(-xr*xscale)*cost+( yr*yscale)*sint;
  double sey=(-xr*xscale)*sint-( yr*yscale)*cost;
  graf_set_filter(&g.graf,1);
  graf_triangle_strip_tex_begin(&g.graf,
    dstx+(int)nwx,dsty+(int)nwy,srcx     ,srcy     ,
    dstx+(int)nex,dsty+(int)ney,srcx+srcw,srcy     ,
    dstx+(int)swx,dsty+(int)swy,srcx     ,srcy+srch
  );
  graf_triangle_strip_tex_more(&g.graf,
    dstx+(int)sex,dsty+(int)sey,srcx+srcw,srcy+srch
  );
  graf_set_filter(&g.graf,0);
}

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
  int dstx0=cola*NS_sys_tilesize+(NS_sys_tilesize>>1)-camerax;
  int dsty=rowa*NS_sys_tilesize+(NS_sys_tilesize>>1)-cameray;
  const uint8_t *mrow=game->map->v+rowa*game->map->w+cola;
  int row=rowa;
  graf_set_input(&g.graf,g.texid_tiles);
  for (;row<=rowz;row++,mrow+=game->map->w,dsty+=NS_sys_tilesize) {
    const uint8_t *mp=mrow;
    int dstx=dstx0;
    int col=cola;
    for (;col<=colz;col++,mp++,dstx+=NS_sys_tilesize) {
      graf_tile(&g.graf,dstx,dsty,*mp,0);
    }
  }
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
  graf_set_input(&g.graf,g.texid_hero);
  render_mode7(
    dstx,dsty+shadowdy,
    srcx,srcy,NS_sys_tilesize*3,NS_sys_tilesize*3,0.5f,0.5f,
    game->racer.t
  );
  graf_set_alpha(&g.graf,0xff);
  graf_set_tint(&g.graf,0);
  
  // And then the real thing.
  render_mode7(
    dstx,dsty,
    srcx,srcy,NS_sys_tilesize*3,NS_sys_tilesize*3,0.5f,0.5f,
    game->racer.t
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
    graf_decal(&g.graf,
      targetx-NS_sys_tilesize,dsty,
      0,NS_sys_tilesize,
      NS_sys_tilesize<<1,NS_sys_tilesize<<1
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
    render_mode7(
      dstx,dsty,
      1,NS_sys_tilesize+1,(NS_sys_tilesize<<1)-2,(NS_sys_tilesize<<1)-2,
      scale,scale,t
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

  if (blackout) graf_fill_rect(&g.graf,0,0,FBW,FBH,0x000000ff);
  
  /* Draw background grid.
   */
  game_render_grid(game,camerax,cameray);
  
  /* Target sprite.
   */
  graf_set_input(&g.graf,g.texid_tiles);
  int targetx=999,targety=0;
  if (game->target.type) {
    targetx=(int)game->target.x-camerax;
    targety=(int)game->target.y-cameray;
    graf_tile(&g.graf,targetx,targety,game->target.tileid,0);
  }
  
  /* Bystanders.
   */
  {
    struct bystander *bystander=game->bystanderv;
    int i=game->bystanderc;
    for (;i-->0;bystander++) {
      int16_t dstx=(int)bystander->x-camerax;
      int16_t dsty=(int)bystander->y-cameray;
      graf_tile(&g.graf,dstx,dsty,bystander->tileid+((bystander->react>0.0)?0x10:0),0);
    }
  }
  
  /* Hero.
   */
  game_draw_hero(game,herox-camerax,heroy-cameray);
  graf_set_input(&g.graf,g.texid_tiles);
  
  /* Arrow: Either the big animated one, or the little one on the edge.
   */
  if (game->running) {
    game_draw_arrow(game,camerax,cameray,targetx,targety);
  }
  
  /* Show clock in the top right.
   */
  {
    int ms=(int)(game->clock*1000.0);
    int s=ms/1000; ms%=1000;
    int min=s/60; s%=60;
    if (min>9) { min=9; s=99; }
    int16_t dstx=FBW-30,dsty=8,xstride=7;
    graf_tile(&g.graf,dstx,dsty,'0'+min,0); dstx+=xstride;
    if (ms>=250) graf_tile(&g.graf,dstx,dsty,':',0); dstx+=xstride;
    graf_tile(&g.graf,dstx,dsty,'0'+s/10,0); dstx+=xstride;
    graf_tile(&g.graf,dstx,dsty,'0'+s%10,0);
  }
  
  /* Progress indicator left of the clock.
   */
  {
    int16_t dstx=FBW-30-NS_sys_tilesize;
    int16_t dsty=8;
    int i=0;
    for (;i<game->map->dropoffc;i++,dstx-=NS_sys_tilesize) {
      graf_tile(&g.graf,dstx,dsty,(i>=game->dropoffc)?0x3d:0x3c,0);
    }
  }
  
  /* If we're ended, fade to black.
   */
  if (!game->running&&(game->endtime<GAME_END_FADE_TIME)) {
    int alpha=256.0-(game->endtime*256.0)/GAME_END_FADE_TIME;
    if (alpha<=0) return;
    if (alpha>0xff) alpha=0xff;
    graf_fill_rect(&g.graf,0,0,FBW,FBH,0x00000000|alpha);
  }
  
  /* Pause menu.
   */
  if (game->pause_selp) {
    graf_fill_rect(&g.graf,0,0,FBW,FBH,0x000000c0);
    if (!game->texid_resume) {
      const char *src=0;
      int srcc=text_get_string(&src,1,17);
      game->texid_resume=font_render_to_texture(0,g.font,src,srcc,FBW,font_get_line_height(g.font),0xffffffff);
      egg_texture_get_size(&game->w_resume,&game->h_resume,game->texid_resume);
    }
    if (!game->texid_menu) {
      const char *src=0;
      int srcc=text_get_string(&src,1,18);
      game->texid_menu=font_render_to_texture(0,g.font,src,srcc,FBW,font_get_line_height(g.font),0xffffffff);
      egg_texture_get_size(&game->w_menu,&game->h_menu,game->texid_menu);
    }
    int margin=4;
    int boxw=((game->w_resume>game->w_menu)?game->w_resume:game->w_menu)+(margin<<1);
    int boxh=game->h_resume+game->h_menu+margin*3;
    int dstx=(FBW>>1)-(boxw>>1);
    int dsty=(FBH>>1)-(boxh>>1);
    graf_fill_rect(&g.graf,dstx,dsty,boxw,boxh,0x200000ff);
    graf_fill_rect(&g.graf,dstx+1,dsty+1+((game->pause_selp==2)?(game->h_resume+margin):0),boxw-2,game->h_resume+margin*2-2,0x203040ff);
    graf_set_input(&g.graf,game->texid_resume);
    graf_decal(&g.graf,dstx+(boxw>>1)-(game->w_resume>>1),dsty+margin,0,0,game->w_resume,game->h_resume);
    graf_set_input(&g.graf,game->texid_menu);
    graf_decal(&g.graf,dstx+(boxw>>1)-(game->w_menu>>1),dsty+boxh-margin-game->h_menu,0,0,game->w_menu,game->h_menu);
  }
}
