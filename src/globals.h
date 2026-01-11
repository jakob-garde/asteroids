#ifndef __ASTEROIDS_TYPES_H__
#define __ASTEROIDS_TYPES_H__


#include "memory.h"
#include "entities.h"


#define ARENA_CAP 1024 * 1024 * 64
u8 arena_mem[ARENA_CAP];


Array<Animation> animations;
Array<Entity> entities;
Array<Entity> entities_next;
Array<SEffect> sounds;

Music music_track;

f32 screen_w;
f32 screen_h;
f32 mask_left;
f32 mask_right;
f32 mask_top;
f32 mask_bottom;

f32 ship_vy = 0.2f;

Entity background;
Entity mask;

bool pause;
bool debug;
bool music;

#endif
