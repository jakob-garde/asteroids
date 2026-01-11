#ifndef __PHASE_H__
#define __PHASE_H__


#include "raylib.h"
#include "memory.h"
#include "entities.h"
#include "globals.h"
#include "asteroids.h"
#include "ship.h"


struct Phase {
    s32 spawn_ast_small;
    s32 spawn_ast_med;

    bool respawn;
    bool play;
};

Phase phase_play;
Phase phase_respawn;


void InitPhases() {
    phase_play.spawn_ast_small = 16;
    phase_play.spawn_ast_med = 4;
    phase_play.play = true;

    phase_respawn.respawn = true;
    phase_respawn.play = false;
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


    if (game.GetState() == GS_RESPAWN) {
        // spawn
        SpawnAsteroids(&entities, dt, phase_respawn.spawn_ast_small, phase_respawn.spawn_ast_med);

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
        // spawn
        SpawnAsteroids(&entities, dt, phase_play.spawn_ast_small, phase_play.spawn_ast_med);

        if (game.phase_elapsed % 40 == 0) {
            king->position.y += -1;
        }
    }

    game.phase_elapsed++;
}


#endif
