#ifndef __ASTEROIDS_TYPES_H__
#define __ASTEROIDS_TYPES_H__


#include "memory.h"
#include "entities.h"


#define ARENA_CAP 1024 * 1024 * 64
u8 arena_mem[ARENA_CAP];


enum GameState {
    GS_TITLE,
    GS_GAME,
    GS_RESPAWN,
    GS_ADVANCE,
    GS_END,
};

struct AsteroidGame {
private:
    GameState state;
public:
    s32 phase_elapsed;

    void SetState(GameState stt) {
        state = stt;
        phase_elapsed = 0;
    }
    GameState GetState() {
        return state;
    }
};
AsteroidGame game;

bool pause;
bool debug;
bool music_enabled = true;

Array<Animation> animations;
Array<Entity> entities;
Array<Entity> entities_next;
Array<SEffect> sounds;

Music music_track_chill;
Music music_track_action;
Music music_track_end;
Music *music_track;

void SetMusicTrack(Music *track) {
    if (music_track) {
        StopMusicStream(*music_track);
    }
    if (music_enabled) {
        PlayMusicStream(*track);
    }
    music_track = track;
}

f32 screen_w;
f32 screen_h;
f32 mask_left;
f32 mask_right;
f32 mask_top;
f32 mask_bottom;

f32 ship_vy = 0.1f;
s32 king_advance_small = 1;
s32 king_advance_tick = - 2;
s32 king_advance_med = 10;
s32 king_advance_shoot = - 2;

Entity background;
Entity mask;
Entity *ship;
Entity *king;

s32 AnimationGetFirstByType(EntityType tpe) {
    for (s32 i = 0; i < animations.len; ++i) {
        Animation ani = animations.arr[i];
        if (ani.tpe == tpe) {
            return i;
        }
    }
    return {};
}

Array<s32> AnimationGetAllByType(MArena *a_tmp, EntityType tpe) {
    Array<s32> result = {};
    result.InitForExpand(a_tmp);

    for (s32 i = 0; i < animations.len; ++i) {
        Animation ani = animations.arr[i];
        if (ani.tpe == tpe) {
            result.Expand(a_tmp, i);
        }
    }
    return result;
}

// helpers

inline
f32 GetFrameTimeMS() {
    f32 result = GetFrameTime() * 1000;
    return result;
}


#endif
