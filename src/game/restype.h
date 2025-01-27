/* restype.h
 * We maintain a global repository of map, tilesheet, and sprite, built up at init.
 * Everything in these resources is immutable.
 */
 
#ifndef RESTYPE_H
#define RESTYPE_H

struct map {
  uint16_t id;
  uint16_t w,h;
  const uint8_t *v;
  const uint8_t *cmdv;
  int cmdc;
  const uint8_t *physics;
  uint16_t imageid;
  struct sbox { // Static solids.
    double x,y,w,h;
  } *sboxv;
  int sboxc,sboxa;
  //TODO pre-pull important fields
};

struct sprdef { // "sprite" resource
  uint16_t id;
  const uint8_t *cmdv;
  int cmdc;
  //TODO pre-pull important fields;
};

// Call during init, after everything's installed.
int restype_ready();

int map_install(int rid,const void *serial,int serialc);
const struct map *map_get(int rid);

int tilesheet_install(int rid,const void *serial,int serialc);
const uint8_t *tilesheet_get(int rid); // => Never fails. Straight zeroes if absent.

int sprdef_install(int rid,const void *serial,int serialc);
const struct sprdef *sprdef_get(int rid);

#endif
