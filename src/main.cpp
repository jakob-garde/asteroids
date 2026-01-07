#include "raylib.h"

#include "memory.h"
#include "entities.h"
#include "types.h"

#include "asteroids.h"


Array<Animation> LoadAssets(MArena *a_dest) {
    animations = InitArray<Animation>(a_dest, 64);

    animations.Add( InitAnimation(a_dest, "resources/shoot.png", ET_SHOOT, 0, 1, 8) );
    animations.Add( InitAnimation(a_dest, "resources/notebook.png", ET_AST_BACKGROUND, 0, 1, 1) );
    animations.Add( InitAnimation(a_dest, "resources/notebook_mask.png", ET_AST_BACKGROUND_MASK, 0, 1, 1) );

    animations.Add( InitAnimation(a_dest, "resources/ast_small_01.png", ET_AST_SMALL, 0, 6) );
    animations.Add( InitAnimation(a_dest, "resources/ast_small_02.png", ET_AST_SMALL, 1, 6) );
    animations.Add( InitAnimation(a_dest, "resources/ast_small_03.png", ET_AST_SMALL, 2, 6) );
    animations.Add( InitAnimation(a_dest, "resources/ast_small_04.png", ET_AST_SMALL, 3, 6) );
    animations.Add( InitAnimation(a_dest, "resources/ast_small_05.png", ET_AST_SMALL, 4, 6) );
    animations.Add( InitAnimation(a_dest, "resources/ast_small_06.png", ET_AST_SMALL, 5, 6) );
    animations.Add( InitAnimation(a_dest, "resources/ast_medium_01.png", ET_AST_MED, 0, 4) );
    animations.Add( InitAnimation(a_dest, "resources/ast_medium_02.png", ET_AST_MED, 1, 4) );
    animations.Add( InitAnimation(a_dest, "resources/ast_medium_03.png", ET_AST_MED, 2, 4) );
    animations.Add( InitAnimation(a_dest, "resources/ast_medium_04.png", ET_AST_MED, 3, 4) );
    animations.Add( InitAnimation(a_dest, "resources/ast_large_02.png", ET_AST_LARGE, 0, 2) );
    animations.Add( InitAnimation(a_dest, "resources/ast_large_01.png", ET_AST_LARGE, 1, 2) );
    animations.Add( InitAnimation(a_dest, "resources/ast_brutal_01.png", ET_AST_BRUTAL, 0, 2) );
    animations.Add( InitAnimation(a_dest, "resources/ast_brutal_02.png", ET_AST_BRUTAL, 1, 2) );

    animations.Add( InitAnimation(a_dest, "resources/ship_idle.png", ET_SHIP, 0, 2) );
    animations.Add( InitAnimation(a_dest, "resources/ship_side.png", ET_SHIP, 1, 2) );

    return animations;
}

void Init() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Asteroids");
    ToggleFullscreen();
    SetTargetFPS(60);

    screen_w = GetScreenWidth();
    screen_h = GetScreenHeight();

    RandInit();
    MArena a = ArenaCreate(arena_mem, ARENA_CAP);
    entities_next = InitArray<Entity>(&a, 256);
    entities = InitArray<Entity>(&a, 256);
    animations = LoadAssets(&a);

    background = CreateEntity(ET_AST_BACKGROUND, animations);
    Frame frm = background.GetFrame(animations);
    f32 aspect = 1.0f * frm.source.width / frm.source.height;
    background.ani_offset = { 0, 0 };
    background.ani_rect.width = screen_h * aspect;
    background.ani_rect.height = screen_h;
    background.ani_rect.x = (screen_w - background.ani_rect.width) / 2.0f;
    background.ani_rect.y = 0;
    entities.Add( background );

    mask = CreateEntity(ET_AST_BACKGROUND_MASK, animations);
    frm = mask.GetFrame(animations);
    mask.ani_offset = { 0, 0 };
    mask.ani_rect.width = screen_h * aspect;
    mask.ani_rect.height = screen_h;
    mask.ani_rect.x = (screen_w - mask.ani_rect.width) / 2.0f;
    mask.ani_rect.y = 0;
    entities.Add( mask );

    mask_left = (screen_w - mask.ani_rect.width) / 2.0f + 32;
    mask_right = mask_left + mask.ani_rect.width - 64;

    Entity ship = CreateEntity(ET_SHIP, animations, false);
    ship.stt = ES_SHIP_IDLE;
    ship.anchor.x = (mask_left + mask_right) / 2;
    ship.anchor.y = 200;
    ship.Update(0);
    entities.Add(ship);

    SpawnStartupAsteroids(&entities, 0);
}

void Close() {
    UnloadTextures(animations);
    CloseWindow();
}

Entity ShotCreate() {
    Entity shot = CreateEntity(ET_SHOOT, animations);
    shot.stt = ES_SHOOT_CHARGE;

    shot.anchor = { (f32) GetMouseX(), (f32) GetMouseY() };
    shot.ani_offset.y = shot.ani_rect.height;
    shot.velocity.y = -0.9;
    shot.Update(0);

    return shot;
}

