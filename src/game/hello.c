#include "game.h"

#define HELLO_MESSAGE_TIME 6.0
#define HELLO_MESSAGE_FADE_TIME 0.5 /* in and out */

#define HELLO_OPTION_WIDTH 100
#define HELLO_OPTION_HEIGHT 24

#define OPTION_SLIDE_RATE 4.000 /* hz */

// Option ID is chosen to match the string index.
#define HELLO_OPTION_PLAY 6
#define HELLO_OPTION_QUIT 7
#define HELLO_OPTION_LANGUAGE 8
#define HELLO_OPTION_MUSIC 9
#define HELLO_OPTION_SOUND 10
#define HELLO_OPTION_INPUT 11

// Index in strings:1
static const int messagev[]={4,12,13,14};

struct hello {
  int blackout;
  int dismissed;
  double clock; // Counts up forever.
  int msgp;
  double msgclock;
  int msg_texid;
  int msg_texw;
  int msg_texh;
  struct option {
    int id;
    int lbl_texid;
    int lblw,lblh;
    int sub_texid;
    int subw,subh;
  } *optionv;
  int optionc,optiona;
  int optionp;
  int *langv;
  int langc,langa;
  double option_slide;
};

/* Delete.
 */
 
static void option_cleanup(struct option *option) {
  egg_texture_del(option->lbl_texid);
  egg_texture_del(option->sub_texid);
}
 
void hello_del(struct hello *hello) {
  if (!hello) return;
  egg_texture_del(hello->msg_texid);
  if (hello->optionv) {
    while (hello->optionc-->0) option_cleanup(hello->optionv+hello->optionc);
    free(hello->optionv);
  }
  if (hello->langv) free(hello->langv);
  free(hello);
}

/* Set option sub.
 */
 
static int option_set_sub_string(struct option *option,int ix) {
  egg_texture_del(option->sub_texid);
  const char *src=0;
  int srcc=text_get_string(&src,1,ix);
  option->sub_texid=font_render_to_texture(0,g.font,src,srcc,HELLO_OPTION_WIDTH,font_get_line_height(g.font),0x80ffffff);
  egg_texture_get_size(&option->subw,&option->subh,option->sub_texid);
  return 0;
}

/* Init options.
 */
 
static struct option *hello_add_option(struct hello *hello,int id) {
  if (hello->optionc>=hello->optiona) {
    int na=hello->optiona+8;
    if (na>INT_MAX/sizeof(struct option)) return 0;
    void *nv=realloc(hello->optionv,sizeof(struct option)*na);
    if (!nv) return 0;
    hello->optionv=nv;
    hello->optiona=na;
  }
  struct option *option=hello->optionv+hello->optionc++;
  memset(option,0,sizeof(struct option));
  option->id=id;
  const char *src=0;
  int srcc=text_get_string(&src,1,id);
  option->lbl_texid=font_render_to_texture(0,g.font,src,srcc,HELLO_OPTION_WIDTH,font_get_line_height(g.font),0xffffffff);
  egg_texture_get_size(&option->lblw,&option->lblh,option->lbl_texid);
  return option;
}
 
static int hello_init_options(struct hello *hello) {
  struct option *option;
  if (!(option=hello_add_option(hello,HELLO_OPTION_PLAY))) return -1;
  if (!(option=hello_add_option(hello,HELLO_OPTION_LANGUAGE))) return -1;
  option_set_sub_string(option,1);
  if (!(option=hello_add_option(hello,HELLO_OPTION_INPUT))) return -1;
  if (!(option=hello_add_option(hello,HELLO_OPTION_MUSIC))) return -1;
  option_set_sub_string(option,g.enable_music?15:16);
  if (!(option=hello_add_option(hello,HELLO_OPTION_SOUND))) return -1;
  option_set_sub_string(option,g.enable_sound?15:16);
  if (!(option=hello_add_option(hello,HELLO_OPTION_QUIT))) return -1;
  return 0;
}

// Call after changing language.
static void hello_rebuild_all_labels(struct hello *hello) {
  struct option *option=hello->optionv;
  int i=hello->optionc;
  for (;i-->0;option++) {
    egg_texture_del(option->lbl_texid);
    const char *src=0;
    int srcc=text_get_string(&src,1,option->id);
    option->lbl_texid=font_render_to_texture(0,g.font,src,srcc,HELLO_OPTION_WIDTH,font_get_line_height(g.font),0xffffffff);
    egg_texture_get_size(&option->lblw,&option->lblh,option->lbl_texid);
    switch (option->id) {
      case HELLO_OPTION_LANGUAGE: option_set_sub_string(option,1); break;
      case HELLO_OPTION_MUSIC: option_set_sub_string(option,g.enable_music?15:16); break;
      case HELLO_OPTION_SOUND: option_set_sub_string(option,g.enable_sound?15:16); break;
    }
  }
  hello->msgclock=0.0;
}

/* Build list of languages.
 */
 
