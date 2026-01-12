#ifndef __SHIP_H__
#define __SHIP_H__


#include "raylib.h"
#include "memory.h"
#include "entities.h"
#include "globals.h"


// create

Entity ShipCreate() {
    Entity ent = CreateEntity(ET_SHIP, animations, false);

    ent.state = ES_SHIP_RESPAWN;
    ent.position.x = (background_mask_left + background_mask_right) / 2;
    ent.position.y = screen_h - 256;
    ent.coll_radius *= 0.8f;

    return ent;
}

f32 KingHeightAtTop() {
    f32 result = king->position.y - king->ani_rect.height;
    return result;
}
f32 KingHeightForAdvance() {
    f32 result = screen_h - king->ani_rect.height * 2.3f;
    return result;
}
f32 KingStartingHeight() {
    f32 result = (f32) screen_h;
    return result;
}

Entity KingCreate() {
    Entity king = CreateEntity(ET_KING, animations);

    king.state = ES_KING_PHASE_0;
    king.duration = 60000; // duration before king should automatically start to advance
    king.position = { screen_w / 2.0f, KingStartingHeight() };
    king.Update(0);

    return king;
}

Entity ShotCreate(Vector2 position) {
    Entity ent = CreateEntity(ET_SHOOT, animations);
    ent.state = ES_SHOOT_CHARGE;

    ent.position = position;
    ent.ani_offset.y = ent.ani_rect.height;
    ent.velocity.y = -0.9;

    return ent;
}

// draw

void KingDraw(Entity *ent) {
    Animation ani = animations.arr[ent->ani_idx0 + ent->ani_idx];
    Frame frame = ani.frames.arr[ent->frame_idx];

    if (frame.duration == 0) {
    }

    else if (ent->frame_elapsed > frame.duration) {
        EntityState ks = ent->state;
        if (ks == ES_KING_ADVANCE) {
            ks = ent->state_next;
        }

        if (ks == ES_KING_PHASE_0) {
            ent->frame_elapsed = 0;
            ent->frame_idx = 0;
            frame = ani.frames.arr[ent->frame_idx];
        }
        else if (ks == ES_KING_PHASE_1) {
            ent->frame_elapsed = 0;
            ent->frame_idx = (ent->frame_idx - 1 + 1) % 2 + 1;
            frame = ani.frames.arr[ent->frame_idx];
        }
        else if (ks == ES_KING_PHASE_2) {
            ent->frame_elapsed = 0;
            ent->frame_idx = (ent->frame_idx - 3 + 1) % 2 + 3;
            frame = ani.frames.arr[ent->frame_idx];
        }
        else if (ks == ES_KING_PHASE_3) {
            ent->frame_elapsed = 0;
            ent->frame_idx = (ent->frame_idx - 5 + 1) % 2 + 5;
            frame = ani.frames.arr[ent->frame_idx];
        }
        else {
            printf("%d\n", ent->state);
            assert(1 == 0 && "set king animation state");
        }
    }

    DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);
}

void ShotDraw(Entity *ent) {
    Animation ani = animations.arr[ent->ani_idx0 + ent->ani_idx];
    Frame frame = ani.frames.arr[ent->frame_idx];

    if (ent->frame_elapsed > frame.duration) {
        if (ent->state == ES_SHOOT_CHARGE) {
            if (ent->frame_idx == 2) {
                ent->state = ES_SHOOT_RELEASE;
            }
            ent->frame_idx = ent->frame_idx + 1;
        }
        else if (ent->state == ES_SHOOT_RELEASE) {
            ent->frame_idx = 3 + (ent->frame_idx + 1) % 5;
        }
        else {
            assert(1 == 0);
        }
        frame = ani.frames.arr[ent->frame_idx];
        ent->frame_elapsed = 0;
    }

    DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);
}

void ShipDraw(Entity *ent) {
    if (ent->state == ES_SHIP_IDLE) {
        ent->ani_idx = 0;
    }
    else if (ent->state == ES_SHIP_LEFT || ent->state == ES_SHIP_RIGHT) {
        ent->ani_idx = 1;
    }

    Animation ani = animations.arr[ent->ani_idx0 + ent->ani_idx];
    Frame frame = ani.frames.arr[ent->frame_idx];

    if (ent->facing_left) {
        frame = frame.Mirror();
    }

    DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);
}

// update

