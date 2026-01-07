#include "raylib.h"

#include "memory.h"
#include "entities.h"
#include "types.h"

#include "asteroids.h"


Array<Animation> LoadAssets(MArena *a_dest) {
    animations = InitArray<Animation>(a_dest, 64);

    animations.Add( InitAnimation(a_dest, "resources/notebook.png", ET_AST_BACKGROUND, 0, 1, 1) );

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
    animations.Add( InitAnimation(a_dest, "resources/shoot.png", ET_SHOOT, 0, 1, 8) );

    animations.Add( InitAnimation(a_dest, "resources/notebook_mask.png", ET_AST_BACKGROUND_MASK, 0, 1, 1) );

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

    SpawnStartupAsteroids(&entities, 0);

}

void Close() {
    UnloadTextures(animations);
    CloseWindow();
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
