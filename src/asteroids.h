#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__


#include "memory.h"
#include "entities.h"
#include "globals.h"


Entity CreateAsteroid(EntityType tpe, bool on_screen = false) {
    Entity ast = CreateEntity(tpe, animations);

    if (on_screen) {
        f32 x = GetRandomValue(0, GetScreenWidth() - 1);
        f32 y = GetRandomValue(0, GetScreenHeight() - 1);
        ast.position = { x, y };
    }
    else {
        f32 x = GetRandomValue(0, GetScreenWidth() - 1);
        f32 y = GetRandomValue(-512, -64);
        ast.position = { x, y };
    }

    f32 vx = GetRandomValue(-100, 100) / 700.0f;
    f32 vy = GetRandomValue(-100, 100) / 700.0f;
    ast.velocity = { vx, vy };

    ast.rot = GetRandomValue(-100, 100);
    ast.vrot = GetRandomValue(-100, 100) / 300.0f;

    return ast;
}

bool DoSpawn(f32 dt, f32 vy, f32 rate) {
    f32 vy_base = 0.1f;
    bool result = Rand01() < rate * dt * (abs(vy) + vy_base) / 1000.0f;
    return result;
}

void SpawnAsteroids(Array<Entity> *entities, f32 dt) {
    f32 rate_small = 16; // asteroids per second
    f32 rate_med = 2;

    if (DoSpawn(dt, ship_vy, rate_small)) {
        entities->AddSafe( CreateAsteroid(ET_AST_SMALL) );
    }
    if (DoSpawn(dt, ship_vy, rate_med)) {
        entities->AddSafe( CreateAsteroid(ET_AST_MED) );
    }
}

void SpawnStartupAsteroids(Array<Entity> *entities, f32 vy) {
    f32 rate_small = 16;
    f32 rate_med = 2;

    for (s32 i = 0; i < rate_small; ++i) 
        entities->AddSafe( CreateAsteroid(ET_AST_SMALL, true) );
    for (s32 i = 0; i < rate_med; ++i)
        entities->AddSafe( CreateAsteroid(ET_AST_MED, true) );
}

bool IsAsteroid(EntityType tpe) {
    bool result = tpe == ET_AST_SMALL
        || tpe == ET_AST_MED
        || tpe == ET_AST_LARGE
        || tpe == ET_AST_BRUTAL;
    return result;
}

#endif
