/*
 * Open Surge Engine
 * v2d.c - 2D vectors
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

//2D vector stucture 가 있는 함수

#include <math.h>
#include "v2d.h"
#include "util.h"
#include "global.h"

/*
 * v2d_new()
 * Creates a new 2D vector
 */
v2d_t v2d_new(float x, float y)
//x,y 두개의 2D vertor 새로 생성
{
    v2d_t v = { x , y };
    return v;
}


/*
 * v2d_add()
 * Adds two vectors
 */
v2d_t v2d_add(v2d_t u, v2d_t v)
//두 백터 u,v 를 입력받아 더한 값 return
{
    v2d_t w = { u.x + v.x , u.y + v.y };
    return w;
}


/*
 * v2d_subtract()
 * Subtracts two vectors
 */
v2d_t v2d_subtract(v2d_t u, v2d_t v)
//두 벡터 u,v 를 입력받아 뺀 값 return
{
    v2d_t w = { u.x - v.x , u.y - v.y };
    return w;
}


/*
 * v2d_multiply()
 * Multiplies a vector by a scalar
 */
v2d_t v2d_multiply(v2d_t u, float h)
//두 벡터 u,v 를 입력받아 곱한 값 return
{
    v2d_t v = { h * u.x , h * u.y };
    return v;
}


/*
 * v2d_magnitude()
 * Returns the magnitude of a given vector
 */
float v2d_magnitude(v2d_t v)
//두 백터에 내적한 값에 루트 씌운값 리턴
{
    return sqrt( (v.x*v.x) + (v.y*v.y) );
}


/*
 * v2d_dotproduct()
 * Dot product: u.v
 */
float v2d_dotproduct(v2d_t u, v2d_t v)
//두 벡터 u,v 를 입력받아 내적한 값 return
{
    return (u.x*v.x + u.y*v.y);
}


/*
 * v2d_rotate()
 * Rotates a vector. Angle in radians.
 */
v2d_t v2d_rotate(v2d_t v, float ang)
//한 벡터를 angle(radians) 만큼 회전
{
    float x = v.x, y = v.y;
    v2d_t w;

    w.x = x*cos(ang) - y*sin(ang);
    w.y = y*cos(ang) + x*sin(ang);

    return w;
}


/*
 * v2d_normalize()
 * The same thing as v = v / |v|,
 * where |v| is the magnitude of v.
 */
v2d_t v2d_normalize(v2d_t v)
//벡터의 크기가 1인지 아닌지 비교해서 아니면 1로만드는(정규화) 함수
{
    float m = v2d_magnitude(v);
    v2d_t w = (m > EPSILON) ? v2d_new(v.x/m,v.y/m) : v2d_new(0,0);
    return w;
}



/*
 * v2d_lerp()
 * Performs a linear interpolation
 * between u and v.
 * 0.0 <= weight <= 1.0
 * The same as: (1-weight)*u + weight*v
 */
v2d_t v2d_lerp(v2d_t u, v2d_t v, float weight)
// 두 벡터의 선형 보간값(두 벡터 끝점 사이의 점을 표시한 값)을 return
{
    float w = clip(weight, 0.0, 1.0);
    float c = 1.0 - w;
    return v2d_new(u.x*c+v.x*w, u.y*c+v.y*w);
}
