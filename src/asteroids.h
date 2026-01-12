#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__


#include "memory.h"
#include "entities.h"
#include "globals.h"


Entity CreateAsteroid(EntityType tpe, f32 sigma) {
    Entity ast = CreateEntity(tpe, animations);

    f32 x = Rand(GetScreenWidth() - 1);
    f32 y = - 32;
    ast.position = { x, y };
    f32 vx = sigma * RandPM1();
    f32 vy = sigma * Rand01() / 2.0f;
    ast.velocity = { vx, vy };
    ast.vrot = GetRandomValue(-1, 1) / 3.0f;

    return ast;
}

bool DoSpawn(f32 dt, f32 vy, f32 rate) {
    f32 vy_base = 0.1f;
    bool result = Rand01() < rate * dt * (abs(vy) + vy_base) / 1000.0f;
    return result;
}

void SpawnAsteroids(Array<Entity> *entities, f32 dt, f32 rate_small, f32 rate_med, f32 sigma) {
    if (DoSpawn(dt, ship_vy, rate_small)) {
        entities->AddSafe( CreateAsteroid(ET_AST_SMALL, sigma) );
    }
    if (DoSpawn(dt, ship_vy, rate_med)) {
        entities->AddSafe( CreateAsteroid(ET_AST_MED, sigma) );
    }
}

bool IsAsteroid(EntityType tpe) {
    bool result = tpe == ET_AST_SMALL
        || tpe == ET_AST_MED
        || tpe == ET_AST_LARGE
        || tpe == ET_AST_BRUTAL;
    return result;
}


#endif
