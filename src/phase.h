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

void InitPhases(EntityAnimationState stt) {
    if (stt == ES_KING_PHASE_1) {
        phase_lst = { phases_mem, 0 };
        phase_lst.cap = 32;

        phase_lst.Add( InitPhase(5, 1, 600, 2.0f) ); // wait

        phase_lst.Add( InitPhase(10, 2, 300, 2.0f) );
        phase_lst.Add( InitPhase(10, 20, 120, 0.5f) );
        phase_lst.Add( InitPhase(10, 2, 300, 2.0f) );
        phase_lst.Add( InitPhase(10, 20, 120, 0.5f) );
        phase_lst.Add( InitPhase(10, 2, 300, 2.0f) );
        phase_lst.Add( InitPhase(10, 20, 120, 0.5f) );
        phase_lst.Add( InitPhase(10, 2, 300, 2.0f) );
        phase_lst.Add( InitPhase(10, 50, 120, 0.5f) );

        phase_lst.Add( InitPhasePause(120) );
    }
    else if (stt == ES_KING_PHASE_2) {
        phase_lst = { phases_mem, 0 };
        phase_lst.cap = 32;

        phase_lst.Add( InitPhase(20, 1, 100, 2.0f) ); // wait

        phase_lst.Add( InitPhase(40, 2, 200, 2.0f) );
        phase_lst.Add( InitPhase(10, 30, 120, 0.5f) );
        phase_lst.Add( InitPhase(40, 2, 300, 2.0f) );
        phase_lst.Add( InitPhase(20, 40, 120, 0.5f) );

        phase_lst.Add( InitPhasePause(120) );
    }
    else if (stt == ES_KING_PHASE_3) {
        phase_lst = { phases_mem, 0 };
        phase_lst.cap = 32;

        phase_lst.Add( InitPhase(5, 1, 100, 2.0f) ); // wait

        phase_lst.Add( InitPhase(40, 2, 500, 2.0f) );
        phase_lst.Add( InitPhase(30, 30, 120, 0.5f) );
        phase_lst.Add( InitPhase(30, 2, 400, 2.0f) );
        phase_lst.Add( InitPhase(30, 40, 120, 0.5f) );

        phase_lst.Add( InitPhasePause(120) );
    }
    else {
        assert(1 == 0 && "king state needed");
    }
}

bool IsShipControlled() {
    bool left = IsKeyDown(KEY_LEFT);
    bool right = IsKeyDown(KEY_RIGHT);
    bool up = IsKeyDown(KEY_UP);
    bool down = IsKeyDown(KEY_DOWN);

    return left || right || up || down;
}

bool IsOverKing() {
    bool result = (ship->position.y < king->position.y - king->ani_rect.height / 2 - 16);
    return result;
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

    else if (game.GetState() == GS_GAME) {
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

        if (king->position.y < screen_h - king->ani_rect.y / 2) {
            if (king->stt == ES_KING_PHASE_1) {
                // TODO:
                //game.SetState(GS_ADVANCE);

                StopMusicStream(music_track);
                music_track = LoadMusicStream("resources/Dreams.mp3");
                if (music) {
                    PlayMusicStream(music_track);
                }

                king->stt = ES_KING_PHASE_2;
                king->position.y = screen_h - 128;
                king_advance_tick += -2;
                ship_vy += 0.05f;

                InitPhases(king->stt);
            }
            else if (king->stt == ES_KING_PHASE_2 && (king->position.y < screen_h - king->ani_rect.y * 1.5f)) {
                // TODO:
                //game.SetState(GS_ADVANCE);

                king->stt = ES_KING_PHASE_3;
                king->position.y = screen_h - 128;
                king_advance_tick += -2;
                ship_vy += 0.05f;

                InitPhases(king->stt);
            }
            else if (king->stt == ES_KING_PHASE_3 && (king->position.y < screen_h - king->ani_rect.y * 2.0f)) {
                // TODO:
                //game.SetState(GS_ADVANCE);

                StopMusicStream(music_track);
                music_track = LoadMusicStream("resources/Nostalgia.mp3");
                if (music) {
                    PlayMusicStream(music_track);
                }

                game.SetState(GS_END);
            }
        }
    }
    game.phase_elapsed++;
}


#endif
