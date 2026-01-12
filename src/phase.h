#ifndef __PHASE_H__
#define __PHASE_H__


#include "raylib.h"
#include "memory.h"
#include "entities.h"
#include "globals.h"
#include "asteroids.h"
#include "ship.h"


struct SpawnPhase {
    f32 spawn_ast_small;
    f32 spawn_ast_med;
    s32 duration;
    f32 spawn_sigma;
};

SpawnPhase phases_mem[32];
Array<SpawnPhase> phase_lst;
s32 phase_idx;
s32 phase_selected;

SpawnPhase SpawnPhaseInit(f32 spawn_ast_small, f32 spawn_ast_med, s32 duration, f32 spawn_sigma) {
    SpawnPhase p = {};
    p.spawn_ast_small = spawn_ast_small;
    p.spawn_ast_med = spawn_ast_med;
    p.duration = duration;
    p.spawn_sigma = spawn_sigma;
    return p;
}

SpawnPhase SpawnPhasePause(s32 duration) {
    SpawnPhase p = SpawnPhaseInit(0, 0, duration, 0);
    return p;
}

void InitSpawnCycle(s32 selector, f32 pause = 0.0f) {
    phase_selected = selector;

    if (pause != 0.0f) {
        phase_lst = { phases_mem, 0 };
        phase_lst.Add( SpawnPhasePause(pause) );
        phase_selected = -1;
    }

    else if (selector == 0) {
        phase_lst = { phases_mem, 0 };
        phase_lst.cap = 32;

        phase_lst.Add( SpawnPhaseInit(5, 4, 10000, 0.1f) );
    }

    else if (selector == 1) {
        phase_lst = { phases_mem, 0 };
        phase_lst.cap = 32;

        phase_lst.Add( SpawnPhaseInit(5, 1, 10000, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(10, 2, 5000, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(0, 10, 5000, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(10, 2, 5000, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(0, 10, 5000, 0.05f) );
        phase_lst.Add( SpawnPhaseInit(10, 2, 5000, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(10, 20, 3000, 0.05f) );
        phase_lst.Add( SpawnPhaseInit(10, 2, 5000, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(10, 50, 1000, 0.03f) );

        phase_lst.Add( SpawnPhasePause(3000) );
    }

    else if (selector == 2) {
        phase_lst = { phases_mem, 0 };
        phase_lst.cap = 32;

        phase_lst.Add( SpawnPhaseInit(20, 3, 1600, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(30, 3, 3200, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(10, 10, 2000, 0.05f) );
        phase_lst.Add( SpawnPhaseInit(20, 3, 5000, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(20, 10, 2000, 0.05f) );

        phase_lst.Add( SpawnPhasePause(2000) );
    }

    else if (selector == 3) {
        phase_lst = { phases_mem, 0 };
        phase_lst.cap = 32;

        phase_lst.Add( SpawnPhaseInit(5, 1, 1600, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(20, 8, 8000, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(40, 4, 2000, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(20, 5, 6400, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(5, 15, 2000, 0.05f) );

        phase_lst.Add( SpawnPhasePause(2000) );
    }

    else {
        printf("%d\n", selector);
        assert(1 == 0 && "unknown selector");
    }

    phase_idx = 0;
}

void FrameUpdateLevel01() {
    // init
    if (phase_lst.len == 0) {
        InitSpawnCycle(0);
    }

    f32 dt = GetFrameTimeMS();

    // trigger ship respawn
    if (ship == NULL) {
        ship_do_respawn = true;
    }

    // update
    if ((ship != NULL) || ship_do_respawn) {
        if (ship_do_respawn) {
            ship_do_respawn = false;

            Entity s = ShipCreate();
            s.state = ES_SHIP_RESPAWN;
            s.position = { screen_w / 2, screen_h + s.ani_rect.height / 2 };
            s.velocity = { 0, -0.1f };
            ship = entities.Add(s);
            game.SetState(GS_GAME);
        }
        else if (((ship->position.y < king->position.y - king->ani_rect.height / 2 - 4) == false)) {
            // wait
        }
        else if (((ship->position.y < king->position.y - king->ani_rect.height / 2 - 4) == true)) {
            ship->state = ES_SHIP_IDLE;
            ship->velocity = {};
        }
        else {
            ship->Update(dt);
        }
    }

    SpawnPhase phase = phase_lst.arr[phase_idx];
    SpawnAsteroids(&entities, dt, phase.spawn_ast_small, phase.spawn_ast_med, phase.spawn_sigma);

    // switch to next phase
    if (game.phase_elapsed > phase.duration) {
        game.phase_elapsed = 0;
        phase_idx = (phase_idx + 1) % phase_lst.len;
    }

    if (king->state == ES_KING_PHASE_0) {
        king->elapsed += dt;

        if (med_kill_cnt >= med_kill_for_advance || king->elapsed > king->duration) {
            king->elapsed = 0;
            king->state = ES_KING_PHASE_1;
            InitSpawnCycle(1);
        }
    }

    // kingship advance
    else if ((king->state == ES_KING_PHASE_1) && (KingHeightAtTop() <= KingHeightForAdvance())) {
        king->state = ES_KING_ADVANCE;
        king->state_next = ES_KING_PHASE_2;
        king->elapsed = 0;

        ship_global_vy += 0.05f;
        king_advance_interval_ms = 230;
        star_size *= 1.5;
        SetStarVelocities(star_velocity * 2);

        InitSpawnCycle(2);
        SetMusicTrack(&music_track_action);
    }
    else if ((king->state == ES_KING_PHASE_2) && (KingHeightAtTop() <= KingHeightForAdvance())) {
        king->state = ES_KING_ADVANCE;
        king->state_next = ES_KING_PHASE_3;
        king->elapsed = 0;

        ship_global_vy += 0.05f;
        king_advance_interval_ms = 150;
        star_size *= 1.3;
        SetStarVelocities(star_velocity * 3);

        InitSpawnCycle(3);
    }

    // set end screen
    else if ((king->state == ES_KING_PHASE_3) && (KingHeightAtTop() <= KingHeightForAdvance())) {
        king_advance_interval_ms = 50;
        SetStarVelocities(star_velocity * 5);
        game.phase_elapsed = 0;

        SetMusicTrack(&music_track_end);

        game.SetState(GS_END);
        return;
    }

    game.phase_elapsed += dt;
}

void FrameUpdateEnd() {
    f32 dt = GetFrameTimeMS();
    if (game.phase_elapsed > 20000 && game.phase_elapsed < 20020) {
        ship->state = ES_SHIP_RESPAWN; // stops player control
        ship->velocity.x = 0;
        ship->velocity.y = - ship_movement_speed * 1.3;
        star_velocity *= 1.5;
        game.phase_elapsed += dt;
    }
    else {
        game.phase_elapsed += dt;
    }
}


#endif
