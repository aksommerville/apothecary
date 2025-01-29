#include "game.h"

struct hello {
  int blackout;
  int dismissed;
  double clock; // Counts up forever.
};

/* Delete.
 */
 
void hello_del(struct hello *hello) {
  if (!hello) return;
  free(hello);
}

/* New.
 */
 
struct hello *hello_new() {
  struct hello *hello=calloc(1,sizeof(struct hello));
  if (!hello) return 0;
  
  hello->blackout=1;
  
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

/* Update.
 */
 
int hello_update(struct hello *hello,double elapsed) {
  hello->clock+=elapsed;
  if (hello->dismissed) return -1;
  return 0;
}

/* Render.
 */
 
void hello_render(struct hello *hello) {
  graf_draw_rect(&g.graf,0,0,FBW,FBH,0x080420ff);
  
  /* Options and cursor.
   */
  //TODO
  
  /* Title slides in from the top then stays put forever.
   */
  int titley=0;
  if (hello->clock<1.000) {
    titley=-100;
  } else if (hello->clock<3.000) {
    titley=-100+(int)(((hello->clock-1.000)*100.0)/2.000);
  }
  graf_draw_decal(&g.graf,g.texid_gross,0,titley,0,0,320,100,0);
  
  /* Dot occasionally zips by horizontally, alternating direction.
   */
  double dotperiod=fmod(hello->clock,20.0);
  uint8_t dotxform=0;
  if (dotperiod>=10.0) {
    dotperiod-=10.0;
    dotxform=EGG_XFORM_XREV;
  }
  if ((dotperiod>6.0)&&(dotperiod<=7.0)) {
    double normx=(dotperiod-6.0)/1.0;
    double rangex=FBW+167.0;
    int dstx;
    if (dotxform) { // right to left
      dstx=FBW-(int)(normx*rangex);
    } else { // left to right
      dstx=-167+(int)(normx*rangex);
    }
    graf_draw_decal(&g.graf,g.texid_gross,dstx,40,0,101,167,121,dotxform);
  }
}
