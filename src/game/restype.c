#include "game.h"

/* Globals.
 * All stores are packed, index is (rid-1).
 */
 
static const uint8_t tilesheet_default[256]={0};
 
static struct map *mapv=0;
static int mapa=0;
static uint8_t *tilesheetv=0; // Table 0 only.
static int tilesheeta=0; // Count of resources; 256 bytes each.
static struct sprdef *sprdefv=0;
static int sprdefa=0;

/* Rebuild list of static boxes from cell physics.
 */
 
static int map_row_solid(const uint8_t *physics,const uint8_t *p,int c) {
  for (;c-->0;p++) if (physics[*p]!=NS_physics_solid) return 0;
  return 1;
}
 
static int map_generate_sboxv(struct map *map) {
  map->sboxc=0;
  int cellc=map->w*map->h;
  uint8_t *visited=calloc(cellc,1);
  if (!visited) return -1;
  
  int p=0,row=0;
  for (;row<map->h;row++) {
    int col=0;
    for (;col<map->w;col++,p++) {
      if (visited[p]) continue;
      if (map->physics[map->v[p]]!=NS_physics_solid) continue;
      
      // Extend as far right as we can, then as far down.
      int colc=1;
      while ((col+colc<map->w)&&(map->physics[map->v[p+colc]]==NS_physics_solid)) colc++;
      int rowc=1;
      while ((row+rowc<map->h)&&map_row_solid(map->physics,map->v+p+rowc*map->w,colc)) rowc++;
      
      // Visit all.
      int yi=rowc; while (yi-->0) {
        int xi=colc; while (xi-->0) {
          visited[p+yi*map->w+xi]=1;
        }
      }
      
      // Add to sboxv.
      if (map->sboxc>=map->sboxa) {
        int na=map->sboxa+128;
        if (na>INT_MAX/sizeof(struct sbox)) return -1;
        void *nv=realloc(map->sboxv,sizeof(struct sbox)*na);
        if (!nv) return -1;
        map->sboxv=nv;
        map->sboxa=na;
      }
      struct sbox *sbox=map->sboxv+map->sboxc++;
      sbox->x=col*NS_sys_tilesize;
      sbox->y=row*NS_sys_tilesize;
      sbox->w=colc*NS_sys_tilesize;
      sbox->h=rowc*NS_sys_tilesize;
    }
  }
  
  //fprintf(stderr,"map:%d, sboxc=%d\n",map->id,map->sboxc);
  free(visited);
  return 0;
}

/* Finish installation.
 */
 
int restype_ready() {
  struct map *map=mapv;
  int i=mapa;
  for (;i-->0;map++) {
    if (!map->id) continue;
    map->physics=tilesheet_get(map->imageid);
    if (map_generate_sboxv(map)<0) return -1;
  }
  return 0;
}

/* Install map.
 */

int map_install(int rid,const void *serial,int serialc) {
  if (rid<1) return -1;
  int na=rid+1;
  if (na>mapa) {
    na=(na+16)&~15;
    if (na>INT_MAX/sizeof(struct map)) return -1;
    void *nv=realloc(mapv,sizeof(struct map)*na);
    if (!nv) return -1;
    mapv=nv;
    memset(mapv+mapa,0,sizeof(struct map)*(na-mapa));
    mapa=na;
  }
  struct map *dst=mapv+rid-1;
  struct rom_map src;
  if (rom_map_decode(&src,serial,serialc)<0) return -1;
  dst->id=rid;
  dst->w=src.w;
  dst->h=src.h;
  dst->v=src.v;
  dst->cmdv=src.cmdv;
  dst->cmdc=src.cmdc;
  struct rom_command_reader reader={.v=src.cmdv,.c=src.cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_image: dst->imageid=(cmd.argv[0]<<8)|cmd.argv[1]; break;
    }
  }
  return 0;
}

/* Install tilesheet.
 */

int tilesheet_install(int rid,const void *serial,int serialc) {
  if (rid<1) return -1;
  int na=rid+1;
  if (na>tilesheeta) {
    na=(na+16)&~15;
    if (na>INT_MAX/256) return -1;
    void *nv=realloc(tilesheetv,256*na);
    if (!nv) return -1;
    tilesheetv=nv;
    memset(tilesheetv+tilesheeta*256,0,256*(na-tilesheeta));
    tilesheeta=na;
  }
  uint8_t *dst=tilesheetv+(rid-1)*256;
  struct rom_tilesheet_reader reader;
  if (rom_tilesheet_reader_init(&reader,serial,serialc)<0) return -1;
  struct rom_tilesheet_entry entry;
  while (rom_tilesheet_reader_next(&entry,&reader)>0) {
    if (entry.tableid!=NS_tilesheet_physics) continue;
    memcpy(dst+entry.tileid,entry.v,entry.c);
  }
  return 0;
}

/* Install sprite.
 */

int sprdef_install(int rid,const void *serial,int serialc) {
  if (rid<1) return -1;
  int na=rid+1;
  if (na>sprdefa) {
    na=(na+16)&~15;
    if (na>INT_MAX/sizeof(struct sprdef)) return -1;
    void *nv=realloc(sprdefv,sizeof(struct sprdef)*na);
    if (!nv) return -1;
    sprdefv=nv;
    memset(sprdefv+sprdefa,0,sizeof(struct sprdef)*(na-sprdefa));
    sprdefa=na;
  }
  struct sprdef *dst=sprdefv+rid-1;
  dst->id=rid;
  struct rom_sprite rspr;
  if (rom_sprite_decode(&rspr,serial,serialc)<0) return -1;
  struct rom_command_reader reader={.v=rspr.cmdv,.c=rspr.cmdc};
  struct rom_command cmd;
  while (rom_command_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      //TODO which sprite fields do we need?
      //TODO ...do we need sprites?
    }
  }
  return 0;
}

/* Get instances.
 */
 
const struct map *map_get(int rid) {
  if ((rid<1)||(rid>mapa)) return 0;
  struct map *map=mapv+rid-1;
  if (!map->id) return 0;
  return map;
}

const uint8_t *tilesheet_get(int rid) {
  if ((rid<1)||(rid>tilesheeta)) return tilesheet_default;
  return tilesheetv+(rid-1)*256;
}

const struct sprdef *sprdef_get(int rid) {
  if ((rid<1)||(rid>sprdefa)) return 0;
  struct sprdef *sprdef=sprdefv+rid-1;
  if (!sprdef->id) return 0;
  return sprdef;
}
