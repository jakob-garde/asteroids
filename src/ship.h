#ifndef __SHIP_H__
#define __SHIP_H__

#include "raylib.h"

#include "memory.h"
#include "entities.h"

#include "globals.h"


Entity ShotCreate() {
    Entity ent = CreateEntity(ET_SHOOT, animations);
    ent.stt = ES_SHOOT_CHARGE;

    ent.anchor = { (f32) GetMouseX(), (f32) GetMouseY() };
    ent.ani_offset.y = ent.ani_rect.height;
    ent.velocity.y = -0.9;
    ent.Update(0);

    return ent;
}

void ShotUpdate(Entity *ent, f32 dt) {
    if (ent->frame_idx == 2) {
        ent->anchor.y += dt * ent->velocity.y;
    }
    else if (ent->frame_idx > 2) {
        ent->anchor.y += dt * ent->velocity.y;
    }
    ent->ani_rect.x = ent->anchor.x;
    ent->ani_rect.y = ent->anchor.y;
    ent->frame_elapsed += dt;

    if (ent->anchor.y < -200) {
        ent->deleted = true;
    }
}

void ShotDraw(Entity *ent) {
    Animation ani = animations.arr[ent->ani_idx0 + ent->ani_idx];
    Frame frame = ani.frames.arr[ent->frame_idx];

    if (ent->frame_elapsed > frame.duration) {
        if (ent->stt == ES_SHOOT_CHARGE) {
            if (ent->frame_idx == 2) {
                ent->stt = ES_SHOOT_RELEASE;
            }
            ent->frame_idx = ent->frame_idx + 1;
        }
        else if (ent->stt == ES_SHOOT_RELEASE) {
            ent->frame_idx = 3 + (ent->frame_idx + 1) % 5;
        }
        else {
            assert(1 == 0);
        }
        frame = ani.frames.arr[ent->frame_idx];
        ent->frame_elapsed = 0;
    }

    DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);
}

Entity ShiptCreate() {
    Entity ent = CreateEntity(ET_SHIP, animations, false);

    ent.stt = ES_SHIP_IDLE;
    ent.anchor.x = (mask_left + mask_right) / 2;
    ent.anchor.y = screen_h - 256;
    ent.Update(0);

    return ent;
}

void ShipUpdate(Entity *ent, f32 dt) {
    f32 speed = 0.4f;

    if (IsKeyPressed(KEY_TAB)) {
        ent->stt = ES_SHIP_CRASH;
        ent->facing_left = RandBin();
        ent->vrot = 0.05f * RandPM1();
        ent->velocity.x = RandPM1() * 0.1f;
        ent->velocity.y = RandPM1() * 0.1f;
    }

    if (ent->stt == ES_SHIP_CRASH) {
        ent->Update(dt);

        return;
    }

    ent->stt = ES_SHIP_IDLE;
    ent->facing_left = false;
    if (IsKeyDown(KEY_LEFT)) {
        ent->stt = ES_SHIP_LEFT;
        ent->facing_left = true;

        ent->anchor.x -= speed * dt;
    }
    else if (IsKeyDown(KEY_RIGHT)) {
        ent->stt = ES_SHIP_RIGHT;
        ent->facing_left = false;

        ent->anchor.x += speed * dt;
    }

    if (IsKeyDown(KEY_UP)) {
        ent->anchor.y -= speed * dt;
    }
    else if (IsKeyDown(KEY_DOWN)) {
        ent->anchor.y += speed * dt;
    }
    ent->Update(dt);

    if (IsKeyPressed(KEY_SPACE)) {
        Entity shot = ShotCreate();
        shot.anchor = ent->anchor;
        shot.anchor.y -= 4;
        shot.anchor.x += 32;
        shot.Update(0);

        entities.Add(shot);
    }
}

void ShipDraw(Entity *ent) {
    if (ent->stt == ES_SHIP_IDLE) {
        ent->ani_idx = 0;
    }
    else if (ent->stt == ES_SHIP_LEFT || ent->stt == ES_SHIP_RIGHT) {
        ent->ani_idx = 1;
    }
    else if (ent->stt == ES_SHIP_CRASH) {
        ent->ani_idx = 2;
    }

    Animation ani = animations.arr[ent->ani_idx0 + ent->ani_idx];
    Frame frame = ani.frames.arr[ent->frame_idx];

    if (ent->facing_left) {
        frame = frame.Mirror();
    }

    DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);
}


#endif
