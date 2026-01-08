#include "raylib.h"

// generic
#include "memory.h"
#include "entities.h"

// specific to this game
#include "globals.h"
#include "asteroids.h"
#include "ship.h"


/*
enum GameState {
    GS_TITLE,
    GS_GAME,
    GS_END,
};
struct AsteroidGame {
    GameState state;
};
AsteroidGame game;
*/


Array<SEffect> LoadSoundEffects(MArena *a_dest) {
    sounds = InitArray<SEffect>(a_dest, 64);

    sounds.Add( InitSoundEffect("resources/shoot.wav", SE_SHOOT) );
    sounds.Add( InitSoundEffect("resources/explosion.wav", SE_EXPLOSION) );
    sounds.Add( InitSoundEffect("resources/crash.wav", SE_CRASH) );

    return sounds;
}


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

    animations.Add( InitAnimation(a_dest, "resources/ship_idle.png", ET_SHIP, 0, 3) );
    animations.Add( InitAnimation(a_dest, "resources/ship_side.png", ET_SHIP, 1, 3) );
    animations.Add( InitAnimation(a_dest, "resources/ship_crash.png", ET_SHIP, 2, 3) );

    return animations;
}


Music music;

void Init() {
    // raylib
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Asteroids");
    InitAudioDevice();
    ToggleFullscreen();
    SetTargetFPS(60);
    screen_w = GetScreenWidth();
    screen_h = GetScreenHeight();

    // usr
    RandInit();

    MArena a = ArenaCreate(arena_mem, ARENA_CAP);
    entities_next = InitArray<Entity>(&a, 256);
    entities = InitArray<Entity>(&a, 256);
    animations = LoadAssets(&a);
    sounds = LoadSoundEffects(&a);

    // music loop
    music = LoadMusicStream("resources/Heartbeat.mp3");
    PlayMusicStream(music);
    float volume = 0.5f;
    SetMusicVolume(music, volume);

    // background
    background = CreateEntity(ET_AST_BACKGROUND, animations);
    background.disable_debug_draw = true;

    Frame frm = animations.arr[background.ani_idx0].frames.arr[0];
    f32 aspect = 1.0f * frm.source.width / frm.source.height;

    background.ani_offset = { 0, 0 };
    background.ani_rect.width = screen_h * aspect;
    background.ani_rect.height = screen_h;
    background.ani_rect.x = (screen_w - background.ani_rect.width) / 2.0f;
    background.ani_rect.y = 0;
    entities.Add( background );

    // bckgrnd mask
    mask = CreateEntity(ET_AST_BACKGROUND_MASK, animations);
    mask.disable_debug_draw = true;

    mask.ani_offset = { 0, 0 };
    mask.ani_rect.width = screen_h * aspect;
    mask.ani_rect.height = screen_h;
    mask.ani_rect.x = (screen_w - mask.ani_rect.width) / 2.0f;
    mask.ani_rect.y = 0;
    entities.Add( mask );

    mask_left = (screen_w - mask.ani_rect.width) / 2.0f + 32;
    mask_right = mask_left + mask.ani_rect.width - 64;
    mask_bottom = screen_h * 0.88f;
    mask_top = screen_h * 0.15f;

    // ship / player
    entities.Add(ShipCreate());

    // start
    SpawnStartupAsteroids(&entities, 0);
}

void Close() {
    UnloadTextures(animations);
    UnloadSounds(sounds);
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();
}

void FrameDrawAndSwap() {
    BeginDrawing();
    ClearBackground(BLACK);
    EntityDraw(animations, &background);

    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ent = entities.arr + i;
        if (IsAsteroid(ent->tpe)) {
            EntityDraw(animations, ent);
        }
        else if (ent->tpe == ET_SHOOT) {
            ShotDraw(ent);
        }
        else if (ent->tpe == ET_SHIP) {
            ShipDraw(ent);
        }

        if (debug) {
            EntityDrawDebug(ent);
        }
    }

    EntityDraw(animations, &mask);
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
    UpdateMusicStream(music);

    if (IsKeyPressed(KEY_R)) {

        for (s32 i = 0; i < entities.len; ++i) {
            Entity *ent = entities.arr + i;
            if (ent->tpe == ET_SHIP) {
                ent->stt = ES_SHIP_IDLE;
                ent->vrot = 0;
                ent->rot = 0;
                ent->velocity = {};
                break;
            }
        }

    }
    if (IsKeyPressed(KEY_D)) {
        debug = !debug;
    }
    if (IsKeyPressed(KEY_P)) {
        pause = !pause;
    }
    if (pause) {
        return;
    }

    // NOTE: ship_delta_vy is the ship speed relative to the "ambient" asteroids
    f32 ship_delta_vy = 0;
    ship_vy += ship_delta_vy;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        entities.Add( ShotCreate() );
    }

    // update
    f32 dt = GetFrameTime() * 1000;

    // spawn
    SpawnAsteroids(&entities, dt);

    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ent = entities.arr + i;

        if (IsAsteroid(ent->tpe)) {
            if (ent->disable_vy == false) {
                ent->position.y += dt * ship_vy;
            }
            ent->Update(dt);

            if (ent->life > 0) {
                ent->life--;
                ent->deleted = ent->life == 0;
            }

            // clip
            if (ent->position.x < mask_left
                || ent->position.y < - ent->ani_rect.height
                || ent->position.x > mask_right
                || ent->position.y > screen_h + ent->ani_rect.height)
            {
                if (ent->position.y < 0) {
                    // above window, let it run
                }
                else {
                    // out of window - don't copy to next frame // 
                    ent->deleted = true;
                }
            }
        }
        else if (ent->tpe == ET_SHOOT) {
            ShotUpdate(ent, dt);
        }
        else if (ent->tpe == ET_SHIP) {
            ShipUpdate(ent, dt);
        }
    }
}

void Run() {
    Init();

    while (!WindowShouldClose()) {
        FrameUpdate();
        FrameDrawAndSwap();
    }

    Close();
}


void Test() {

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Asteroids");
    InitAudioDevice();

    music = LoadMusicStream("resources/Heartbeat.ogg");
    PlayMusicStream(music);
    float volume = 1.0f;
    SetMusicVolume(music, volume);

    while (!WindowShouldClose()) {
        UpdateMusicStream(music);
        BeginDrawing();
        EndDrawing();
    }

}


int main(void) {
    Run();
    //Test();
}
