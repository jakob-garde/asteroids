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

    bool respawn;
    bool play;
};

Phase phases[3];
s32 phase_idx = 2;

void InitPhases() {
    phases[0].spawn_ast_small = 4;
    phases[0].spawn_ast_med = 0;
    phases[1].spawn_ast_small = 0;
    phases[1].spawn_ast_med = 4;
    phases[2].spawn_ast_small = 0;
    phases[2].spawn_ast_med = 16;
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

    //Array<Phase> phase_lst = { phases, 3 };
    Phase phase = phases[phase_idx];

    // spawn
    SpawnAsteroids(&entities, dt, phase.spawn_ast_small, phase.spawn_ast_med);

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

        if (game.phase_elapsed % 300 == 0) {

            phase_idx = (phase_idx + 1) % 3;
        }
    }

    game.phase_elapsed++;
}


#endif
