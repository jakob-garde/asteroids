#ifndef __ENTITIES_H__
#define __ENTITIES_H__

#include "raylib.h"
#include "memory.h"


enum EntityType {
    ET_UNKNOWN,

    ET_AST_SMALL,
    ET_AST_MED,
    ET_AST_LARGE,
    ET_AST_BRUTE,

    ET_CNT
};

struct Frame {
    Rectangle source;
    s32 duration;
    Texture tex; // const

    Rectangle InvertedLeftRight() {
        Rectangle flipped = source;
        flipped.width = -flipped.width;
        flipped.x = source.x + source.width;
        return flipped;
    }

    Frame Mirror() {
        Frame flipped = *this;
        flipped.source.width = - flipped.source.width;
        return flipped;
    }
};

struct Animation {
    EntityType tpe;
    Texture texture;
    s32 frame_sz;
    Array<Frame> frames;
};

Animation InitAnimation(MArena *a_dest, const char* anifile, EntityType tpe) {
    Animation ani = {};

    ani.tpe = tpe;
    ani.texture = LoadTexture(anifile);
    assert(ani.texture.width % ani.texture.height == 0);
    ani.frame_sz = ani.texture.height;
    s32 ani_cnt = ani.texture.width / ani.texture.height;
    ani.frames = InitArray<Frame>(a_dest, ani_cnt);
    ani.frames.len = ani_cnt;

    for (s32 i = 0; i < ani.frames.len; ++i) {
        ani.frames.arr[i].source = { (f32) ani.texture.height * i, 0.0f, (f32) ani.texture.height, (f32) ani.texture.height };
        ani.frames.arr[i].duration = 100;
        ani.frames.arr[i].tex = ani.texture;
    }
    return ani;
}

struct Entity {
    EntityType tpe;
    s32 facing_right;
    s32 state;

    // kinematics
    Vector2 anchor;
    Vector2 velocity;
    f32 rot;
    f32 vrot;
    Rectangle coll_rect;
    Vector2 coll_offset;

    // animations
    Rectangle ani_rect;
    Vector2 ani_offset;
    s32 ani_idx;
    s32 ani_idx0;
    s32 ani_cnt;
    s32 frame_idx;
    f32 frame_elapsed;

    void Update(f32 dt) {
        anchor.x += dt * velocity.x;
        anchor.y += dt * velocity.y;

        ani_rect.x = anchor.x + ani_offset.x;
        ani_rect.y = anchor.y + ani_offset.y;

        coll_rect.x = anchor.x + coll_offset.x;
        coll_rect.y = anchor.y + coll_offset.y;
    }

    Frame GetFrame(Array<Animation> animations) {
        Animation ani = animations.arr[ani_idx + ani_idx0];
        Frame frame = ani.frames.arr[frame_idx];

        if (frame.duration == 0) {
            return frame;
        }

        if (frame_elapsed > frame.duration) {
            frame_elapsed = 0;
            frame_idx = (frame_idx + 1) % ani.frames.len;
            frame = ani.frames.arr[frame_idx];
        }

        if (facing_right) {
            return frame;
        }
        else {
            return frame.Mirror();
        }
    }
};

void UnloadTextures(Array<Animation> animations) {
    for (s32 i = 0; i < animations.len; ++i) {
        UnloadTexture(animations.arr[i].texture);
    }
}


#endif
