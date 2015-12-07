/*
 * itemutil.c - items: internal utilities
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

#include <stdlib.h>
#include "itemutil.h"
#include "../../../core/v2d.h"
#include "../../../core/global.h"
#include "../../../entities/item.h"
#include "../../../entities/actor.h"

/* 가까운 거리의 아이템, 장애물 설정*/
item_t *find_closest_item(item_t *me, item_list_t *list, int desired_type, float *distance)
{
    float min_dist = INFINITY_FLT;
    item_list_t *it;
    item_t *ret = NULL;
    v2d_t v;

/* 캐릭터의 장애물과 아이템에 대한 크기 설정 */
    for(it=list; it; it=it->next) {
        if(it->data->type == desired_type) {
            v = v2d_subtract(it->data->actor->position, me->actor->position);
            if(v2d_magnitude(v) < min_dist) {
                ret = it->data;
                min_dist = v2d_magnitude(v);
            }
        }
    }

    if(distance)
        *distance = min_dist;

    return ret;
}
