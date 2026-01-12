#include "raylib.h"

// generic
#include "memory.h"
#include "entities.h"

// specific to this game
#include "phase.h"
#include "globals.h"
#include "asteroids.h"
#include "ship.h"


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

    animations.Add( InitAnimation(a_dest, "resources/expl_01.png", ET_EXPLOSION_SMALL, 0, 1) );
    animations.Add( InitAnimation(a_dest, "resources/expl_02.png", ET_EXPLOSION_MED, 0, 1) );
    animations.Add( InitAnimation(a_dest, "resources/expl_03.png", ET_EXPLOSION_LARGE, 0, 1) );

    animations.Add( InitAnimation(a_dest, "resources/ship_idle.png", ET_SHIP, 0, 2) );
    animations.Add( InitAnimation(a_dest, "resources/ship_side.png", ET_SHIP, 1, 2) );
    animations.Add( InitAnimation(a_dest, "resources/ship_crash.png", ET_SHIP_CHASH, 0, 1) );

    animations.Add( InitAnimation(a_dest, "resources/kingship_256-sheet.png", ET_KING, 0, 1) );

    return animations;
}

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

    // music
    music_track_chill = LoadMusicStream("resources/Waves.mp3");
    music_track_action = LoadMusicStream("resources/Nostalgia.mp3");
    music_track_end = LoadMusicStream("resources/Dreams.mp3");
    SetMusicTrack(&music_track_chill);

    float volume = 0.5f;
    SetMusicVolume(music_track_chill, volume);
    SetMusicVolume(music_track_action, volume);
    SetMusicVolume(music_track_end, volume);
    f32 master_volume = 0.3f;
    SetMasterVolume(master_volume);

    // background
    f32 bcgrnd_aspect = 0;
    {
        background = CreateEntity(ET_AST_BACKGROUND, animations);
        background.disable_debug_draw = true;

        Frame frm = animations.arr[background.ani_idx0].frames.arr[0];
        bcgrnd_aspect = 1.0f * frm.source.width / frm.source.height;

        background.ani_offset = { 0, 0 };
        background.ani_rect.width = screen_h * bcgrnd_aspect;
        background.ani_rect.height = screen_h;
        background.ani_rect.x = (screen_w - background.ani_rect.width) / 2.0f;
        background.ani_rect.y = 0;
    }

    // bckgrnd mask
    {
        mask = CreateEntity(ET_AST_BACKGROUND_MASK, animations);
        mask.disable_debug_draw = true;

        mask.ani_offset = { 0, 0 };
        mask.ani_rect.width = screen_h * bcgrnd_aspect;
        mask.ani_rect.height = screen_h;
        mask.ani_rect.x = (screen_w - mask.ani_rect.width) / 2.0f;
        mask.ani_rect.y = 0;

        mask_left = (screen_w - mask.ani_rect.width) / 2.0f + 32;
        mask_right = mask_left + mask.ani_rect.width - 64;
        mask_bottom = screen_h * 0.88f;
        mask_top = screen_h * 0.15f;
    }

    // kingship
    Entity k = KingCreate();
    king = FindFirstEntityByType(ET_KING, entities);
    InitSpawnCycle(k.state);

    // start
    game.SetState(GS_RESPAWN);
}

void Close() {
    UnloadTextures(animations);

    UnloadSounds(sounds);
    UnloadMusicStream(music_track_chill);
    UnloadMusicStream(music_track_action);
    UnloadMusicStream(music_track_end);
    CloseAudioDevice();

    CloseWindow();
}

void FrameDrawAndSwap() {
    BeginDrawing();
    ClearBackground(BLACK);
    EntityDraw(animations, &background);

    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ent = entities.arr + i;
        if (ent->tpe == ET_SHOOT) {
            ShotDraw(ent);
        }
        else if (ent->tpe == ET_SHIP) {
            ShipDraw(ent);
        }
        else if (ent->tpe == ET_KING) {
            KingDraw(ent);
        }
        else {
            EntityDraw(animations, ent);
        }

        // draw debug info
        if (debug) {
            EntityDrawDebug(ent);
        }
    }

    EntityDraw(animations, &mask);
    EndDrawing();

    // copy entitues to next frame
    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ent = entities.arr + i;

        if (ent->deleted == false) {
            Entity *added = entities_next.Add(*ent);

            // update the location of unique entities
            if (added->tpe == ET_SHIP) {
                ship = added;
            }
            else if (added->tpe == ET_KING) {
                king = added;
            }
        }
    }

    // swap entity buffers
    Array<Entity> swap = entities;
    entities = entities_next;
    entities_next = swap;
    entities_next.len = 0;
}

void FrameUpdate() {
    if (debug && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // DBG shoot using the mouse
        entities.Add( ShotCreate({ (f32) GetMouseX(), (f32) GetMouseY() }) );
    }
    if (debug && IsKeyPressed(KEY_R)) {
        // DBG reset
        for (s32 i = 0; i < entities.len; ++i) {
            Entity *ent = entities.arr + i;
            if (ent->tpe == ET_SHIP) {
                ent->state = ES_SHIP_IDLE;
                ent->vrot = 0;
                ent->rot = 0;
                ent->velocity = {};
                break;
            }
        }
    }

    if (IsKeyPressed(KEY_M)) {
        music_enabled = !music_enabled;

        if (music_enabled) {
            PlayMusicStream(*music_track);
        }
        else {
            StopMusicStream(*music_track);
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
    if (music_enabled) {
        UpdateMusicStream(*music_track);
    }


    // NOTE: ship_delta_vy is the ship speed relative to the "ambient" asteroids
    f32 ship_delta_vy = 0;
    ship_vy += ship_delta_vy;
    f32 dt = GetFrameTimeMS();

    // update
    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ent = entities.arr + i;

        if (ent->life > 0) {
            ent->life--;
            if (ent->life == 0) {
                ent->deleted = true;
            }
        }

        if (IsAsteroid(ent->tpe)) {
            if (ent->disable_vy == false) {
                ent->position.y += dt * ship_vy;
            }
            ent->Update(dt);

            // clip asteroids
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
        else if (ent->tpe == ET_SHIP && (game.GetState() == GS_GAME || game.GetState() == GS_END)) {
            ShipUpdate(ent, dt);
        }
        else if (ent->tpe == ET_KING) {
            KingUpdate(ent, dt);
        }

        else {
            ent->Update(dt);
        }
    }
}

void Run() {
    Init();

    while (!WindowShouldClose()) {
        if (game.GetState() == GS_END) {
            FrameUpdate();
        }
        else {
            FrameUpdate();
            FrameUpdatePhase();
        }

        FrameDrawAndSwap();
    }

    Close();
}

int main(void) {
    Run();
}