static int hello_lang_cb(int lang,void *userdata) {
  struct hello *hello=userdata;
  if (hello->langc>=hello->langa) {
    int na=hello->langa+8;
    if (na>INT_MAX/sizeof(int)) return -1;
    void *nv=realloc(hello->langv,sizeof(int)*na);
    if (!nv) return -1;
    hello->langv=nv;
    hello->langa=na;
  }
  hello->langv[hello->langc++]=lang;
  return 0;
}
 
static int hello_list_languages(struct hello *hello) {
  hello->langc=0;
  return text_for_each_language(hello_lang_cb,hello);
  /*XXX need to read (g.rom) ourselves for this; The new text unit doesn't do it.
  int p=0; for (;;p++) {
    int lang=strings_lang_by_index(p);
    if (lang<0) break;
    if (hello->langc>=hello->langa) {
      int na=hello->langa+8;
      if (na>INT_MAX/sizeof(int)) return -1;
      void *nv=realloc(hello->langv,sizeof(int)*na);
      if (!nv) return -1;
      hello->langv=nv;
      hello->langa=na;
    }
    hello->langv[hello->langc++]=lang;
  }
  /**/
  return 0;
}

/* New.
 */
 
struct hello *hello_new() {
  struct hello *hello=calloc(1,sizeof(struct hello));
  if (!hello) return 0;
  
  hello->blackout=1;
  hello->msgp=-1; // Start with no message.
  hello->msgclock=3.0;
  
  if (
    (hello_list_languages(hello)<0)||
    (hello_init_options(hello)<0)
  ) {
    hello_del(hello);
    return 0;
  }
  
  play_song(RID_song_thirty_seconds);
  
  return hello;
}

/* Move cursor.
 */
 
static void hello_xmotion(struct hello *hello,int d) {
  play_sound(RID_sound_uimotion);
  hello->optionp+=d;
  if (hello->optionp<0) hello->optionp=hello->optionc-1;
  else if (hello->optionp>=hello->optionc) hello->optionp=0;
  hello->option_slide+=d;
}

static void hello_ymotion(struct hello *hello,int d) {
  if ((hello->optionp<0)||(hello->optionp>=hello->optionc)) return;
  play_sound(RID_sound_uimotion);
  struct option *option=hello->optionv+hello->optionp;
  switch (option->id) {
  
    case HELLO_OPTION_LANGUAGE: {
        int current=egg_prefs_get(EGG_PREF_LANG);
        int langp=-1;
        int i=0; for (;i<hello->langc;i++) {
          if (hello->langv[i]==current) {
            langp=i;
            break;
          }
        }
        if (langp<0) return;
        langp+=d;
        if (langp<0) langp=hello->langc-1;
        else if (langp>=hello->langc) langp=0;
        egg_prefs_set(EGG_PREF_LANG,hello->langv[langp]);
        hello_rebuild_all_labels(hello);
      } break;
      
    case HELLO_OPTION_MUSIC: {
        g.enable_music=g.enable_music?0:1;
        option_set_sub_string(option,g.enable_music?15:16);
        play_song(g.songid);
      } break;
    case HELLO_OPTION_SOUND: {
        g.enable_sound=g.enable_sound?0:1;
        option_set_sub_string(option,g.enable_sound?15:16);
      } break;
  }
}

/* Activate.
 */
 
static void hello_activate(struct hello *hello) {
  if ((hello->optionp<0)||(hello->optionp>=hello->optionc)) return;
  struct option *option=hello->optionv+hello->optionp;
  switch (option->id) {
    case HELLO_OPTION_PLAY: play_sound(RID_sound_uiactivate); hello->dismissed=1; break;
    case HELLO_OPTION_QUIT: egg_terminate(0); break;
    case HELLO_OPTION_LANGUAGE: hello_ymotion(hello,1); break;
    case HELLO_OPTION_MUSIC: hello_ymotion(hello,1); break;
    case HELLO_OPTION_SOUND: hello_ymotion(hello,1); break;
    case HELLO_OPTION_INPUT: play_sound(RID_sound_uiactivate); egg_input_configure(); break;
  }
}

/* Input.
 */
 
void hello_input(struct hello *hello,int input,int pvinput) {
  if (hello->clock>=3.0) {
    if ((input&EGG_BTN_LEFT)&&!(pvinput&EGG_BTN_LEFT)) hello_xmotion(hello,-1);
    if ((input&EGG_BTN_RIGHT)&&!(pvinput&EGG_BTN_RIGHT)) hello_xmotion(hello,1);
    if ((input&EGG_BTN_UP)&&!(pvinput&EGG_BTN_UP)) hello_ymotion(hello,-1);
    if ((input&EGG_BTN_DOWN)&&!(pvinput&EGG_BTN_DOWN)) hello_ymotion(hello,1);
  }
  if ((input&EGG_BTN_SOUTH)&&!(pvinput&EGG_BTN_SOUTH)) {
    if (hello->clock<3.0) {
      play_sound(RID_sound_uiactivate);
      hello->clock=3.0;
    } else hello_activate(hello);
  }
}

/* Format time: M:SS.mmm
 * Always returns in 0..dsta.
 */

