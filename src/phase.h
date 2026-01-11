#ifndef __PHASE_H__
#define __PHASE_H__


#include "raylib.h"
#include "memory.h"
#include "entities.h"
#include "globals.h"
#include "asteroids.h"
#include "ship.h"


struct Phase {
    f32 spawn_ast_small;
    f32 spawn_ast_med;
    s32 duration;
    f32 spawn_sigma;
};

Phase phases_mem[32];
Array<Phase> phase_lst;
s32 phase_idx = 0;



Phase InitPhase(f32 spawn_ast_small, f32 spawn_ast_med, s32 duration, f32 spawn_sigma) {
    Phase p = {};
    p.spawn_ast_small = spawn_ast_small;
    p.spawn_ast_med = spawn_ast_med;
    p.duration = duration;
    p.spawn_sigma = spawn_sigma;
    return p;
}
Phase InitPhasePause(s32 duration) {
    Phase p = InitPhase(0, 0, duration, 0);
    return p;
}


void InitPhases() {
    phase_lst = { phases_mem, 0 };
    phase_lst.cap = 32;

    phase_lst.Add( InitPhase(10, 0, 300, 2.0f) );
    //phase_lst.Add( InitPhasePause(120) );
    //phase_lst.Add( InitPhase(40, 0, 120, 1.0f) );
    //phase_lst.Add( InitPhasePause(120) );
    phase_lst.Add( InitPhase(10, 10, 300, 0.5f) );
    phase_lst.Add( InitPhasePause(120) );
}

bool IsShipControlled() {
    bool left = IsKeyDown(KEY_LEFT);
    bool right = IsKeyDown(KEY_RIGHT);
    bool up = IsKeyDown(KEY_UP);
    bool down = IsKeyDown(KEY_DOWN);

    return left || right || up || down;
}

void FrameUpdatePhase() {
    f32 dt = GetFrameTime() * 1000;
    Phase phase = phase_lst.arr[phase_idx];

    // spawn
    SpawnAsteroids(&entities, dt, phase.spawn_ast_small, phase.spawn_ast_med, 0.1f);

    if (game.GetState() == GS_RESPAWN) {
        if (game.phase_elapsed == 0) {
            Entity ship = ShipCreate();
            ship.position = { screen_w / 2, screen_h + ship.ani_rect.height / 2 };
            ship.velocity = { 0, -0.1f };
            entities.Add(ship);
        }
        else if (game.phase_elapsed >= 60 || (game.phase_elapsed >= 40 && IsShipControlled())) {
            game.SetState(GS_GAME);

            ship->velocity = {};
        }
        else {
            ship->Update(dt);
        }
    }

    else if (game.GetState() == GS_GAME) {
        if (game.phase_elapsed % 40 == 0) {
            king->position.y += -1;
        }

        // switch to next phase
        if (game.phase_elapsed > phase.duration) {
            game.phase_elapsed = 0;
            phase_idx = (phase_idx + 1) % phase_lst.len;
        }

    }
    game.phase_elapsed++;
}


#endif
