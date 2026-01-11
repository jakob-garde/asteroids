#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__


#include "memory.h"
#include "entities.h"
#include "globals.h"


Entity CreateAsteroid(EntityType tpe, f32 sigma) {
    Entity ast = CreateEntity(tpe, animations);

    //f32 x = GetRandomValue(0, GetScreenWidth());
    //f32 y = GetRandomValue(0, GetScreenHeight() / 3);
    //ast.position = { x, y };
    //f32 vx = GetRandomValue(-sigma, sigma) / 1400.0f;
    //f32 vy = GetRandomValue(-sigma, sigma) / 1400.0f;
    //ast.velocity = { vx, vy };
    //ast.disable_vy = true;

    f32 x = GetRandomValue(0, GetScreenWidth() - 1);
    f32 y = -32;
    ast.position = { x, y };
    f32 vx = GetRandomValue(-sigma, sigma) / 700.0f;
    f32 vy = GetRandomValue(-sigma, sigma) / 700.0f;
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

void SpawnAsteroids(Array<Entity> *entities, f32 dt, f32 rate_small, f32 rate_med) {
    if (DoSpawn(dt, ship_vy, rate_small)) {
        entities->AddSafe( CreateAsteroid(ET_AST_SMALL, 100.0f) );
    }
    if (DoSpawn(dt, ship_vy, rate_med)) {
        entities->AddSafe( CreateAsteroid(ET_AST_MED, 100.0f) );
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