Frame ShotGetFrame(Entity *shot) {
    Animation ani = animations.arr[shot->ani_idx0 + shot->ani_idx];
    Frame frame = ani.frames.arr[shot->frame_idx];

    if (frame.duration == 0) {
        return frame;
    }

    if (shot->frame_elapsed > frame.duration) {
        if (shot->stt == ES_SHOOT_CHARGE) {
            if (shot->frame_idx == 2) {
                shot->stt = ES_SHOOT_RELEASE;
            }
            shot->frame_idx = shot->frame_idx + 1;
        }
        else if (shot->stt == ES_SHOOT_RELEASE) {
            shot->frame_idx = 3 + (shot->frame_idx + 1) % 5;
        }
        else {
            assert(1 == 0);
        }
        frame = ani.frames.arr[shot->frame_idx];
        shot->frame_elapsed = 0;

        printf("%d\n", shot->frame_idx);
    }

    return frame;
}

void ShotUpdate(Entity *shot, f32 dt) {
    if (shot->frame_idx == 2) {
        shot->anchor.y += dt * shot->velocity.y;
    }
    else if (shot->frame_idx > 2) {
        shot->anchor.y += dt * shot->velocity.y;
    }
    shot->ani_rect.x = shot->anchor.x;
    shot->ani_rect.y = shot->anchor.y;
    shot->frame_elapsed += dt;
}


void ShipUpdate(Entity *ship, f32 dt) {
    ship->facing_left = false;
    ship->stt = ES_SHIP_IDLE;

    f32 speed = 0.4f;

    if (IsKeyDown(KEY_LEFT)) {
        ship->stt = ES_SHIP_LEFT;
        ship->facing_left = true;

        ship->anchor.x -= speed * dt;
    }
    else if (IsKeyDown(KEY_RIGHT)) {
        ship->stt = ES_SHIP_RIGHT;

        ship->anchor.x += speed * dt;
    }

    if (IsKeyDown(KEY_UP)) {
        ship->anchor.y -= speed * dt;
    }
    else if (IsKeyDown(KEY_DOWN)) {
        ship->anchor.y += speed * dt;
    }
    ship->Update(dt);


    if (IsKeyPressed(KEY_SPACE)) {
        Entity shot = ShotCreate();
        shot.anchor = ship->anchor;
        shot.anchor.y -= 4;
        shot.anchor.x += 32;
        shot.Update(0);
        entities.Add(shot);
    }

}

Frame ShipGetFrame(Entity *ship) {
    if (ship->stt == ES_SHIP_IDLE) {
        ship->ani_idx = 0;
    }
    else {
        ship->ani_idx = 1;
    }

    Animation ani = animations.arr[ship->ani_idx0 + ship->ani_idx];
    Frame frame = ani.frames.arr[ship->frame_idx];

    if (ship->stt == ES_SHIP_LEFT) {
        return frame.Mirror();
    }
    else {
        return frame;
    }
}

void FrameDrawAndSwap() {
    BeginDrawing();
    ClearBackground(BLACK);

    Frame frame = background.GetFrame(animations);
    Entity *ent = &background;
    DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);

    for (s32 i = 0; i < entities.len; ++i) {
        ent = entities.arr + i;
        if (IsAsteroid(ent->tpe)) {
            frame = ent->GetFrame(animations);
            DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);
        }
        else if (ent->tpe == ET_SHOOT) {
            frame = ShotGetFrame(ent);
            DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);
        }
        else if (ent->tpe == ET_SHIP) {
            frame = ShipGetFrame(ent);
            DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);
        }
    }

    frame = mask.GetFrame(animations);
    ent = &mask;
    DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);

    EndDrawing();

    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ent = entities.arr + i;

        if (ent->deleted == false) {
            entities_next.Add(*ent);
        }
    }

    Array<Entity> swap = entities;
    entities = entities_next;
    entities_next = swap;
    entities_next.len = 0;
}


void FrameUpdate() {
    f32 ship_delta_vy = 0;
    if (IsKeyPressed(KEY_UP)) {
        ship_delta_vy = 0.1f;
    }
    else if (IsKeyPressed(KEY_DOWN)) {
        ship_delta_vy = - 0.1f;
    }
    ship_vy += ship_delta_vy;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        entities.Add( ShotCreate() );
    }

    // update
    s32 asteroid_fcnt = 0;
    f32 dt = GetFrameTime() * 1000;
    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ent = entities.arr + i;

        if (IsAsteroid(ent->tpe)) {
            ent->rot += dt * ent->vrot;
            ent->anchor.x += dt * ent->velocity.x;
            ent->anchor.y += dt * ent->velocity.y;
            ent->ani_rect.x = ent->anchor.x;
            ent->ani_rect.y = ent->anchor.y;
            ent->velocity.y += ship_delta_vy;

            if (ent->anchor.x < mask_left
                || ent->anchor.y < - ent->ani_rect.height
                || ent->anchor.x > mask_right
                || ent->anchor.y > screen_h + ent->ani_rect.height)
            {
                if (ent->anchor.y < 0) {
                    // above window
                }
                else {
                    // out of window - don't copy to next frame // 
                    ent->deleted = true;
                }
            }
            else {
                asteroid_fcnt++;
            }
        }
        else if (ent->tpe == ET_SHOOT) {
            ShotUpdate(ent, dt);

            if (ent->anchor.y < -200) {
                ent->deleted = true;
            }
        }
        else if (ent->tpe == ET_SHIP) {
            ShipUpdate(ent, dt);
        }
    }

    // spawn
    SpawnAsteroids(&entities, dt, ship_vy);
}

void Run() {
    Init();

    while (!WindowShouldClose()) {
        FrameUpdate();
        FrameDrawAndSwap();
    }

    Close();
}

int main(void) {
    Run();
}
