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

Array<Entity> entities;
AsteroidGame game;
Array<Animation> animations;

Array<Animation> LoadAssets(MArena *a_dest) {
    Array<Animation> _animations = InitArray<Animation>(a_dest, 14);

    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_01.png", ET_AST_SMALL, 0, 6) );
    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_02.png", ET_AST_SMALL, 1, 6) );
    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_03.png", ET_AST_SMALL, 2, 6) );
    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_04.png", ET_AST_SMALL, 3, 6) );
    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_05.png", ET_AST_SMALL, 4, 6) );
    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_small_06.png", ET_AST_SMALL, 5, 6) );

    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_01.png", ET_AST_MED, 0, 4) );
    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_02.png", ET_AST_MED, 1, 4) );
    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_03.png", ET_AST_MED, 2, 4) );
    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_medium_04.png", ET_AST_MED, 3, 4) );

    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_large_02.png", ET_AST_LARGE, 0, 2) );
    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_large_01.png", ET_AST_LARGE, 1, 2) );

    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_brutal_01.png", ET_AST_BRUTE, 0, 2) );
    _animations.AddSafe( InitAnimation(a_dest, "resources/ast_brutal_02.png", ET_AST_BRUTE, 1, 2) );

    return _animations;
}

bool IsAsteroid(EntityType tpe) {
    bool result = tpe == ET_AST_SMALL
        || tpe == ET_AST_MED
        || tpe == ET_AST_LARGE
        || tpe == ET_AST_BRUTE;
    return result;
}


f32 ship_vy = 0.0f;

Entity CreateAsteroid(EntityType tpe, bool on_screen = false) {
    Entity ast = CreateEntity(tpe, animations);

    if (on_screen) {
        f32 x = GetRandomValue(0, GetScreenWidth() - 1);
        f32 y = GetRandomValue(0, GetScreenHeight() - 1);
        ast.anchor = { x, y };
    }
    else {
        f32 x = GetRandomValue(0, GetScreenWidth() - 1);
        f32 y = GetRandomValue(-512, -64);
        ast.anchor = { x, y };
    }

    f32 vx = GetRandomValue(-100, 100) / 700.0f;
    f32 vy = GetRandomValue(-100, 100) / 700.0f + ship_vy;
    ast.velocity = { vx, vy };

    ast.rot = GetRandomValue(-100, 100);
    ast.vrot = GetRandomValue(-100, 100) / 300.0f;
    ast.Update(0);

    return ast;
}


inline
bool DoSpawn(f32 dt, f32 vy, f32 rate) {
    f32 vy_base = 0.1f;
    bool result = Rand01() < rate * dt * (abs(vy) + vy_base) / 1000.0f;
    return result;
}

void SpawnAsteroids(Array<Entity> *entities, f32 dt, f32 vy) {
    f32 rate_small = 16; // asteroids per second
    f32 rate_med = 2;

    if (DoSpawn(dt, vy, rate_small)) {
        entities->AddSafe( CreateAsteroid(ET_AST_SMALL) );
    }
    if (DoSpawn(dt, vy, rate_med)) {
        entities->AddSafe( CreateAsteroid(ET_AST_MED) );
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

    RandInit();
    MArena a = ArenaCreate(arena_mem, ARENA_CAP);
    Array<Entity> entities_next = InitArray<Entity>(&a, 256);
    entities = InitArray<Entity>(&a, 256);
    animations = LoadAssets(&a);

    s32 asteroid_avg_cnt = 32;


    while (!WindowShouldClose()) {
        f32 dt = GetFrameTime() * 1000;


        // draw
        BeginDrawing();
        ClearBackground(WHITE);
        for (s32 i = 0; i < entities.len; ++i) {
            Entity *ent = entities.arr + i;

            Frame frame = ent->GetFrame(animations);
            DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, BLACK);
        }
        EndDrawing();


        // DBG

        if (IsKeyPressed(KEY_SPACE)) {
            printf("loading entities\n");
            printf("live entities: %d\n", entities.len);
        }
        if (IsKeyPressed(KEY_TAB)) {
            printf("live entities: %d\n", entities.len);
        }


        f32 ship_delta_vy = 0;
        if (IsKeyPressed(KEY_UP)) {
            ship_delta_vy = 0.1f;
        }
        else if (IsKeyPressed(KEY_DOWN)) {
            ship_delta_vy = - 0.1f;
        }
        ship_vy += ship_delta_vy;

        s32 asteroid_fcnt = 0;

        // update & copy
        for (s32 i = 0; i < entities.len; ++i) {
            Entity *ent = entities.arr + i;

            if (IsAsteroid(ent->tpe)) {
                ent->rot += dt * ent->vrot;
                ent->anchor.x += dt * ent->velocity.x;
                ent->anchor.y += dt * ent->velocity.y;
                ent->ani_rect.x = ent->anchor.x;
                ent->ani_rect.y = ent->anchor.y;
                ent->velocity.y += ship_delta_vy;

                if (ent->anchor.x < - ent->ani_rect.width 
                    || ent->anchor.y < - ent->ani_rect.height
                    || ent->anchor.x > w + ent->ani_rect.width
                    || ent->anchor.y > h + ent->ani_rect.height)
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

        // load new ones if there aren't too many
        SpawnAsteroids(&entities, dt, ship_vy);

        // swap
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

    UnloadTextures(animations);
    CloseWindow();
}

int main(void) {
    Run();
}