void ShipUpdate(Entity *ent, f32 dt) {
    // crash
    bool crash = false;
    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ast = entities.arr + i;
        if (IsAsteroid(ast->tpe)) {

            if (CheckCollisionCircles(ast->position, ast->coll_radius, ent->position, ent->coll_radius)) {
                crash = true;

                break;
            }
        }
    }
    if (IsKeyPressed(KEY_TAB) || crash) {
        PlaySoundEffect(SE_CRASH, sounds);

        ent->tpe = ET_SHIP_CHASH;
        ent->life_frames = 300;
        ent->ani_idx0 = AnimationGetFirstByType(ET_SHIP_CHASH);
        ent->ani_idx = 0;

        ent->facing_left = RandBin();
        ent->vrot = 0.05f * RandPM1();
        ent->velocity.x = RandPM1() * 0.1f;
        ent->velocity.y = RandPM1() * 0.1f;

        // trigger ship respawn
        ship_do_respawn = true;

        return;
    }

    if (ent->state != ES_SHIP_RESPAWN) {
        ent->state = ES_SHIP_IDLE;
        ent->facing_left = false;
        if (IsKeyDown(KEY_LEFT)) {
            ent->state = ES_SHIP_LEFT;
            ent->facing_left = true;

            if (ent->position.x > background_mask_left) {
                ent->position.x -= ship_movement_speed * dt;
            }
        }
        else if (IsKeyDown(KEY_RIGHT)) {
            ent->state = ES_SHIP_RIGHT;
            ent->facing_left = false;

            if (ent->position.x < background_mask_right) {
                ent->position.x += ship_movement_speed * dt;
            }
        }

        if (IsKeyDown(KEY_UP)) {
            if (ent->position.y > background_mask_top) {
                ent->position.y -= ship_movement_speed * dt;
            }
        }
        else if (IsKeyDown(KEY_DOWN)) {
            if (ent->position.y < background_mask_bottom) {
                ent->position.y += ship_movement_speed * dt;
            }
        }

        if (IsKeyPressed(KEY_SPACE)) {
            Vector2 shot_pos = ent->position;
            shot_pos.y -= 32;
            Entity shot = ShotCreate( shot_pos );
            PlaySoundEffect(SE_SHOOT, sounds);

            entities.Add(shot);
        }
    }

    ent->Update(dt);
}

void KingUpdate(Entity *ent, f32 dt) {
    bool king_do_advance = (ent->state != ES_KING_PHASE_0) && (ent->state != ES_KING_ADVANCE);

    if (king->state == ES_KING_ADVANCE) {
        if (king->elapsed == 0) {
            king->velocity.y = 0.1;
        }
        else if (king->position.y >= KingStartingHeight()) {
            king->elapsed = 0;
            king->state = king->state_next;
            king->position.y = KingStartingHeight();
            king->velocity = {};
        }
    }

    else if (ent->elapsed >= king_advance_interval_ms && king_do_advance) {
        ent->elapsed = 0;
        if (ship && CheckCollisionPointCircle(ship->position, ent->position, ent->coll_radius)) {
            // TODO: the king must speak! MOVE, LITTLE ONE!
        }
        else if (ship) {
            ent->position.y += - 1;
        }
    }

    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ast = entities.arr + i;
        if (ast->tpe == ET_AST_SMALL) {
            if (CheckCollisionCircles(ast->position, ast->coll_radius, ent->position, ent->coll_radius)) {
                ast->deleted = true;

                Entity exp = CreateEntity(ET_EXPLOSION_MED, animations);
                exp.position = ast->position;
                exp.velocity = { ast->velocity.x * 0.05f, (ast->velocity.y + ship_global_vy) * 0.05f };
                exp.life_frames = 60;
                exp.Update(0);
                entities.Add(exp);

                if (king_do_advance) {
                    ent->position.y += king_advance_small;
                }
            }
        }
        else if (ast->tpe == ET_AST_MED) {
            if (CheckCollisionCircles(ast->position, ast->coll_radius, ent->position, ent->coll_radius)) {
                ast->deleted = true;
                Entity exp = CreateEntity(ET_EXPLOSION_LARGE, animations);
                exp.position = ast->position;
                exp.velocity = { ast->velocity.x * 0.05f, (ast->velocity.y + ship_global_vy) * 0.05f };
                exp.life_frames = 120;
                exp.Update(0);
                entities.Add(exp);

                if (king_do_advance) {
                    ent->position.y += king_advance_med;
                }
            }
        }
    }

    ent->frame_elapsed += dt;
    ent->elapsed += dt;
    ent->Update(dt);
}

void ShotUpdate(Entity *ent, f32 dt) {
    if (ent->frame_idx == 2) {
        ent->position.y += dt * ent->velocity.y;
    }
    else if (ent->frame_idx > 2) {
        ent->position.y += dt * ent->velocity.y;
    }
    ent->ani_rect.x = ent->position.x;
    ent->ani_rect.y = ent->position.y;
    ent->frame_elapsed += dt;

    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ast = entities.arr + i;
        if (IsAsteroid(ast->tpe)) {

            if (CheckCollisionPointCircle(ent->position, ast->position, ast->coll_radius)) {
                if (ast->tpe == ET_AST_SMALL) {
                    ast->deleted = true;
                }
                else if (ast->tpe == ET_AST_MED) {
                    med_kill_cnt++;
                    ast->deleted = true;
                    for (s32 i = 0; i < 16; ++i) {
                        Entity sml = CreateAsteroid(ET_AST_SMALL, 0);
                        sml.disable_vy = true;
                        sml.position = ast->position;
                        sml.life_frames = Rand(60) + 10 * i; // number of frames
                        f32 vx = GetRandomValue(-100, 100) / 2000.0f;
                        f32 vy = GetRandomValue(-100, 100) / 2000.0f;
                        sml.velocity = { vx, vy };
                        entities.Add( sml );
                    }

                    PlaySoundEffect(SE_EXPLOSION, sounds);
                    king->position.y += king_advance_shoot;
                }

                ent->deleted = true;
                break;
            }
        }
    }

    if (ent->position.y < -200) {
        ent->deleted = true;
    }
}


#endif
