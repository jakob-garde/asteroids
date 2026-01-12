#ifndef __ENTITIES_H__
#define __ENTITIES_H__


#include "memory.h"


enum EntityType {
    ET_UNDEF,

    ET_BACKGROUND,
    ET_BACKGROUND_MASK,

    ET_STAR,

    ET_AST_SMALL,
    ET_AST_MED,
    ET_AST_LARGE,
    ET_AST_BRUTAL,

    ET_EXPLOSION_SMALL,
    ET_EXPLOSION_MED,
    ET_EXPLOSION_LARGE,

    ET_SHOOT,
    ET_SHIP,
    ET_SHIP_CHASH,
    ET_KING,
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

void UnloadTextures(Array<Animation> animations) {
    for (s32 i = 0; i < animations.len; ++i) {
        UnloadTexture(animations.arr[i].texture);
    }
}

enum EntityState {
    ES_UNDEF,

    ES_SHOOT_CHARGE,
    ES_SHOOT_RELEASE,

    ES_SHIP_IDLE,
    ES_SHIP_LEFT,
    ES_SHIP_RIGHT,
    ES_SHIP_RESPAWN,

    ES_KING_PHASE_0, // peeking up
    ES_KING_PHASE_1, // one engine
    ES_KING_PHASE_2, // three engines
    ES_KING_PHASE_3, // four engines
    ES_KING_ADVANCE, // moving to ES_KING_PHASE_0
};

const char *EntityStateToText(EntityState state) {
    if (ES_UNDEF == state) return "ES_UNDEF";
    else if (ES_SHOOT_CHARGE == state) return "ES_SHOOT_CHARGE";
    else if (ES_SHOOT_RELEASE == state) return "ES_SHOOT_RELEASE";
    else if (ES_SHIP_IDLE == state) return "ES_SHIP_IDLE";
    else if (ES_SHIP_LEFT == state) return "ES_SHIP_LEFT";
    else if (ES_SHIP_RIGHT == state) return "ES_SHIP_RIGHT";
    else if (ES_SHIP_RESPAWN == state) return "ES_SHIP_RESPAWN";
    else if (ES_KING_PHASE_0 == state) return "ES_KING_PHASE_0";
    else if (ES_KING_PHASE_1 == state) return "ES_KING_PHASE_1";
    else if (ES_KING_PHASE_2 == state) return "ES_KING_PHASE_2";
    else if (ES_KING_PHASE_3 == state) return "ES_KING_PHASE_3";
    else if (ES_KING_ADVANCE == state) return "ES_KING_ADVANCE";
    else return "ERROR";
}

struct Entity {
    EntityType tpe;
    EntityState state;
    EntityState state_next;
    bool facing_left;
    bool deleted;
    bool disable_debug_draw;
    bool disable_vy;
    s32 life_frames;
    f32 duration;
    f32 elapsed;

    // kinematics
    Vector2 position;
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
        rot += dt * vrot;
        position.x += dt * velocity.x;
        position.y += dt * velocity.y;

        ani_rect.x = position.x;
        ani_rect.y = position.y;

        coll_rect.x = position.x + coll_offset.x;
        coll_rect.y = position.y + coll_offset.y;
    }
};

void EntityDrawDebug(Entity *ent) {
    if (ent->disable_debug_draw) {
        return;
    }

    Rectangle anirect = ent->ani_rect;
    anirect.x -= ent->ani_offset.x;
    anirect.y -= ent->ani_offset.y;
    DrawRectangleLinesEx(anirect, 4, BLUE);

    DrawRectangleLinesEx(ent->coll_rect, 2, RED);
    DrawCircleLinesV(ent->position, ent->coll_radius, BLACK);

    DrawCircleV(ent->position, 2, RED);
}

void EntityDraw(Array<Animation> animations, Entity *ent) {
    Animation ani = animations.arr[ent->ani_idx0 + ent->ani_idx];
    Frame frame = ani.frames.arr[ent->frame_idx];

    if (frame.duration == 0) {
    }

    else if (ent->frame_elapsed > frame.duration) {
        ent->frame_elapsed = 0;
        ent->frame_idx = (ent->frame_idx + 1) % ani.frames.len;
        frame = ani.frames.arr[ent->frame_idx];
    }

    if (ent->facing_left) {
        frame = frame.Mirror();
    }

    DrawTexturePro(frame.tex, frame.source, ent->ani_rect, ent->ani_offset, ent->rot, WHITE);
}

Entity *FindFirstEntityByType(EntityType tpe, Array<Entity> entities) {
    for (s32 i = 0; i < entities.len; ++i) {
        Entity *ent = entities.arr + i;
        if (ent->tpe == tpe) {
            return ent;
        }
    }
    return NULL;
}

Entity CreateEntity(EntityType tpe, Array<Animation> animations, bool select_random = true) {
    // TODO: remove "select_random" parameter

    // sets as many fields as possible on a generic Entity

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
            if (select_random && ani->frames.len == 1) {
                ent.ani_idx0 += GetRandomValue(0, ani->group_sz - 1);
            }

            ent.ani_cnt = ani->frames.len;
            ent.ani_offset = { ani->frame_w / 2.0f, ani->frame_h / 2.0f };
            ent.ani_rect = { 0, 0, 1.0f * ani->frame_w, 1.0f * ani->frame_h };

            ent.coll_offset = { -0.5f * ani->frame_w, -0.5f * ani->frame_h };
            ent.coll_rect = { 0, 0, 1.0f * ani->frame_w, 1.0f * ani->frame_h };

            ent.coll_radius = fmin( ani->frame_w / 2, ani->frame_h / 2 );
            ent.Update(0);

            return ent;
        }
    }

    return {};
}


enum SoundType {
    SE_EXPLOSION,
    SE_SHOOT,
    SE_CRASH,
};

struct SEffect {
    SoundType tpe;
    Sound sound;
};

SEffect InitSoundEffect(const char *filename, SoundType tpe) {
    SEffect sfx = {};
    sfx.tpe = tpe;
    sfx.sound = LoadSound(filename);

    return sfx;
}

void UnloadSounds(Array<SEffect> snds) {
    for (s32 i = 0; i < snds.len; ++i) {
        UnloadSound(snds.arr[i].sound);
    }
}

void PlaySoundEffect(SoundType tpe, Array<SEffect> sfxs) {
    for (s32 i = 0; i < sfxs.len; ++i) {
        SEffect sfx = sfxs.arr[i];
        if (sfx.tpe == tpe) {
            PlaySound(sfx.sound);
        }
    }
}


#endif
