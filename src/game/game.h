#ifndef GAME_H
#define GAME_H

#include "egg/egg.h"
#include "util/stdlib/egg-stdlib.h"
#include "util/graf/graf.h"
#include "util/text/text.h"
#include "util/font/font.h"
#include "util/res/res.h"
#include "egg_res_toc.h"
#include "shared_symbols.h"
#include "restype.h"

struct game;
struct hello;
struct gameover;

extern struct g {
  void *rom;
  int romc;
  struct graf graf;
  struct font *font;
  int pvinput;
  
  /* Only one of the three is active at a time.
   * Listed here in order of precedence.
   * (game) should exist while (gameover) is active.
   */
  struct gameover *gameover;
  struct game *game;
  struct hello *hello;
  
  int texid_hero;
  int texid_tiles;
  int texid_gross;
  
  double hiscore; // Seconds. It's stored in milliseconds; we convert at load and save.
  int hiscore_is_new;
  
  int enable_music;
  int enable_sound;
  int songid;
  
  int skip_next_gameover;
} g;

// Noop if the existing hiscore is higher.
void set_hiscore(double s);

/* Use these instead of egg_play_song/egg_play_sound; they're savvy to the enable flags.
 * play_song(g.songid) if you change enable_music.
 */
void play_song(int rid);
void play_sound(int rid);

/* "game" instance is one session of play.
 * If there's more than one level, they'll be separate instances.
 * Games may use globals, but they'll pointedly ignore (g.game).
 */
void game_del(struct game *game);
struct game *game_new();
void game_input(struct game *game,int input,int pvinput);
int game_update(struct game *game,double elapsed); // <0 to complete
void game_render(struct game *game);

/* In the interest of avoiding over-generalization, the two menus are completely concrete.
 * But they also export identical interfaces (identical to game too).
 */
void gameover_del(struct gameover *gameover);
struct gameover *gameover_new();
void gameover_input(struct gameover *go,int input,int pvinput);
int gameover_update(struct gameover *go,double elapsed);
void gameover_render(struct gameover *go);

void hello_del(struct hello *hello);
struct hello *hello_new();
void hello_input(struct hello *hello,int input,int pvinput);
int hello_update(struct hello *hello,double elapsed);
void hello_render(struct hello *hello);

/* We use this for four things: Dot at Hello, Dot's Shadow, Dot, and Offscreen Arrows.
 * It was a built-in render mode in Egg v1 but Egg v2 has no such thing. We fake it in game_render.c.
 * Caller must graf_set_input() first.
 */
void render_mode7(
  int dstx,int dsty,
  int srcx,int srcy,int srcw,int srch,
  double xscale,double yscale,double rotate
);

#endif
