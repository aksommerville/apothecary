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
  
  if (egg_texture_load_image(g.texid_hero=egg_texture_new(),RID_image_hero)<0) return -1;
  if (egg_texture_load_image(g.texid_tiles=egg_texture_new(),RID_image_tiles)<0) return -1;
  
  srand_auto();
  
  return 0;
}

void egg_client_update(double elapsed) {

  int input=egg_input_get_one(0);
  if (input!=g.pvinput) {
    if ((input&EGG_BTN_AUX3)&&!(g.pvinput&EGG_BTN_AUX3)) { egg_terminate(0); return; }
    if (g.game) game_input(g.game,input,g.pvinput);
    g.pvinput=input;
  }
  
  if (g.game) {
    if (game_update(g.game,elapsed)<0) {
      game_del(g.game);
      g.game=0;
    }
  } else {
    //TODO Other top-level modes, eg menu.
    if (!(g.game=game_new())) {
      egg_terminate(1);
      return;
    }
  }
}

void egg_client_render() {
  graf_reset(&g.graf);
  if (g.game) {
    game_render(g.game);
  }
  graf_flush(&g.graf);
}
