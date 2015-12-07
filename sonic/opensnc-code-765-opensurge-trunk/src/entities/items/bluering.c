/*
 * bluering.c - blue ring
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

#include "bluering.h"
#include "../../core/util.h"
#include "../../core/soundfactory.h"
#include "../../scenes/level.h"

/* bluering 클래스 */
typedef struct bluering_t bluering_t;
struct bluering_t {
    item_t item; /* base class */
    int is_disappearing;
    /* 사라지는 모습 재생 변수 */
};

static void bluering_init(item_t *item);
static void bluering_release(item_t* item);
static void bluering_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void bluering_render(item_t* item, v2d_t camera_position);



/* bluering 아이템 생성 */
item_t* bluering_create()
{
    item_t *item = mallocx(sizeof(bluering_t));

    item->init = bluering_init;
    item->release = bluering_release;
    item->update = bluering_update;
    item->render = bluering_render;

    return item;
}


/* bluering 장착 */
void bluering_init(item_t *item)
{
    bluering_t *me = (bluering_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    me->is_disappearing = FALSE;
    actor_change_animation(item->actor, sprite_get_animation("SD_BLUERING", 0));
}

/* bluering 삭제 */
void bluering_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* bluering 아이템 능력 */
void bluering_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    player_t *player = level_player();
    bluering_t *me = (bluering_t*)item;
    actor_t *act = item->actor;

    act->visible = (player->got_glasses || level_editmode());

    if(!me->is_disappearing) {
        if(!player->dying && player->got_glasses && actor_collision(act, player->actor)) {
            /* 캐릭터가 bluering 획득 하였는지 확인*/
            actor_change_animation(act, sprite_get_animation("SD_BLUERING", 1));
            player_set_rings( player_get_rings() + 5 );
            sound_play( soundfactory_get("blue ring") );
            me->is_disappearing = TRUE;
        }
    }
    else {
        if(actor_animation_finished(act)) {
            /* bluering 아이템 발견, 사라지는 시간*/
            item->state = IS_DEAD;
        }
    }
}

/* bluering 획득시에 캐릭터 모습 */
void bluering_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
