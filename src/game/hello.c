#include "game.h"

#define HELLO_MESSAGE_TIME 6.0
#define HELLO_MESSAGE_FADE_TIME 0.5 /* in and out */

// Index in strings:1
static const int messagev[]={10,11,12};

struct hello {
  int blackout;
  int dismissed;
  double clock; // Counts up forever.
  int msgp;
  double msgclock;
  int msg_texid;
  int msg_texw;
  int msg_texh;
};

/* Delete.
 */
 
void hello_del(struct hello *hello) {
  if (!hello) return;
  egg_texture_del(hello->msg_texid);
  free(hello);
}

/* New.
 */
 
struct hello *hello_new() {
  struct hello *hello=calloc(1,sizeof(struct hello));
  if (!hello) return 0;
  
  hello->blackout=1;
  hello->msgp=-1; // Start with no message.
  hello->msgclock=3.0;
  
  egg_play_song(RID_song_thirty_seconds,0,1);
  
  return hello;
}

/* Input.
 */
 
void hello_input(struct hello *hello,int input,int pvinput) {
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) {
    if (hello->clock<3.0) hello->clock=3.0;
    else hello->dismissed=1;
  }
}

/* Trash existing message and render the next one.
 */
 
static void hello_next_message(struct hello *hello) {
  hello->msgp++;
  if ((hello->msgp<0)||(hello->msgp>=sizeof(messagev)/sizeof(messagev[0]))) hello->msgp=0;
  egg_texture_del(hello->msg_texid);
  hello->msg_texid=font_texres_oneline(g.font,1,messagev[hello->msgp],FBW,0xffffffff);
  egg_texture_get_status(&hello->msg_texw,&hello->msg_texh,hello->msg_texid);
}

/* Update.
 */
 
int hello_update(struct hello *hello,double elapsed) {
  hello->clock+=elapsed;
  if (hello->dismissed) return -1;
  if ((hello->msgclock-=elapsed)<=0.0) {
    hello->msgclock+=HELLO_MESSAGE_TIME;
    hello_next_message(hello);
  }
  return 0;
}

/* Render.
 */
 
void hello_render(struct hello *hello) {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x080420ff);
  
  /* When Dot flies right-to-left, she's behind the title.
   */
  double dotperiod=fmod(hello->clock,20.0);
  uint8_t dotxform=0;
  if (dotperiod>=10.0) {
    dotperiod-=10.0;
    dotxform=EGG_XFORM_XREV;
    double normx=(dotperiod-6.0)/1.0;
    double rangex=FBW+167.0;
    int dstx=FBW-(int)(normx*rangex)+83;
    graf_set_tint(&g.graf,0x00000080);
    graf_draw_mode7(&g.graf,g.texid_gross,
      dstx,90,
      0,101,167,121,
      -0.500,0.500,0.0,1
    );
    graf_set_tint(&g.graf,0);
  }
  
  /* Title slides in from the top then stays put forever.
   */
  int titley=0;
  if (hello->clock<1.000) {
    titley=-100;
  } else if (hello->clock<3.000) {
    titley=-100+(int)(((hello->clock-1.000)*100.0)/2.000);
  }
  graf_draw_decal(&g.graf,g.texid_gross,0,titley,0,0,320,100,0);
  
  /* Credits and such at the bottom.
   */
  {
    int16_t dstx=(FBW>>1)-(hello->msg_texw>>1);
    int16_t dsty=FBH-hello->msg_texh;
    int alpha=0xff;
    if (hello->msgclock<HELLO_MESSAGE_FADE_TIME) {
      alpha=(int)((hello->msgclock*256.0)/HELLO_MESSAGE_FADE_TIME);
    } else if (hello->msgclock>HELLO_MESSAGE_TIME-HELLO_MESSAGE_FADE_TIME) {
      alpha=(int)(((HELLO_MESSAGE_TIME-hello->msgclock)*256.0)/HELLO_MESSAGE_FADE_TIME);
    }
    if (alpha<0) alpha=0; else if (alpha>0xff) alpha=0xff;
    graf_set_alpha(&g.graf,alpha);
    graf_draw_decal(&g.graf,hello->msg_texid,dstx,dsty,0,0,hello->msg_texw,hello->msg_texh,0);
    graf_set_alpha(&g.graf,0xff);
  }
  
  /* Dot left-to-right, in front of the title and unobscured.
   */
  if (!dotxform) {
    double normx=(dotperiod-6.0)/1.0;
    double rangex=FBW+167.0;
    int dstx=-167+(int)(normx*rangex);
    graf_draw_decal(&g.graf,g.texid_gross,dstx,40,0,101,167,121,dotxform);
  }
  
  /* Options and cursor.
   */
  //TODO
}
