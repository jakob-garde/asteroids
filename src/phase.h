#ifndef __PHASE_H__
#define __PHASE_H__


#include "raylib.h"
#include "memory.h"
#include "entities.h"
#include "asteroids.h"
#include "globals.h"


struct Phase {
    s32 spawn_ast_small;
    s32 spawn_ast_med;
};


Phase phase;
Phase phase_game;
Phase phase_respawn;

void InitPhases() {
    phase_game.spawn_ast_small = 16;
    phase_game.spawn_ast_med = 4;

    phase = phase_game;
}


void FrameUpdatePhase() {
    f32 dt = GetFrameTime() * 1000;

    // spawn
    SpawnAsteroids(&entities, dt, phase.spawn_ast_small, phase.spawn_ast_med);
}


#endif
