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

/* Format time: M:SS.mmm
 * Always returns in 0..dsta.
 */

static int gameover_format_time(char *dst,int dsta,double f) {
  int ms=(int)(f*1000.0);
  if (ms<0) ms=0;
  int s=ms/1000; ms%=1000;
  int min=s/60; s%=60;
  if (min>9) { min=9; s=99; ms=999; }
  if (dsta<8) return 0;
  dst[0]='0'+min;
  dst[1]=':';
  dst[2]='0'+s/10;
  dst[3]='0'+s%10;
  dst[4]='.';
  dst[5]='0'+(ms/100)%10;
  dst[6]='0'+(ms/10)%10;
  dst[7]='0'+ms%10;
  return 8;
}

/* Render the big message.
 */
 
static int gameover_render_message(struct gameover *go) {
  if ((go->texid=egg_texture_new())<1) return -1;
  int score=0;
  
  /* Allocate a scratch buffer the size of the framebuffer.
   * We'll implicitly crop it when uploading to the texture.
   */
  uint32_t *rgba=calloc(FBW<<2,FBH);
  if (!rgba) return -1;
  
  int dsty=0,w=0,w1,textc,srcc,tmpc;
  int lineh=font_get_line_height(g.font);
  char text[256],tmp[32];
  const char *src;
  
  // "Your score: M:SS.mmm"
  {
    tmpc=gameover_format_time(tmp,sizeof(tmp),g.game->clock);
    struct text_insertion ins={'s',.s={.v=tmp,.c=tmpc}};
    textc=text_format_res(text,sizeof(text),1,3,&ins,1);
    if ((textc<0)||(textc>sizeof(text))) textc=0;
    w1=font_render_string(rgba,FBW,FBH,FBW<<2,0,dsty,g.font,text,textc,0xffffffff);
    if (w1>w) w=w1;
    dsty+=lineh;
  }
  
  // "High score: M:SS.mmm"
  {
    tmpc=gameover_format_time(tmp,sizeof(tmp),g.hiscore);
    struct text_insertion ins={'s',.s={.v=tmp,.c=tmpc}};
    textc=text_format_res(text,sizeof(text),1,4,&ins,1);
    if ((textc<0)||(textc>sizeof(text))) textc=0;
    w1=font_render_string(rgba,FBW,FBH,FBW<<2,0,dsty,g.font,text,textc,0xffffffff);
    if (w1>w) w=w1;
    dsty+=lineh;
  }
  
  // "New high score!"
  if (g.hiscore_is_new) {
    srcc=text_get_string(&src,1,5);
    w1=font_render_string(rgba,FBW,FBH,FBW<<2,0,dsty,g.font,src,srcc,0xffffffff);
    if (w1>w) w=w1;
    dsty+=lineh;
  }
  
  if (w>FBW) go->texw=FBW;
  else go->texw=w;
  if (dsty>FBH) go->texh=FBH;
  else go->texh=dsty;
  egg_texture_load_raw(go->texid,go->texw,go->texh,FBW<<2,rgba,FBW*FBH*4);
  
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
  play_song(RID_song_emotional_support_bird);
  
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
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x080420ff);
  
  int dstx=(FBW>>1)-(go->texw>>1);
  int dsty=(FBH>>1)-(go->texh>>1);
  graf_set_input(&g.graf,go->texid);
  graf_decal(&g.graf,dstx,dsty,0,0,go->texw,go->texh);
  
  /* Fade from black at startup.
   */
  if (go->clock<GAMEOVER_FADE_IN_TIME) {
    int alpha=256.0-(go->clock*256.0)/GAMEOVER_FADE_IN_TIME;
    if (alpha<=0) return;
    if (alpha>0xff) alpha=0xff;
    graf_fill_rect(&g.graf,0,0,FBW,FBH,0x00000000|alpha);
  }
}
