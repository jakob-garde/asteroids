#ifndef __ENTITIES_H__
#define __ENTITIES_H__


#include "memory.h"


enum EntityType {
    ET_UNKNOWN,

    ET_AST_BACKGROUND,
    ET_AST_BACKGROUND_MASK,

    ET_AST_SMALL,
    ET_AST_MED,
    ET_AST_LARGE,
    ET_AST_BRUTAL,

    ET_SHOOT,

    ET_CNT
};

enum EntityAnimationState {
    ES_UNDEF,

    ES_SHOOT_CHARGE,
    ES_SHOOT_RELEASE,
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
    s32 group_sz;
    s32 group_idx;

    Texture texture;
    s32 frame_h;
    s32 frame_w;
    Array<Frame> frames;
};

Animation InitAnimation(MArena *a_dest, const char* anifile, EntityType tpe, s32 group_idx, s32 group_sz, s32 frame_cnt_if_nonsquare = 0) {
    // animations are:
    //      - single-row
    //      - square unless frame_w_if_nonsquare != 0

    Animation ani = {};
    ani.tpe = tpe;
    ani.group_sz = group_sz;
    ani.group_idx = group_idx;

    ani.texture = LoadTexture(anifile);
    ani.frame_h = ani.texture.height;
    ani.frame_w = ani.frame_h;
    if (frame_cnt_if_nonsquare == 0) {
        assert(ani.texture.width % ani.texture.height == 0);
    }
    else {
        assert(ani.texture.width % frame_cnt_if_nonsquare == 0);
        ani.frame_w = ani.texture.width / frame_cnt_if_nonsquare;
    }

    s32 frame_cnt = ani.texture.width / ani.frame_w;
    ani.frames = InitArray<Frame>(a_dest, frame_cnt);
    ani.frames.len = frame_cnt;

    for (s32 i = 0; i < ani.frames.len; ++i) {
        ani.frames.arr[i].source = { (f32) ani.frame_w * i, 0.0f, (f32) ani.frame_w, (f32) ani.frame_h };
        ani.frames.arr[i].duration = 100;
        ani.frames.arr[i].tex = ani.texture;
    }
    return ani;
}

struct Entity {
    EntityType tpe;
    EntityAnimationState stt;
    s32 facing_left;
    s32 state;
    s32 deleted;

    // kinematics
    Vector2 anchor;
    Vector2 velocity;
    f32 rot;
    f32 vrot;
    Rectangle coll_rect;
    Vector2 coll_offset;
    f32 coll_radius;

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
        Animation ani = animations.arr[ani_idx0 + ani_idx];
        Frame frame = ani.frames.arr[frame_idx];

        if (frame.duration == 0) {
            return frame;
        }

        if (frame_elapsed > frame.duration) {
            frame_elapsed = 0;
            frame_idx = (frame_idx + 1) % ani.frames.len;
            frame = ani.frames.arr[frame_idx];
        }

        if (facing_left) {
            return frame.Mirror();
        }
        else {
            return frame;
        }
    }
};

void UnloadTextures(Array<Animation> animations) {
    for (s32 i = 0; i < animations.len; ++i) {
        UnloadTexture(animations.arr[i].texture);
    }
}

Entity CreateEntity(EntityType tpe, Array<Animation> animations) {
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


#endif
