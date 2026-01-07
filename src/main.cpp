#include "raylib.h"
#include "memory.h"
#include "entities.h"

// for release builds, we should set DEBUG_BUILD = 0
#ifndef DEBUG_BUILD
#define DEBUG_BUILD 1
#endif

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

    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_01.png", ET_AST_SMALL, 0, 6) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_02.png", ET_AST_SMALL, 1, 6) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_03.png", ET_AST_SMALL, 2, 6) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_04.png", ET_AST_SMALL, 3, 6) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_05.png", ET_AST_SMALL, 4, 6) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_06.png", ET_AST_SMALL, 5, 6) );

    animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_01.png", ET_AST_MED, 0, 4) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_02.png", ET_AST_MED, 1, 4) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_03.png", ET_AST_MED, 2, 4) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_04.png", ET_AST_MED, 3, 4) );

    animations.AddSafe( InitAnimation(a_dest, "resources/ast_large_02.png", ET_AST_LARGE, 0, 2) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_large_01.png", ET_AST_LARGE, 1, 2) );

    animations.AddSafe( InitAnimation(a_dest, "resources/ast_brutal_01.png", ET_AST_BRUTE, 0, 2) );
    animations.AddSafe( InitAnimation(a_dest, "resources/ast_brutal_02.png", ET_AST_BRUTE, 1, 2) );
}


Entity CreateEntity(EntityType tpe) {
    // sets as many things as possible on the entity of type tpe, for single-frame and animated entities

    for (s32 i = 0; i < animations.len; ++i) {
        Animation *ani = animations.arr + i;
        if (ani->tpe == tpe) {

            if (DEBUG_BUILD) {
                // check internal consistence of animations:
                //      - must be consecutive
                //      - group_size must match found count
                for (s32 j = 0; j < ani->group_sz; ++j) {
                    Animation ani_check = animations.arr[i + j];

                    assert( ani_check.group_sz == ani->group_sz  );
                    assert( ani_check.group_idx == j );
                    if (ani->frames.len == 1) {
                        assert( ani_check.frames.len == 1 );
                    }
                }
            }

            Entity ent = {};
            ent.tpe = tpe;
            ent.ani_idx0 = i;
            if (ani->frames.len == 1) {
                ent.ani_idx0 += GetRandomValue(0, ani->group_sz - 1);
            }
            ent.ani_cnt = ani->frames.len;
            ent.ani_offset = { ani->frame_w / 2.0f, ani->frame_h / 2.0f };
            ent.ani_rect = { ent.anchor.x, ent.anchor.y, 1.0f * ani->frame_w, 1.0f * ani->frame_h };
            ent.coll_offset = { ani->frame_w / 2.0f, ani->frame_h / 2.0f };
            ent.coll_rect = { ent.anchor.x, ent.anchor.y, 1.0f * ani->frame_w, 1.0f * ani->frame_h };
            ent.coll_radius = fmin( ani->frame_w, ani->frame_h );

            return ent;
        }
    }
}


f32 ship_vy = 0.0f;

Entity LoadAsteroid(EntityType tpe) {
    Entity ast = CreateEntity(tpe); 
    f32 x = GetRandomValue(0, GetScreenWidth() - 1);
    f32 y = GetRandomValue(0, GetScreenHeight() - 1);
    ast.anchor = { x, y };
    f32 vx = GetRandomValue(-100, 100) / 700.0f;
    f32 vy = GetRandomValue(-100, 100) / 700.0f + ship_vy;
    ast.velocity = { vx, vy };
    ast.vrot = GetRandomValue(-100, 100) / 300.0f;
    ast.Update(0);
    return ast;
}


void LoadAsteroids(Array<Entity> *entities) {

    for (s32 i = 0; i < 10; ++i) {
        // @ update:
        entities->AddSafe( LoadAsteroid(ET_AST_SMALL) );
    }
    for (s32 i = 0; i < 5; ++i) {
        // @ update:
        entities->AddSafe( LoadAsteroid(ET_AST_MED) );
    }
    for (s32 i = 0; i < 3; ++i) {
        // @ update:
        entities->AddSafe( LoadAsteroid(ET_AST_LARGE) );
    }
    for (s32 i = 0; i < 3; ++i) {
        // @ update:
        entities->AddSafe( LoadAsteroid(ET_AST_BRUTE) );
    }
}


void InitRaylib() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Asteroids");
    SetTargetFPS(60);
}


void Run() {
    InitRaylib();
    f32 w = GetScreenWidth();
    f32 h = GetScreenHeight();

    MArena a = ArenaCreate(arena_mem, ARENA_CAP);
    Array<Entity> ents = InitArray<Entity>(&a, 256);
    Array<Entity> ents_next = InitArray<Entity>(&a, 256);
    LoadAssets(&a);
    LoadAsteroids(&ents);

    while (!WindowShouldClose()) {
        f32 dt = GetFrameTime() * 1000;


        // draw
        BeginDrawing();
        ClearBackground(WHITE);
        for (s32 i = 0; i < ents.len; ++i) {
            Entity *ent = ents.arr + i;

            Frame frame = ent->GetFrame(animations);
            DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, BLACK);
        }
        EndDrawing();


        // DBG

        if (IsKeyPressed(KEY_SPACE)) {
            LoadAsteroids(&ents);
            printf("loading entities\n");
            printf("live entities: %d\n", ents.len);
        }
        if (IsKeyPressed(KEY_TAB)) {
            printf("live entities: %d\n", ents.len);
        }


        f32 ship_delta_vy = 0;
        if (IsKeyPressed(KEY_UP)) {
            ship_delta_vy = 0.1f;
        }
        else if (IsKeyPressed(KEY_DOWN)) {
            ship_delta_vy = - 0.1f;
        }

        // update & copy
        for (s32 i = 0; i < ents.len; ++i) {
            Entity *ent = ents.arr + i;

            //if (ent->tpe == ET_AST_SMALL) {
            if (true /* is_asteroid */) {
                ent->rot += dt * ent->vrot;
                ent->anchor.x += dt * ent->velocity.x;
                ent->anchor.y += dt * ent->velocity.y;
                ent->ani_rect.x = ent->anchor.x;
                ent->ani_rect.y = ent->anchor.y;
                ent->velocity.y += ship_delta_vy;

            }
            if (ent->anchor.x < - ent->ani_rect.width 
                || ent->anchor.y < - ent->ani_rect.height
                || ent->anchor.x > w + ent->ani_rect.width
                || ent->anchor.y > h + ent->ani_rect.height)
            {
                // out of window - don't copy to next frame // 
                ent->deleted = true;
            }

            if (ent->deleted == false) {
                ents_next.Add(*ent);
            }
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
