#include "game.h"

struct g g={0};

void egg_client_quit(int status) {
}

int egg_client_init() {
  int fbw=0,fbh=0;
  egg_texture_get_status(&fbw,&fbh,1);
  if ((fbw!=FBW)||(fbh!=FBH)) {
    fprintf(stderr,"Framebuffer size mismatch. Metadata=%dx%d, header=%dx%d\n",fbw,fbh,FBW,FBH);
    return -2;
  }
  
  if ((g.romc=egg_get_rom(0,0))<=0) return -1;
  if (!(g.rom=malloc(g.romc))) return -1;
  if (egg_get_rom(g.rom,g.romc)!=g.romc) return -1;
  strings_set_rom(g.rom,g.romc);
  
  if (!(g.font=font_new())) return -1;
  if (font_add_image_resource(g.font,0x0020,RID_image_font9_0020)<0) return -1;
  if (font_add_image_resource(g.font,0x00a1,RID_image_font9_00a1)<0) return -1;
  if (font_add_image_resource(g.font,0x0400,RID_image_font9_0400)<0) return -1;
  
  if (egg_texture_load_image(g.texid_hero=egg_texture_new(),RID_image_hero)<0) return -1;
  if (egg_texture_load_image(g.texid_tiles=egg_texture_new(),RID_image_tiles)<0) return -1;
  if (egg_texture_load_image(g.texid_gross=egg_texture_new(),RID_image_gross)<0) return -1;
  
  // Iterate ROM and install resources we need to track -- map, tilesheet, sprite
  struct rom_reader reader;
  if (rom_reader_init(&reader,g.rom,g.romc)<0) return -1;
  struct rom_res *res;
  while (res=rom_reader_next(&reader)) {
    switch (res->tid) {
      case EGG_TID_map: if (map_install(res->rid,res->v,res->c)<0) return -1; break;
      case EGG_TID_tilesheet: if (tilesheet_install(res->rid,res->v,res->c)<0) return -1; break;
      case EGG_TID_sprite: if (sprdef_install(res->rid,res->v,res->c)<0) return -1; break;
    }
  }
  if (restype_ready()<0) return -1;
  
  char tmp[2];
  int tmpc=egg_store_get(tmp,2,"hiscore",7);
  if ((tmpc==2)&&(tmp[0]>='0')&&(tmp[0]<='9')&&(tmp[1]>='0')&&(tmp[1]<='9')) {
    g.hiscore=(tmp[0]-'0')*10+(tmp[1]-'0');
  }
  
  srand_auto();
  
  return 0;
}

static void main_begin_gameover() {
  if (g.gameover) gameover_del(g.gameover);
  if (!(g.gameover=gameover_new())) { egg_terminate(1); return; }
}

static void main_begin_game() {
  if (g.game) game_del(g.game);
  if (!(g.game=game_new())) { egg_terminate(1); return; }
  if (g.hello) {
    hello_del(g.hello);
    g.hello=0;
  }
  if (g.gameover) {
    gameover_del(g.gameover);
    g.gameover=0;
  }
}

static void main_begin_hello() {
  if (g.hello) hello_del(g.hello);
  if (!(g.hello=hello_new())) { egg_terminate(1); return; }
  if (g.game) {
    game_del(g.game);
    g.game=0;
  }
  if (g.gameover) {
    gameover_del(g.gameover);
    g.gameover=0;
  }
}

void egg_client_update(double elapsed) {

  int input=egg_input_get_one(0);
  if (input!=g.pvinput) {
    if ((input&EGG_BTN_AUX3)&&!(g.pvinput&EGG_BTN_AUX3)) { egg_terminate(0); return; }
    if (g.gameover) gameover_input(g.gameover,input,g.pvinput);
    else if (g.game) game_input(g.game,input,g.pvinput);
    else if (g.hello) hello_input(g.hello,input,g.pvinput);
    g.pvinput=input;
  }
  
  if (g.gameover) {
    if (gameover_update(g.gameover,elapsed)<0) {
      main_begin_hello();
    }
  } else if (g.game) {
    if (game_update(g.game,elapsed)<0) {
      main_begin_gameover();
    }
  } else if (g.hello) {
    if (hello_update(g.hello,elapsed)<0) {
      main_begin_game();
    }
  } else {
    main_begin_hello();
  }
}

void egg_client_render() {
  graf_reset(&g.graf);
  if (g.gameover) gameover_render(g.gameover);
  else if (g.game) game_render(g.game);
  else if (g.hello) hello_render(g.hello);
  graf_flush(&g.graf);
}

void set_hiscore(int score) {
  if (score>99) score=99;
  if (score<=g.hiscore) return;
  g.hiscore=score;
  char tmp[2]={'0'+score/10,'0'+score%10};
  egg_store_set("hiscore",7,tmp,2);
}
