/*
 * Open Surge Engine
 * camera.c - camera routines
 * Copyright (C) 2010  Alexandre Martins <alemartf(at)gmail(dot)com>
 * http://opensnc.sourceforge.net
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

#include "camera.h"
#include "../core/util.h"
#include "../core/video.h"
#include "../core/timer.h"
#include "../scenes/level.h"

/* private stuff */
typedef struct camera_t camera_t;
struct camera_t {
    v2d_t position; /* 현재 position */
    v2d_t dest; /* 타켓 position : 부드럽게 하기 위해 사용한다. */
    float speed; /* 카메라는 px/s의 속도로 position에서 dest로 이동한다. */

    /* 카메라는 오직 region 배열의 boundary 안에서만 확인할 수 있다. */
    v2d_t region_topleft, region_bottomright; /* 사각형으로 표시한다. : 현재의 boundaries */
    v2d_t dest_region_topleft, dest_region_bottomright; /* target boundaries */
    float region_topleft_speed, region_bottomright_speed; /* speed, in px/s */

    /* misc */
    int is_locked; /* 카메라가 locked 되있거나 level에서 자유롭게 움직일 수 있는지 확인한다. */
};

static camera_t camera;
static void define_boundaries(int x1, int y1, int x2, int y2);
static void update_boundaries();



/* public methods */

/*
 * camera_init()
 * 카메라를 초기화하는 함수
 */
void camera_init()
{
    camera.is_locked = FALSE;

    camera.speed = 0.0f;
    camera.region_topleft_speed = 0.0f;
    camera.region_bottomright_speed = 0.0f;

    camera.position = camera.dest = v2d_new(0.0f, 0.0f);
    camera.region_topleft.x = camera.dest_region_topleft.x = VIDEO_SCREEN_W/2;
    camera.region_topleft.y = camera.dest_region_topleft.y = VIDEO_SCREEN_H/2;
    camera.region_bottomright.x = camera.dest_region_bottomright.x = level_size().x-VIDEO_SCREEN_W/2;
    camera.region_bottomright.y = camera.dest_region_bottomright.y = level_size().y-VIDEO_SCREEN_H/2;
}

/*
 * camera_update()
 * 카메라를 업데이트하는 함수
 */
void camera_update()
{
    const float threshold = 10;
    float dt = timer_get_delta();
    v2d_t ds;

    /* level사이즈는 마지막 프레임 동안 변경될 수 있다. */
    update_boundaries();

    /* 카메라 position을 업데이트한다. */
    ds = v2d_subtract(camera.dest, camera.position);
    if(v2d_magnitude(ds) > threshold) {
        ds = v2d_normalize(ds);
        camera.position.x += ds.x * camera.speed * dt;
        camera.position.y += ds.y * camera.speed * dt;
    }

    /* 가능한 지역을 업데이트한다. */
    ds = v2d_subtract(camera.dest_region_topleft, camera.region_topleft);
    if(v2d_magnitude(ds) > threshold) {
        ds = v2d_normalize(ds);
        camera.region_topleft.x += ds.x * camera.region_topleft_speed * dt;
        camera.region_topleft.y += ds.y * camera.region_topleft_speed * dt;
    }

    ds = v2d_subtract(camera.dest_region_bottomright, camera.region_bottomright);
    if(v2d_magnitude(ds) > threshold) {
        ds = v2d_normalize(ds);
        camera.region_bottomright.x += ds.x * camera.region_bottomright_speed * dt;
        camera.region_bottomright.y += ds.y * camera.region_bottomright_speed * dt;
    }

    /* clipping... */
    camera.position.x = clip(camera.position.x, camera.region_topleft.x, camera.region_bottomright.x);
    camera.position.y = clip(camera.position.y, camera.region_topleft.y, camera.region_bottomright.y);
}

/*
 * camera_release()
 * 카메라를 해제하는 함수
 */
void camera_release()
{
    ; /* empty */
}

/*
 * camera_move_to()
 * 단시간에 카메라를 새로운 position으로 이동시키는 함수
 */
void camera_move_to(v2d_t position, float seconds)
{
    /* clipping */
    if(position.x < camera.region_topleft.x)
        position.x = camera.region_topleft.x;

    if(position.y < camera.region_topleft.y)
        position.y = camera.region_topleft.y;

    if(position.x > camera.region_bottomright.x)
        position.x = camera.region_bottomright.x;

    if(position.y > camera.region_bottomright.y)
        position.y = camera.region_bottomright.y;

    /* 타켓 position을 업데이트한다. */
    camera.dest = position;

    /* 너무 빠르게 움직이지 않도록 한다. */
    if(seconds > EPSILON)
        camera.speed = v2d_magnitude( v2d_subtract(camera.position, camera.dest) ) / seconds;
    else
        camera.position = camera.dest;

}

/*
 * camera_lock()
 * 카메라를 lock하는 함수. 픽셀로된 지정된 사각형 안에서만 이동한다.
 */
void camera_lock(int x1, int y1, int x2, int y2)
{
    camera.is_locked = TRUE;
    define_boundaries(x1, y1, x2, y2);
}

/*
 * camera_unlock()
 * 카메라의 lock을 해제하는 함수, level안에서 자유롭게 움직일 수 있다.
 */
void camera_unlock()
{
    camera.is_locked = FALSE;
}

/*
 * camera_get_position()
 * 카메라의 position을 return해주는 함수.
 */
v2d_t camera_get_position()
{
    return v2d_new( (int)camera.position.x, (int)camera.position.y );
}

/*
 * camera_set_position()
 * 새로운 positon을 설정하는 함수
 */
void camera_set_position(v2d_t position)
{
    camera.dest = camera.position = position;
}

/*
 * camera_is_locked()
 * 카메라가 lock되어 있는지 확인하는 함수.
 */
int camera_is_locked()
{
    return camera.is_locked;
}

/* private methods */
void define_boundaries(int x1, int y1, int x2, int y2)
{
    float seconds = 0.25f;

    camera.dest_region_topleft.x = max(min(x1, x2), VIDEO_SCREEN_W/2);
    camera.dest_region_topleft.y = max(min(y1, y2), VIDEO_SCREEN_H/2);
    camera.dest_region_bottomright.x = min(max(x1, x2), level_size().x-VIDEO_SCREEN_W/2);
    camera.dest_region_bottomright.y = min(max(y1, y2), level_size().y-VIDEO_SCREEN_H/2);

    camera.region_topleft_speed = v2d_magnitude (
        v2d_subtract( camera.region_topleft, camera.dest_region_topleft )
    ) / seconds;

    camera.region_bottomright_speed = v2d_magnitude (
        v2d_subtract( camera.region_bottomright, camera.dest_region_bottomright )
    ) / seconds;
}

void update_boundaries()
{
    if(!camera.is_locked)
        define_boundaries(-INFINITY, -INFINITY, INFINITY, INFINITY);
}
