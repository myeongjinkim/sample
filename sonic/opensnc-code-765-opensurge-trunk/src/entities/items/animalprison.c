/*
 * animalprison.c - animal prison (this object appears after the boss fight)
 * Copyright (C) 2010  Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "animalprison.h"
#include "../../core/util.h"
#include "../../core/audio.h"
#include "../../core/timer.h"
#include "../../core/soundfactory.h"
#include "../../scenes/level.h"

/* 캐릭터의 기본적인 상태 */
typedef struct state_t state_t; /* abstract class */
struct state_t {
    void (*handle)(state_t*,item_t*,player_t**,int); /* receives: state, animalprison, team, team_size */
};

/* Idle은 캐릭터가 공격할 때 까지 기다리는 상태 */
typedef struct state_idle_t state_idle_t; /* concrete state: idle */
struct state_idle_t {
    state_t state; /* 기본적인 상태 클래스 */
    int being_hit; /* 공격 입력을 받는 변수 */
    int hit_count; /* 공격을 몇번 했는지 입력받는 변수 */
};

static state_t* state_idle_new(); /* state constructor */
static void state_idle_handle(state_t*,item_t*,player_t**,int); /* private method */

/* 캐릭터가 감옥에 갇힌 동안 공격을 몇번 시도후 감옥이 풀리는 상태 */
typedef struct state_exploding_t state_exploding_t; /* concrete state: exploding */
struct state_exploding_t {
    state_t state;
    float explode_timer; /* 감옥이 풀리는 시도 횟수 */
    float break_timer; /* 얼마동안 깰려고 공격하였는지를 나타내는 횟수 */
};

static state_t* state_exploding_new(); /* state constructor */
static void state_exploding_handle(state_t*,item_t*,player_t**,int); /* private method */

/* 감옥이 풀린후에 캐릭터가 자유롭게 행동할 수 있도록 하는 상태  */
typedef struct state_releasing_t state_releasing_t; /* concrete state: releasing */
struct state_releasing_t {
    state_t state;
};

static state_t* state_releasing_new(); /* state constructor */
static void state_releasing_handle(state_t*,item_t*,player_t**,int); /* private method */

/* 마침내 감옥 상태에서 풀려날 때 */
typedef struct state_broken_t state_broken_t; /* concrete state: broken */
struct state_broken_t {
    state_t state;
};

static state_t* state_broken_new(); /* state constructor */
static void state_broken_handle(state_t*,item_t*,player_t**,int); /* private method */



/* 캐릭터가 갇힌 상태에서의 클래스 */
typedef struct animalprison_t animalprison_t;
struct animalprison_t {
    item_t item; /* base class */
    state_t *state; /* state pattern */
};

static void animalprison_init(item_t *item);
static void animalprison_release(item_t* item);
static void animalprison_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void animalprison_render(item_t* item, v2d_t camera_position);

static void animalprison_set_state(item_t *item, state_t *state);
static int animalprison_got_hit_by_player(item_t *item, player_t *player);



/* 캐릭터 객체(감옥)에 갇힌 상태 생성 */
item_t* animalprison_create()
{
    item_t *item = mallocx(sizeof(animalprison_t));
    animalprison_t *me = (animalprison_t*)item;

    item->init = animalprison_init;
    item->release = animalprison_release;
    item->update = animalprison_update;
    item->render = animalprison_render;

    me->state = NULL;

    return item;
}



/* 캐릭터가 갇힌 상태 설치 */
void animalprison_set_state(item_t *item, state_t *state)
{
    animalprison_t *me = (animalprison_t*)item;

    if(me->state != NULL)
        free(me->state);

    me->state = state;
}

/* 캐릭터가 객체(감옥)에 갇힌 상태 초기화 하는 함수 */
void animalprison_init(item_t *item)
{
    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    animalprison_set_state(item, state_idle_new());
    actor_change_animation(item->actor, sprite_get_animation("SD_ENDLEVEL", 0));
}

/* 캐릭터가 감옥에서 풀려난 상태 함수 */
void animalprison_release(item_t* item)
{
    actor_destroy(item->actor);
    animalprison_set_state(item, NULL);
}
/*캐릭터가 갇힌 상태에서의 조작과 카메라 위치 */
void animalprison_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    animalprison_t *me = (animalprison_t*)item;
    me->state->handle(me->state, item, team, team_size);
}

void animalprison_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}

