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
Entity *ship;

bool pause;
bool debug;
bool music;

s32 AnimationGetFirstByType(EntityType tpe) {
    for (s32 i = 0; i < animations.len; ++i) {
        Animation ani = animations.arr[i];
        if (ani.tpe == tpe) {
            return i;
        }
    }
    return {};
}

// phases

struct Phase {
    s32 spawn_ast_small;
    s32 spawn_ast_med;
    s32 elapsed;

    bool respawn;
    bool play;
};

Phase phase;
Phase phase_play;
Phase phase_respawn;

void SetPhasePlay() {
    assert(ship != NULL && "ensure a ship entity exists");

    ship->velocity = {};
    phase = phase_play;
    phase.elapsed = 0;
}

void SetPhaseRespawn() {
    phase = phase_respawn;
    phase.elapsed = 0;
}


#endif
