#include "raylib.h"
#include "memory.h"
#include "entities.h"

#define ARENA_CAP 1024 * 1024 * 64
u8 arena_mem[ARENA_CAP];

enum GameState {
    GS_TITLE,
    GS_GAME,
    GS_END,
};
struct AsteroidGame {
    GameState state;
};

AsteroidGame game;
Array<Animation> animations;

void LoadAssets(MArena *a_dest) {
    animations = InitArray<Animation>(a_dest, 14);

    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_01.png", ET_AST_SMALL) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_02.png", ET_AST_SMALL) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_03.png", ET_AST_SMALL) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_04.png", ET_AST_SMALL) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_05.png", ET_AST_SMALL) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_06.png", ET_AST_SMALL) );

    animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_01.png", ET_AST_MED) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_02.png", ET_AST_MED) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_03.png", ET_AST_MED) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_04.png", ET_AST_MED) );

    animations.AddSafe( InitAnimation(a_dest, "resources/ast_large_02.png", ET_AST_LARGE) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_large_01.png", ET_AST_LARGE) );

    animations.AddSafe( InitAnimation(a_dest, "resources/ast_brutal_01.png", ET_AST_BRUTE) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_brutal_02.png", ET_AST_BRUTE) );
}

void InitRaylib() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Asteroids");
    SetTargetFPS(60);
}

void LoadTestEntities(Array<Entity> *entities) {
    for (s32 i = 0; i < 10; ++i) {
        Entity sml = {};
        sml.tpe = ET_AST_SMALL;
        sml.facing_right = true;
        sml.ani_cnt = 1;
        sml.ani_idx0 = GetRandomValue(0, 5);
        f32 sz = 8;

        f32 x = GetRandomValue(0, GetScreenWidth() - 1);
        f32 y = GetRandomValue(0, GetScreenHeight() - 1);
        sml.anchor = { x, y };
        f32 vx = GetRandomValue(-100, 100) / 500.0f;
        f32 vy = GetRandomValue(-100, 100) / 500.0f;
        sml.velocity = { vx, vy };
        sml.vrot = GetRandomValue(-100, 100) / 300.0f;

        // @ update:
        sml.ani_rect = { sml.anchor.x - sz/2, sml.anchor.y - sz/2, sz, sz };
        entities->AddSafe(sml);
    }
}

void Run() {
    InitRaylib();
    f32 w = GetScreenWidth();
    f32 h = GetScreenHeight();

    MArena a = ArenaCreate(arena_mem, ARENA_CAP);
    Array<Entity> ents = InitArray<Entity>(&a, 256);
    Array<Entity> ents_next = InitArray<Entity>(&a, 256);
    LoadAssets(&a);
    LoadTestEntities(&ents);

    while (!WindowShouldClose()) {
        f32 dt = GetFrameTime() * 1000;


        // draw
        BeginDrawing();
        ClearBackground(WHITE);
        for (s32 i = 0; i < ents.len; ++i) {
            Entity *ent = ents.arr + i;
            if (ent->tpe != ET_AST_SMALL) {
                assert(1 == 0);
            }

            Frame frame = ent->GetFrame(animations);
            DrawTexturePro(frame.tex, frame.source, ent->ani_rect, {3.5, 3.5}, ent->rot, BLACK);
        }
        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)) {
            LoadTestEntities(&ents);
            printf("loading entities\n");
            printf("live entities: %d\n", ents.len);
        }
        if (IsKeyPressed(KEY_TAB)) {
            printf("live entities: %d\n", ents.len);
        }


        // update & copy
        for (s32 i = 0; i < ents.len; ++i) {
            Entity *ent = ents.arr + i;
            if (ent->tpe == ET_AST_SMALL) {
                ent->rot += dt * ent->vrot;
                ent->anchor.x += dt * ent->velocity.x;
                ent->anchor.y += dt * ent->velocity.y;
                ent->ani_rect.x = ent->anchor.x;
                ent->ani_rect.y = ent->anchor.y;

            }
            if (ent->anchor.x < 0 || ent->anchor.y < 0 || ent->anchor.x > w || ent->anchor.y > h) {
                // don't copy to next frame
                continue;
            }

            ents_next.Add(*ent);
        }

        // swap
        Array<Entity> swap = ents;
        ents = ents_next;
        ents_next = swap;
        ents_next.len = 0;
    }

    UnloadTextures(animations);
    CloseWindow();
}

int main(void) {
    Run();
}
