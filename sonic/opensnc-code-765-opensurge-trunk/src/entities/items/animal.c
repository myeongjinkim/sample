/*
 * animal.c - little animal
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

#include "animal.h"
#include "../../core/util.h"
#include "../../scenes/level.h" /* 헤더파일 3개를 호출 */

#define MAX_ANIMALS                 12

/* animal 클래스 */
typedef struct animal_t animal_t; /* 구조체 테그 이름 */
struct animal_t {
    item_t item; /* 기초 클래스 */
    int animal_id;
    int is_running;
};

static void animal_init(item_t *item);
static void animal_release(item_t* item);
static void animal_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void animal_render(item_t* item, v2d_t camera_position);
/* static 이 붙음으로서 모든 객체들이 공유 가능한 변수들 */


/* public methods */
item_t* animal_create()
{
    item_t *item = mallocx(sizeof(animal_t)); /* animal_t 즉 아이템 저장을 위한 동적할당 */

    item->init = animal_init;
    item->release = animal_release;
    item->update = animal_update;
    item->render = animal_render;

    return item;
}


/* private methods */
/* 캐릭터에 아이템 장착 */
void animal_init(item_t *item)
{
    animal_t *me = (animal_t*)item; /* 같을 경우 */

    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = FALSE;
    item->actor = actor_create();
    item->actor->maxspeed = 45 + random(21); /* 속도 증가 */
    item->actor->input = input_create_computer();

    me->is_running = FALSE;
    me->animal_id = random(MAX_ANIMALS);
    actor_change_animation(item->actor, sprite_get_animation("SD_ANIMAL", 0));
}


/* 캐릭터가 장착한 아이템 제거 */
void animal_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* 캐릭터 변화 */
void animal_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    animal_t *me = (animal_t*)item;
    actor_t *act = item->actor;
    brick_t *up, *down, *left, *right;
    float sqrsize = 2, diff = -2;
    int animation_id = 2*me->animal_id + (me->is_running?1:0);
    /* 객체 생성,  캐릭터의 움직임에 대한 변수 선언 */

    /* 캐릭터의 동작에 따른 움직임 변화 */
    input_simulate_button_down(act->input, IB_FIRE1);
    act->jump_strength = (200 + random(50)) * 1.3;

    if(act->speed.x > EPSILON) {
        act->speed.x = act->maxspeed;
        act->mirror = IF_NONE;
    }
    else if(act->speed.x < -EPSILON) {
        act->speed.x = -act->maxspeed;
        act->mirror = IF_HFLIP;
    }

    /* 캐릭터 동작 key에 따른 움직임 변화 */
    actor_change_animation(act, sprite_get_animation("SD_ANIMAL", animation_id));
    actor_corners(act, sqrsize, diff, brick_list, &up, NULL, &right, NULL, &down, NULL, &left, NULL);
    actor_handle_clouds(act, diff, &up, NULL, &right, NULL, &down, NULL, &left, NULL);

    if(down && !me->is_running) {
        me->is_running = TRUE;
        act->speed.x = (random(2)?-1:1) * act->maxspeed;
    }

    if(left && !up)
        act->speed.x = act->maxspeed;

    if(right && !up)
        act->speed.x = -act->maxspeed;

    if(!me->is_running && ((down && up) || (left && right)))
        item->state = IS_DEAD; /* i'm stuck! */

    actor_move(act, actor_platform_movement(act, brick_list, level_gravity()));
}

/* 캐릭터가 아이템 획득시 움직임과 카메라 위치 변화 만드는 함수 */
void animal_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
