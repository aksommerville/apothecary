#include "game_internal.h"

#define GAMEOVER_FADE_IN_TIME 1.0

struct gameover {
  int blackout;
  int dismissed;
  double clock; // Counts up forever.
  int texid;
  int texw,texh;
};

/* Delete.
 */
 
void gameover_del(struct gameover *go) {
  if (!go) return;
  egg_texture_del(go->texid);
  free(go);
}

/* Render the big message.
 */
 
static int gameover_render_message(struct gameover *go) {
  if ((go->texid=egg_texture_new())<1) return -1;
  int score=g.game?g.game->score:0;
  
  /* Allocate a scratch buffer the size of the framebuffer.
   * We'll implicitly crop it when uploading to the texture.
   */
  uint32_t *rgba=calloc(FBW<<2,FBH);
  if (!rgba) return -1;
  
  int dsty=0,w=0,w1,textc,srcc;
  int lineh=font_get_line_height(g.font);
  char text[256];
  const char *src;
  
  // "You completed %0 deliveries."
  if (score>0) {
    struct strings_insertion ins={'i',.i=score};
    textc=strings_format(text,sizeof(text),1,4,&ins,1);
    if ((textc<0)||(textc>sizeof(text))) textc=0;
    w1=font_render_string(rgba,FBW,FBH,FBW<<2,0,dsty,g.font,text,textc,0xffffffff);
    if (w1>w) w=w1;
    dsty+=lineh;
  }
  
  // Score judgment.
       if (score<=0) srcc=strings_get(&src,1,3); // Special message at zero.
  else if (score<5) srcc=strings_get(&src,1,5); // "Not good"
  else if (score<10) srcc=strings_get(&src,1,6); // "Not bad"
  else srcc=strings_get(&src,1,7); // "Fantastic!"
  w1=font_render_string(rgba,FBW,FBH,FBW<<2,0,dsty,g.font,src,srcc,0xffffffff);
  if (w1>w) w=w1;
  dsty+=lineh;
  
  // "New high score" or report the existing one.
  if (score>=g.hiscore) {
    srcc=strings_get(&src,1,8);
    w1=font_render_string(rgba,FBW,FBH,FBW<<2,0,dsty,g.font,src,srcc,0xffffffff);
    if (w1>w) w=w1;
    dsty+=lineh;
  } else {
    struct strings_insertion ins={'i',.i=g.hiscore};
    int textc=strings_format(text,sizeof(text),1,9,&ins,1);
    if ((textc<1)||(textc>sizeof(text))) textc=0;
    w1=font_render_string(rgba,FBW,FBH,FBW<<2,0,dsty,g.font,text,textc,0xffffffff);
    if (w1>w) w=w1;
    dsty+=lineh;
  }
  
  if (w>FBW) go->texw=FBW;
  else go->texw=w;
  if (dsty>FBH) go->texh=FBH;
  else go->texh=dsty;
  egg_texture_load_raw(go->texid,EGG_TEX_FMT_RGBA,go->texw,go->texh,FBW<<2,rgba,FBW*FBH*4);
  
  free(rgba);
  return 0;
}

/* New.
 */
 
struct gameover *gameover_new() {
  struct gameover *go=calloc(1,sizeof(struct gameover));
  if (!go) return 0;
  
  go->blackout=1;
  
  // Ensure the ending song is playing.
  // But it should be redundant: game starts the ending music before it finishes.
  egg_play_song(RID_song_emotional_support_bird,0,1);
  
  if (gameover_render_message(go)<0) {
    gameover_del(go);
    return 0;
  }
  
  return go;
}

/* Input.
 */
 
void gameover_input(struct gameover *go,int input,int pvinput) {
  if (go->clock<1.0) return;
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) go->dismissed=1;
}

/* Update.
 */
 
int gameover_update(struct gameover *go,double elapsed) {
  go->clock+=elapsed;
  if (go->dismissed) return -1;
  return 0;
}

/* Render.
 */
 
void gameover_render(struct gameover *go) {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x080420ff);
  
  int dstx=(FBW>>1)-(go->texw>>1);
  int dsty=(FBH>>1)-(go->texh>>1);
  graf_draw_decal(&g.graf,go->texid,dstx,dsty,0,0,go->texw,go->texh,0);
  
  /* Fade from black at startup.
   */
  if (go->clock<GAMEOVER_FADE_IN_TIME) {
    int alpha=256.0-(go->clock*256.0)/GAMEOVER_FADE_IN_TIME;
    if (alpha<=0) return;
    if (alpha>0xff) alpha=0xff;
    graf_draw_rect(&g.graf,0,0,FBW,FBH,0x00000000|alpha);
  }
}