static int hello_format_time(char *dst,int dsta,double f) {
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

/* Trash existing message and render the next one.
 */
 
static void hello_next_message(struct hello *hello) {
  egg_texture_del(hello->msg_texid);
  for (;;) {
    hello->msgp++;
    if ((hello->msgp<0)||(hello->msgp>=sizeof(messagev)/sizeof(messagev[0]))) hello->msgp=0;
    int ix=messagev[hello->msgp];
    
    // Pick off strings with inputs or conditional ones.
    if (ix==4) { // High score.
      if (g.hiscore>59.0*9.0) continue; // Unset, don't print it.
      char tmp[16];
      int tmpc=hello_format_time(tmp,sizeof(tmp),g.hiscore);
      char msg[64];
      struct text_insertion ins={.mode='s',.s={.v=tmp,.c=tmpc}};
      int msgc=text_format_res(msg,sizeof(msg),1,4,&ins,1);
      if ((msgc<1)||(msgc>sizeof(msg))) continue;
      hello->msg_texid=font_render_to_texture(0,g.font,msg,msgc,FBW,font_get_line_height(g.font),0x808080ff);
      egg_texture_get_size(&hello->msg_texw,&hello->msg_texh,hello->msg_texid);
      return;
    }
    
    // Everything else is static text.
    const char *src=0;
    int srcc=text_get_string(&src,1,messagev[hello->msgp]);
    hello->msg_texid=font_render_to_texture(0,g.font,src,srcc,FBW,font_get_line_height(g.font),0x808080ff);
    egg_texture_get_size(&hello->msg_texw,&hello->msg_texh,hello->msg_texid);
    return;
  }
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
  if (hello->option_slide<0.0) {
    if ((hello->option_slide+=elapsed*OPTION_SLIDE_RATE)>=0.0) hello->option_slide=0.0;
  } else if (hello->option_slide>0.0) {
    if ((hello->option_slide-=elapsed*OPTION_SLIDE_RATE)<=0.0) hello->option_slide=0.0;
  }
  return 0;
}

/* Draw option.
 */
 
static void hello_draw_option(struct hello *hello,struct option *option,int dstx,int dsty) {
  if (option->sub_texid) {
    graf_set_input(&g.graf,option->lbl_texid);
    graf_decal(&g.graf,dstx+(HELLO_OPTION_WIDTH>>1)-(option->lblw>>1),dsty+2,0,0,option->lblw,option->lblh);
    graf_set_input(&g.graf,option->sub_texid);
    graf_decal(&g.graf,dstx+(HELLO_OPTION_WIDTH>>1)-(option->subw>>1),dsty+HELLO_OPTION_HEIGHT-option->subh-2,0,0,option->subw,option->subh);
  } else {
    graf_set_input(&g.graf,option->lbl_texid);
    graf_decal(&g.graf,dstx+(HELLO_OPTION_WIDTH>>1)-(option->lblw>>1),dsty+(HELLO_OPTION_HEIGHT>>1)-(option->lblh>>1),0,0,option->lblw,option->lblh);
  }
}

/* Render.
 */
 
void hello_render(struct hello *hello) {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x080420ff);
  graf_set_input(&g.graf,g.texid_gross);
  
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
    render_mode7(
      dstx,90,
      0,101,167,121,
      -0.500,0.500,0.0
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
  graf_decal(&g.graf,0,titley,0,0,320,100);
  
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
    graf_set_input(&g.graf,hello->msg_texid);
    graf_decal(&g.graf,dstx,dsty,0,0,hello->msg_texw,hello->msg_texh);
    graf_set_alpha(&g.graf,0xff);
  }
  
  /* Dot left-to-right, in front of the title and unobscured.
   */
  if (!dotxform) {
    double normx=(dotperiod-6.0)/1.0;
    double rangex=FBW+167.0;
    int dstx=-167+(int)(normx*rangex);
    graf_set_input(&g.graf,g.texid_gross);
    graf_decal_xform(&g.graf,dstx,40,0,101,167,121,dotxform);
  }
  
  /* Options and cursor.
   */
  if ((hello->optionc>0)&&(hello->clock>=3.0)) {
    int dstx0=(FBW>>1)-(HELLO_OPTION_WIDTH>>1);
    int dsty=110;
    int i=hello->optionp;
    graf_fill_rect(&g.graf,dstx0,dsty,HELLO_OPTION_WIDTH,HELLO_OPTION_HEIGHT,0xc0a00060);
    dstx0+=hello->option_slide*HELLO_OPTION_WIDTH;
    int dstx=dstx0;
    while (dstx>-HELLO_OPTION_WIDTH) {
      if (i<0) i+=hello->optionc;
      hello_draw_option(hello,hello->optionv+i,dstx,dsty);
      dstx-=HELLO_OPTION_WIDTH;
      i--;
    }
    dstx=dstx0+HELLO_OPTION_WIDTH;
    i=hello->optionp+1;
    while (dstx<FBW) {
      if (i>=hello->optionc) i=0;
      hello_draw_option(hello,hello->optionv+i,dstx,dsty);
      dstx+=HELLO_OPTION_WIDTH;
      i++;
    }
  }
}
