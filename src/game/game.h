#ifndef GAME_H
#define GAME_H

#include "egg/egg.h"
#include "opt/stdlib/egg-stdlib.h"
#include "opt/graf/graf.h"
#include "opt/text/text.h"
#include "opt/rom/rom.h"
#include "egg_rom_toc.h"
#include "shared_symbols.h"
#include "restype.h"

struct game;

extern struct g {
  void *rom;
  int romc;
  struct graf graf;
  struct font *font;
  int pvinput;
  struct game *game;
  int texid_hero;
  int texid_tiles;
} g;

/* "game" instance is one session of play.
 * If there's more than one level, they'll be separate instances.
 * Games may use globals, but they'll pointedly ignore (g.game).
 */
void game_del(struct game *game);
struct game *game_new();
void game_input(struct game *game,int input,int pvinput);
int game_update(struct game *game,double elapsed); // <0 to complete
void game_render(struct game *game);

#endif
