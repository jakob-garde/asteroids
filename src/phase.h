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
s32 phase_idx = 0;

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

void InitSpawnCycle(EntityState stt, f32 pause = 0.0f) {
    if (pause != 0.0f) {
        phase_lst = { phases_mem, 0 };
        phase_lst.Add( SpawnPhasePause(pause) );
    }
    else if (stt == ES_KING_PHASE_1) {
        phase_lst = { phases_mem, 0 };
        phase_lst.cap = 32;

        phase_lst.Add( SpawnPhaseInit(5, 1, 600, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(10, 2, 300, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(0, 10, 300, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(10, 2, 300, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(0, 10, 300, 0.05f) );
        phase_lst.Add( SpawnPhaseInit(10, 2, 300, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(10, 20, 120, 0.05f) );
        phase_lst.Add( SpawnPhaseInit(10, 2, 300, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(10, 50, 60, 0.03f) );

        phase_lst.Add( SpawnPhasePause(120) );
    }
    else if (stt == ES_KING_PHASE_2) {
        phase_lst = { phases_mem, 0 };
        phase_lst.cap = 32;

        phase_lst.Add( SpawnPhaseInit(20, 1, 100, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(40, 2, 200, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(10, 30, 120, 0.05f) );
        phase_lst.Add( SpawnPhaseInit(50, 2, 300, 0.1f) );
        phase_lst.Add( SpawnPhaseInit(20, 40, 120, 0.05f) );

        phase_lst.Add( SpawnPhasePause(120) );
    }
    else if (stt == ES_KING_PHASE_3) {
        phase_lst = { phases_mem, 0 };
        phase_lst.cap = 32;

        phase_lst.Add( SpawnPhaseInit(5, 1, 100, 0.15f) );
        phase_lst.Add( SpawnPhaseInit(50, 10, 500, 0.2f) );
        phase_lst.Add( SpawnPhaseInit(30, 30, 120, 0.15f) );
        phase_lst.Add( SpawnPhaseInit(30, 2, 400, 0.15f) );
        phase_lst.Add( SpawnPhaseInit(30, 40, 120, 0.15f) );

        phase_lst.Add( SpawnPhasePause(120) );
    }
    else {
        assert(1 == 0 && "king state needed");
    }

    phase_idx = 0;
}

bool IsOverKing() {
    bool result = (ship->position.y < king->position.y - king->ani_rect.height / 2 - 16);
    return result;
}

void FrameUpdatePhase() {
    f32 dt = GetFrameTimeMS();

    if (game.GetState() == GS_RESPAWN) {
        if (game.phase_elapsed == 0) {
            Entity s = ShipCreate();
            s.position = { screen_w / 2, screen_h + s.ani_rect.height / 2 };
            s.velocity = { 0, -0.1f };
            ship = entities.Add(s);
        }
        else if ((IsOverKing() == false)) {
            // wait
        }
        else if ((IsOverKing() == true)) {
            game.SetState(GS_GAME);

            ship->velocity = {};
        }
        else {
            ship->Update(dt);
        }
    }

    if (game.GetState() == GS_GAME || game.GetState() == GS_RESPAWN) {
        SpawnPhase phase = phase_lst.arr[phase_idx];
        SpawnAsteroids(&entities, dt, phase.spawn_ast_small, phase.spawn_ast_med, phase.spawn_sigma);

        if (game.phase_elapsed % 20 == 0) {
            if (CheckCollisionPointCircle(ship->position, king->position, king->coll_radius)) {
                // TODO: the king must speak! MOVE, LITTLE ONE!
                king->position.y += king_advance_small;
            }
            else {
                king->position.y += king_advance_tick;
            }
        }

        // switch to next phase
        if (game.phase_elapsed > phase.duration) {
            game.phase_elapsed = 0;
            phase_idx = (phase_idx + 1) % phase_lst.len;
        }

        if (king->position.y < screen_h - king->ani_rect.y / 2 - king->ani_rect.y) {
            if (king->state == ES_KING_PHASE_1) {
                // TODO:
                //game.SetState(GS_ADVANCE);

                king->state = ES_KING_PHASE_2;
                king->position.y = screen_h - 150;
                king_advance_tick += -2;
                ship_vy += 0.05f;

                InitSpawnCycle(king->state);
                SetMusicTrack(&music_track_action);
            }
            else if (king->state == ES_KING_PHASE_2 && (king->position.y < screen_h - king->ani_rect.y * 1.5f)) {
                // TODO:
                //game.SetState(GS_ADVANCE);

                king->state = ES_KING_PHASE_3;
                king->position.y = screen_h - 150;
                king_advance_tick += -2;
                ship_vy += 0.05f;

                InitSpawnCycle(king->state);
            }
            else if (king->state == ES_KING_PHASE_3 && (king->position.y < screen_h - king->ani_rect.y * 2.0f)) {
                // TODO:
                //game.SetState(GS_ADVANCE);

                SetMusicTrack(&music_track_end);
                game.SetState(GS_END);
            }
        }
    }

    game.phase_elapsed++;
}


#endif
