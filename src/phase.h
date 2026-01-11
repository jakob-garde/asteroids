#ifndef __PHASE_H__
#define __PHASE_H__


#include "raylib.h"
#include "memory.h"
#include "entities.h"
#include "asteroids.h"
#include "ship.h"
#include "globals.h"


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

    // spawn
    SpawnAsteroids(&entities, dt, phase.spawn_ast_small, phase.spawn_ast_med);

    if (phase.respawn) {
        if (phase.elapsed == 0) {
            Entity ship = ShipCreate();
            ship.position = { screen_w / 2, screen_h + ship.ani_rect.height / 2 };
            ship.velocity = { 0, -0.1f };
            entities.Add(ship);
        }
        else if (phase.elapsed >= 60) {
            SetPhasePlay();
        }
        else if (phase.elapsed >= 40 && IsShipControlled()) {
            SetPhasePlay();
        }
        else {
            ship->Update(dt);
        }
    }

    phase.elapsed++;
}


#endif
