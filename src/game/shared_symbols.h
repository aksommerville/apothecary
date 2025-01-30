/* shared_symbols.h
 * Consumed by both the game and the tools.
 */

#ifndef SHARED_SYMBOLS_H
#define SHARED_SYMBOLS_H

#define FBW 320
#define FBH 180

#define NS_sys_tilesize 16
#define NS_sys_mapw 20
#define NS_sys_maph 15

#define CMD_map_image     0x20 /* u16:imageid */
#define CMD_map_hero      0x21 /* u16:pos, must be exactly one */
#define CMD_map_pickup    0x22 /* u16:pos, must be exactly one */
#define CMD_map_neighbors 0x60 /* u16:left u16:right u16:up u16:down */
#define CMD_map_sprite    0x61 /* u16:pos u16:spriteid u32:reserved */
#define CMD_map_dropoff   0x62 /* u16:pos u8:tileid u8:difficulty(XXX) u32:reserved. */
#define CMD_map_door      0x62 /* u16:pos u16:mapid u16:dstpos u16:reserved */

#define CMD_sprite_image  0x20 /* u16:imageid */
#define CMD_sprite_tile   0x21 /* u8:tileid u8:xform */

#define NS_tilesheet_physics     1
#define NS_tilesheet_neighbors   0
#define NS_tilesheet_family      0
#define NS_tilesheet_weight      0

#define NS_physics_vacant 0
#define NS_physics_solid  1
#define NS_physics_water 2

#define NS_target_pickup 1
#define NS_target_dropoff 2

#endif