/* 감옥에서 풀려난 상태 */
state_t* state_idle_new()
{
    state_t *base = mallocx(sizeof(state_idle_t));
    state_idle_t *derived = (state_idle_t*)base;

    base->handle = state_idle_handle;
    derived->being_hit = FALSE;
    derived->hit_count = 0;

    return base;
}
/* 감옥에서 풀려난 상태에서 상태 재생성 */
state_t* state_exploding_new()
{
    state_t *base = mallocx(sizeof(state_exploding_t));
    state_exploding_t *derived = (state_exploding_t*)base;

    base->handle = state_exploding_handle;
    derived->explode_timer = 0.0f;
    derived->break_timer = 0.0f;

    return base;
}
/* 풀려난 상태에서의 조작 메모리 동적 할당 */
state_t* state_releasing_new()
{
    state_t *base = mallocx(sizeof(state_releasing_t));
    base->handle = state_releasing_handle;
    return base;
}
/* 풀려난 상태에서의 조작 메모리 동적 할당 */
state_t* state_broken_new()
{
    state_t *base = mallocx(sizeof(state_broken_t));
    base->handle = state_broken_handle;
    return base;
}

/* 상태 구현 */
void state_idle_handle(state_t *state, item_t *item, player_t **team, int team_size)
{
    int i;
    state_idle_t *s = (state_idle_t*)state;
    actor_t *act = item->actor;

    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(animalprison_got_hit_by_player(item, player) && !s->being_hit) {
            /* 캐릭터가 이 개체를 공격 할때 */
            s->being_hit = TRUE;
            actor_change_animation(act, sprite_get_animation("SD_ENDLEVEL", 1));
            sound_play( soundfactory_get("boss hit") );
            player_bounce(player);
            player->actor->speed.x *= -0.5;

            if(++(s->hit_count) >= 3) /* 3번 공격하면 감옥에서 풀려나는 상태 */
                animalprison_set_state(item, state_exploding_new());
        }
    }

    /* 타격을 받은 후 복원 */
    if(actor_animation_finished(act) && s->being_hit) {
        actor_change_animation(act, sprite_get_animation("SD_ENDLEVEL", 0));
        s->being_hit = FALSE;
    }
}

void state_exploding_handle(state_t *state, item_t *item, player_t **team, int team_size)
{
    state_exploding_t *s = (state_exploding_t*)state;
    actor_t *act = item->actor;
    float dt = timer_get_delta();

    s->explode_timer += dt;
    s->break_timer += dt;

    /* 감옥에서 풀려난 상태를 유지 (잠시 동안) */
    if(s->explode_timer >= 0.1f) {
        v2d_t pos = v2d_new(
            act->position.x - act->hot_spot.x + random(actor_image(act)->w),
            act->position.y - act->hot_spot.y + random(actor_image(act)->h/2)
        );
        level_create_item(IT_EXPLOSION, pos);
        sound_play( soundfactory_get("explode") );

        s->explode_timer = 0.0f;
    }

    /* 더이상 감옥에서 풀려날 수 없는 상태 */
    if(s->break_timer >= 2.0f)
        animalprison_set_state(item, state_releasing_new());
}

void state_releasing_handle(state_t *state, item_t *item, player_t **team, int team_size)
{
    actor_t *act = item->actor;
    int i;

    /* 캐릭터 감옥 해체 */
    for(i=0; i<20; i++) {
        v2d_t pos = v2d_new(
            act->position.x - act->hot_spot.x + random(actor_image(act)->w),
            act->position.y - act->hot_spot.y + random(actor_image(act)->h/2)
        );
        level_create_animal(pos);
    }

    /* 이번 단계를 통과한 상태 */
    level_clear(act);

    /* sayonara bye bye */
    actor_change_animation(act, sprite_get_animation("SD_ENDLEVEL", 2));
    animalprison_set_state(item, state_broken_new());
}

void state_broken_handle(state_t *state, item_t *item, player_t **team, int team_size)
{
    ;
}

/* misc */
/* 캐릭터가 감옥에 들어 갔을 경우 상태 값을 반환 */
int animalprison_got_hit_by_player(item_t *item, player_t *player)
{
    float a[4], b[4];
    actor_t *act = item->actor;
    actor_t *pl = player->actor;

    a[0] = pl->position.x - pl->hot_spot.x;
    a[1] = pl->position.y - pl->hot_spot.y;
    a[2] = a[0] + actor_image(pl)->w;
    a[3] = a[1] + actor_image(pl)->h;

    b[0] = act->position.x - act->hot_spot.x + 5;
    b[1] = act->position.y - act->hot_spot.y;
    b[2] = b[0] + actor_image(act)->w - 10;
    b[3] = b[1] + actor_image(act)->h/2;

    return (player_attacking(player) && bounding_box(a,b) && actor_pixelperfect_collision(act,pl));
}
