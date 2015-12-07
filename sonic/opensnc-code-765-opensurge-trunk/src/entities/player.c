/*
 * Open Surge Engine
 * player.c - player module
 * Copyright (C) 2008-2012  Alexandre Martins <alemartf(at)gmail(dot)com>
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
 *
 * Edits by Dalton Sterritt (all edits released under same license):
 * player_enable_roll, player_disable_roll
 */

#include <math.h>
#include "actor.h"
#include "player.h"
#include "brick.h"
#include "collisionmask.h"
#include "enemy.h"
#include "item.h"
#include "character.h"
#include "items/collectible.h"
#include "items/bouncingcollectible.h"
#include "../core/global.h"
#include "../core/audio.h"
#include "../core/util.h"
#include "../core/stringutil.h"
#include "../core/timer.h"
#include "../core/logfile.h"
#include "../core/input.h"
#include "../core/sprite.h"
#include "../core/soundfactory.h"
#include "../scenes/level.h"
#include "physics/physicsactor.h"
#include "physics/obstaclemap.h"
#include "physics/obstacle.h"


/* Uncomment to render the sensors */
/*#define SHOW_SENSORS*/


/* 매크로 */
#define ON_STATE(s) \
    if(p->pa_old_state != (s) && physicsactor_get_state(p->pa) == (s))

#define CHANGE_ANIM(id) { \
    animation_t *an = sprite_get_animation(charactersystem_get(p->name)->animation.sprite_name, charactersystem_get(p->name)->animation.id); \
    /*float xsp = fabs(p->actor->speed.x / 60.0f), sf = 1.0f;*/ \
    actor_change_animation(p->actor, an); \
    /*if( strcmp( #id, "walking" ) == 0 ) sf = 0.5f * 60.0f / ( 8 * max(1, 8 - xsp) );*/ \
    /*else if( strcmp( #id, "running" ) == 0 ) sf = 0.5f * 60.0f / ( 8 * max(1, 8 - xsp) );*/ \
    /*else if( strcmp( #id, "jumping" ) == 0 ) sf = 0.5f * 60.0f / ( 8 * max(1, 5 - xsp) );*/ \
    /*else if( strcmp( #id, "rolling" ) == 0 ) sf = 0.5f * 60.0f / ( 8 * max(1, 5 - xsp) );*/  \
    /* sf = 60.0f / ( an->fps * max(1, 8 - xsp) ); */ \
    /* actor_change_animation_speed_factor(p->actor, sf); */ \
}

/* private data */
#define PLAYER_MAX_BLINK            2.0  /* 그/그녀가 다치는 경우 player는 몇 초동안 깜빡거려야(blink) 하는가? */
#define PLAYER_UNDERWATER_BREATH    30.0 /* player는 익사하기전에 수중에서 몇 초동안 머물수 있는가? */
static int collectibles, hundred_collectibles;         /* collectibles 공유 */
static int lives;                        /* lives(생명) 공유 */
static int score;                        /* score 공유 */


static void update_shield(player_t *p);
static void update_animation(player_t *p);
static void physics_adapter(player_t *player, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static obstacle_t* brick2obstacle(const brick_t *brick);
static obstacle_t* item2obstacle(const item_t *item);
static obstacle_t* object2obstacle(const object_t *object);
static int ignore_obstacle(int brick_angle, int old_loop_system_flags, bricklayer_t brick_layer, bricklayer_t player_layer);


/*
 * player_create()
 * 플레이어를 생성하는 함수
 */
player_t *player_create(const char *character_name)
{
    int i;
    player_t *p = mallocx(sizeof *p);
    const character_t *c = charactersystem_get(character_name);

    logfile_message("player_create(\"%s\")", character_name);

    /* 초기화 */
    p->name = str_dup(character_name);
    p->actor = actor_create();
    p->disable_movement = FALSE;
    p->disable_roll = FALSE;
    p->in_locked_area = FALSE;
    p->at_some_border = FALSE;
    p->bring_to_back = FALSE;
    p->disable_collectible_loss = FALSE;
    p->disable_animation_control = FALSE;
    p->attacking = FALSE;
    p->actor->input = input_create_user(NULL);
    CHANGE_ANIM(stopped);

    /* 보조 변수 */
    p->on_moveable_platform = FALSE;
    p->got_glasses = FALSE;

    /* blink */
    p->blinking = FALSE;
    p->blink_timer = 0.0f;
    p->blink_visibility_timer = 0.0f;

    /* 방패 */
    p->shield = actor_create();
    p->shield_type = SH_NONE;

    /* 무적 */
    p->invincible = FALSE;
    p->invtimer = 0;
    for(i=0; i<PLAYER_MAX_INVSTAR; i++) {
        p->invstar[i] = actor_create();
        actor_change_animation(p->invstar[i], sprite_get_animation("SD_INVSTAR", 0));
    }

    /* speed shoes */
    p->got_speedshoes = FALSE;
    p->speedshoes_timer = 0;

    /* old loop system */
    p->disable_wall = PLAYER_WALL_NONE;
    p->entering_loop = FALSE;
    p->at_loopfloortop = FALSE;

    /* loop system */
    p->layer = BRL_DEFAULT;

    /* physics */
    p->pa = physicsactor_create(p->actor->position);
    p->pa_old_state = physicsactor_get_state(p->pa);

    /* 기타 */
    p->underwater = FALSE;
    p->underwater_timer = 0.0f;

    /* 캐릭터 system */
    if(str_icmp(c->companion_object_name, "") != 0) {
        object_t *companion = level_create_enemy(c->companion_object_name, v2d_new(0, 0));
        companion->created_from_editor = FALSE;
    }

    physicsactor_set_acc(p->pa, physicsactor_get_acc(p->pa) * c->multiplier.acc);
    physicsactor_set_dec(p->pa, physicsactor_get_dec(p->pa) * c->multiplier.dec);
    physicsactor_set_topspeed(p->pa, physicsactor_get_topspeed(p->pa) * c->multiplier.topspeed);
    physicsactor_set_jmp(p->pa, physicsactor_get_jmp(p->pa) * c->multiplier.jmp);
    physicsactor_set_jmprel(p->pa, physicsactor_get_jmprel(p->pa) * c->multiplier.jmprel);
    physicsactor_set_grv(p->pa, physicsactor_get_grv(p->pa) * c->multiplier.grv);
    physicsactor_set_rollthreshold(p->pa, physicsactor_get_rollthreshold(p->pa) * c->multiplier.rollthreshold);
    physicsactor_set_brakingthreshold(p->pa, physicsactor_get_brakingthreshold(p->pa) * c->multiplier.brakingthreshold);
    physicsactor_set_slp(p->pa, physicsactor_get_slp(p->pa) * c->multiplier.slp);
    physicsactor_set_rolluphillslp(p->pa, physicsactor_get_rolluphillslp(p->pa) * c->multiplier.rolluphillslp);
    physicsactor_set_rolldownhillslp(p->pa, physicsactor_get_rolldownhillslp(p->pa) * c->multiplier.rolldownhillslp);

    /* 성공 */
    hundred_collectibles = collectibles = 0;
    logfile_message("player_create() ok");
    return p;
}


/*
 * player_destroy()
 * player를 삭제하는 함수
 */
player_t* player_destroy(player_t *player)
{
    int i;

    for(i=0; i<PLAYER_MAX_INVSTAR; i++)
        actor_destroy(player->invstar[i]);

    physicsactor_destroy(player->pa);
    actor_destroy(player->shield);
    actor_destroy(player->actor);
    free(player->name);
    free(player);

    return NULL;
}



/*
 * player_update()
 * player를 업데이트하는 함수.
 */
void player_update(player_t *player, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, enemy_list_t *enemy_list)
{
    int i;
    actor_t *act = player->actor;
    float dt = timer_get_delta();

    act->hot_spot = v2d_new(image_width(actor_image(act))/2, image_height(actor_image(act))-20);

    /* physics */
    if(!player->disable_movement) {
        player->pa_old_state = physicsactor_get_state(player->pa);
        physics_adapter(player, team, team_size, brick_list, item_list, enemy_list);
    }

    /* player 깜빡거림 */
    if(player->blinking) {
        player->blink_timer += timer_get_delta();

        if(player->blink_timer - player->blink_visibility_timer >= 0.067f) {
            player->blink_visibility_timer = player->blink_timer;
            act->visible = !act->visible;
        }

        if(player->blink_timer >= PLAYER_MAX_BLINK) {
            player->blinking = FALSE;
            act->visible = TRUE;
        }
    }

    if(physicsactor_get_state(player->pa) != PAS_GETTINGHIT && player->pa_old_state == PAS_GETTINGHIT) {
        player->blinking = TRUE;
        player->blink_timer = 0.0f;
        player->blink_visibility_timer = 0.0f;
    }

    /* 방패 */
    if(player->shield_type != SH_NONE)
        update_shield(player);

    /* 수중에서 */
    if(!(player->underwater) && player->actor->position.y >= level_waterlevel())
        player_enter_water(player);
    else if(player->underwater && player->actor->position.y < level_waterlevel())
        player_leave_water(player);

    /* 수중인지 확인 */
    if(player->underwater) {
        player->speedshoes_timer = max(player->speedshoes_timer, PLAYER_MAX_SPEEDSHOES); /* disable speed shoes */

        if(player->shield_type != SH_WATERSHIELD)
            player->underwater_timer += dt;
        else
            player->underwater_timer = 0.0f;

        if(player_seconds_remaining_to_drown(player) <= 0.0f)
            player_drown(player);
    }

    /* 무적의 별 */
    if(player->invincible) {
        int maxf = sprite_get_animation("SD_INVSTAR", 0)->frame_count;
        int invangle[PLAYER_MAX_INVSTAR];
        v2d_t starpos;

        player->invtimer += dt;

        for(i=0; i<PLAYER_MAX_INVSTAR; i++) {
            invangle[i] = (180*4) * timer_get_ticks()*0.001 + (i+1)*(360/PLAYER_MAX_INVSTAR);
            starpos.x = 25*cos(invangle[i]*PI/180);
            starpos.y = ((timer_get_ticks()+i*400)%2000)/40;
            /*starpos = v2d_rotate(starpos, act->angle);*/
            player->invstar[i]->position.x = act->position.x - act->hot_spot.x + image_width(actor_image(act))/2 + starpos.x;
            player->invstar[i]->position.y = act->position.y - act->hot_spot.y + image_height(actor_image(act)) - starpos.y + 5;
            actor_change_animation_frame(player->invstar[i], random(maxf));
        }

        if(player->invtimer >= PLAYER_MAX_INVINCIBILITY)
            player->invincible = FALSE;
    }

    /* speed shoes */
    if(player->got_speedshoes) {
        physicsactor_t *pa = player->pa;

        if(player->speedshoes_timer == 0) {
            physicsactor_set_acc(pa, physicsactor_get_acc(pa) * 2.0f);
            physicsactor_set_frc(pa, physicsactor_get_frc(pa) * 2.0f);
            physicsactor_set_topspeed(pa, physicsactor_get_topspeed(pa) * 2.0f);
            physicsactor_set_air(pa, physicsactor_get_air(pa) * 2.0f);
            physicsactor_set_rollfrc(pa, physicsactor_get_rollfrc(pa) * 2.0f);
            player->speedshoes_timer += dt;
        }
        else if(player->speedshoes_timer >= PLAYER_MAX_SPEEDSHOES) {
            physicsactor_set_acc(pa, physicsactor_get_acc(pa) / 2.0f);
            physicsactor_set_frc(pa, physicsactor_get_frc(pa) / 2.0f);
            physicsactor_set_topspeed(pa, physicsactor_get_topspeed(pa) / 2.0f);
            physicsactor_set_air(pa, physicsactor_get_air(pa) / 2.0f);
            physicsactor_set_rollfrc(pa, physicsactor_get_rollfrc(pa) / 2.0f);
            player->got_speedshoes = FALSE;
        }
        else
            player->speedshoes_timer += dt;
    }

    /* 애니메이션 */
    update_animation(player);

    /* CPU가 제어하는 player인지 확인 */
    if(player != level_player()) {
        for(i=0; i<IB_MAX; i++)
            input_simulate_button_up(act->input, (inputbutton_t)i);
    }

    /* 승리 포즈 */
    if(level_has_been_cleared())
        physicsactor_enable_winning_pose(player->pa);
}


/*
 * player_render()
 * player를 생성하는 함수.
 */
void player_render(player_t *player, v2d_t camera_position)
{
    actor_t *act = player->actor;
    int i, behind_player[PLAYER_MAX_INVSTAR];
    float ang;
    for(i=0;i<PLAYER_MAX_INVSTAR && player->invincible;i++)
        behind_player[i] = (int)((180*4) * timer_get_ticks()*0.001 + (i+1)*(360/PLAYER_MAX_INVSTAR)) % 360 >= 180;

    for(i=0;i<PLAYER_MAX_INVSTAR && player->invincible;i++) {
        if(behind_player[i])
            actor_render(player->invstar[i], camera_position);
    }

    /* player를 생성 */
    switch(physicsactor_get_movmode(player->pa)) {
    case MM_FLOOR: act->position.y -= 1; break;
    case MM_LEFTWALL: act->position.x += 3; break;
    case MM_RIGHTWALL: act->position.x -= 1; break;
    case MM_CEILING: act->position.y += 3; break;
    }

    act->angle = old_school_angle(ang = act->angle);
    actor_render(act, camera_position);
    act->angle = ang;

    switch(physicsactor_get_movmode(player->pa)) {
    case MM_FLOOR: act->position.y += 1; break;
    case MM_LEFTWALL: act->position.x -= 3; break;
    case MM_RIGHTWALL: act->position.x += 1; break;
    case MM_CEILING: act->position.y -= 3; break;
    }

    /* 방패를 생성 */
    if(player->shield_type != SH_NONE)
        actor_render(player->shield, camera_position);

    /* 무적의 별 II */
    for(i=0;i<PLAYER_MAX_INVSTAR && player->invincible;i++) {
        if(!behind_player[i])
            actor_render(player->invstar[i], camera_position);
    }

#ifdef SHOW_SENSORS
    /* 센서 */
    physicsactor_render_sensors(player->pa, camera_position);
#endif
}



/*
 * player_bounce()
 * player의 반동을 제어하는 함수
 */
void player_bounce(player_t *player, actor_t *hazard)
{
    int w = image_width(actor_image(hazard))/2, h = image_height(actor_image(hazard))/2;
    v2d_t hazard_centre = v2d_add(v2d_subtract(hazard->position, hazard->hot_spot), v2d_new(w/2, h/2));
    actor_t *act = player->actor;

    if(physicsactor_is_in_the_air(player->pa)) {
        if(act->position.y <= hazard_centre.y && act->speed.y > 0.0f) {
            if(input_button_down(act->input, IB_FIRE1) || physicsactor_get_state(player->pa) == PAS_ROLLING)
                act->speed.y *= -1.0f;
            else
                act->speed.y = 4 * max(-60.0f, -60.0f * act->speed.y);

            player->pa_old_state = physicsactor_get_state(player->pa);
            physicsactor_bounce(player->pa);
        }
        else
            act->speed.y -= 4 * 60.0f * sign(act->speed.y);
    }
}



/*
 * player_hit()
 * player가 공격한다. collectibles가 없다면 죽는다.
 */
void player_hit(player_t *player, actor_t *hazard)
{
    int w = image_width(actor_image(hazard))/2, h = image_height(actor_image(hazard))/2;
    v2d_t hazard_centre = v2d_add(v2d_subtract(hazard->position, hazard->hot_spot), v2d_new(w/2, h/2));

    if(player->invincible || physicsactor_get_state(player->pa) == PAS_GETTINGHIT || player->blinking || player_is_dying(player))
        return;

    if(player_get_collectibles() > 0 || player->shield_type != SH_NONE) {
        player->actor->speed.x = 120.0f * sign(player->actor->position.x - hazard_centre.x);
        player->actor->speed.y = -240.0f;
        player->actor->position.y -= 2; /* bugfix */

        player->pa_old_state = physicsactor_get_state(player->pa);
        physicsactor_hit(player->pa);

        if(player->shield_type != SH_NONE) {
            player->shield_type = SH_NONE;
            sound_play( soundfactory_get("damaged") );
        }
        else if(!player->disable_collectible_loss) {
            float a = 101.25f, spd = 240.0f*2;
            int i, r = min(32, player_get_collectibles());
            item_t *b;
            player_set_collectibles(0);

            /* collectibles 생성 */
            for(i=0; i<r; i++) {
                b = level_create_item(IT_BOUNCINGRING, player->actor->position);
                bouncingcollectible_set_speed(b, v2d_new(-sin(a*PI/180.0f)*spd*(1-2*(i%2)), cos(a*PI/180.0f)*spd));
                a += 22.5f * (i%2);

                if(i%16 == 0) {
                    spd /= 2.0f;
                    a = 101.25f;
                }
            }

            sound_play( soundfactory_get("collectible loss") );
        }
        else
            sound_play( soundfactory_get("damaged") );
    }
    else
        player_kill(player);
}



/*
 * player_kill()
 * player를 죽이는 함수.
 */
void player_kill(player_t *player)
{
    if(!player_is_dying(player)) {
        player->invincible = FALSE;
        player->got_speedshoes = FALSE;
        player->shield_type = SH_NONE;
        player->blinking = FALSE;
        player->attacking = FALSE;

        player->actor->position.y -= 2;
        player->actor->speed = v2d_new(0, -420);

        player->pa_old_state = physicsactor_get_state(player->pa);
        physicsactor_kill(player->pa);
        sound_play( charactersystem_get(player->name)->sample.death );
    }
}


/*
 * player_roll()
 * player가 구르게 하는 함수.
 */
void player_roll(player_t *player)
{
    player->pa_old_state = physicsactor_get_state(player->pa);
    physicsactor_roll(player->pa);
}

/*
 * player_enable_roll()
 * player가 계속 구르게 하는 함수
 */
void player_enable_roll(player_t *player)
{
    physicsactor_t *pa = player->pa;
    if(player->disable_roll) {
        physicsactor_set_rollthreshold(pa, physicsactor_get_rollthreshold(pa) - 1000.0f);
        player->disable_roll = FALSE;
    }
}

/*
 * player_disable_roll()
 * player의 구르기를 멈추는 함수
 */
void player_disable_roll(player_t *player)
{
    physicsactor_t *pa = player->pa;
    if(!(player->disable_roll)) {
        physicsactor_set_rollthreshold(pa, physicsactor_get_rollthreshold(pa) + 1000.0f);
        player->disable_roll = TRUE;
    }
}

/*
 * player_spring()
 * player가 spring을 밟을때 사용되는 함수
 */
void player_spring(player_t *player)
{
    player->pa_old_state = physicsactor_get_state(player->pa);
    physicsactor_spring(player->pa);
}

/*
 * player_drown()
 * player가 익사(수중에서)할 때 사용하는 함수. 이것은 내부에서 자동으로 호출된다.
 */
void player_drown(player_t *player)
{
    if(player->underwater && !player_is_dying(player)) {
        player->actor->position.y -= 2;
        player->actor->speed = v2d_new(0, 0);
        player->pa_old_state = physicsactor_get_state(player->pa);
        physicsactor_drown(player->pa);
        sound_play( soundfactory_get("drown") );
    }
}


/*
 * player_breathe()
 * 호흡 (air bublle, 수중에서)
 */
void player_breathe(player_t *player)
{
    if(player->underwater && physicsactor_get_state(player->pa) != PAS_BREATHING && physicsactor_get_state(player->pa) != PAS_DROWNED && physicsactor_get_state(player->pa) != PAS_DEAD) {
        player_reset_underwater_timer(player);
        player->actor->speed = v2d_new(0, 0);
        player->pa_old_state = physicsactor_get_state(player->pa);
        physicsactor_breathe(player->pa);
        sound_play( soundfactory_get("breathing") );
    }
}


/*
 * player_enter_water()
 * player가 물에 들어갈 때 호출되는 함수
 */
void player_enter_water(player_t *player)
{
    physicsactor_t *pa = player->pa;

    if(!player->underwater) {
        player->actor->speed.x *= 0.5f;
        player->actor->speed.y *= 0.25f;

        physicsactor_set_acc(pa, physicsactor_get_acc(pa) / 2.0f);
        physicsactor_set_dec(pa, physicsactor_get_dec(pa) / 2.0f);
        physicsactor_set_frc(pa, physicsactor_get_frc(pa) / 2.0f);
        physicsactor_set_rollfrc(pa, physicsactor_get_rollfrc(pa) / 2.0f);
        physicsactor_set_topspeed(pa, physicsactor_get_topspeed(pa) / 2.0f);
        physicsactor_set_air(pa, physicsactor_get_air(pa) / 2.0f);
        physicsactor_set_grv(pa, physicsactor_get_grv(pa) / 3.5f);
        physicsactor_set_jmp(pa, physicsactor_get_jmp(pa) / 1.85f);
        physicsactor_set_jmprel(pa, physicsactor_get_jmprel(pa) / 2.0f);

        sound_play( soundfactory_get("enter water") );
        player->underwater_timer = 0.0f;
        player->underwater = TRUE;
    }
}


/*
 * player_leave_water()
 * player가 물에서 나올 때 호출되는 함수
 */
void player_leave_water(player_t *player)
{
    physicsactor_t *pa = player->pa;

    if(player->underwater) {
        if(!player_is_springing(player) && !player_is_dying(player))
            player->actor->speed.y *= 2.0f;

        physicsactor_set_acc(pa, physicsactor_get_acc(pa) * 2.0f);
        physicsactor_set_dec(pa, physicsactor_get_dec(pa) * 2.0f);
        physicsactor_set_frc(pa, physicsactor_get_frc(pa) * 2.0f);
        physicsactor_set_rollfrc(pa, physicsactor_get_rollfrc(pa) * 2.0f);
        physicsactor_set_topspeed(pa, physicsactor_get_topspeed(pa) * 2.0f);
        physicsactor_set_air(pa, physicsactor_get_air(pa) * 2.0f);
        physicsactor_set_grv(pa, physicsactor_get_grv(pa) * 3.5f);
        physicsactor_set_jmp(pa, physicsactor_get_jmp(pa) * 1.85f);
        physicsactor_set_jmprel(pa, physicsactor_get_jmprel(pa) * 2.0f);

        sound_play( soundfactory_get("leave water") );
        player->underwater = FALSE;
    }
}


/*
 * player_is_underwater()
 * player가 수중에 있는지 확인하는 함수
 */
int player_is_underwater(const player_t *player)
{
    return player->underwater;
}



/*
 * player_reset_underwater_timer()
 * 수중에 있는 시간을 reset하는 함수
 */
void player_reset_underwater_timer(player_t *player)
{
    player->underwater_timer = 0.0f;
}


/*
 * player_seconds_remaining_to_drown()
 * 익사하는데 몇초가 걸리는지 알려주는 함수
 */
float player_seconds_remaining_to_drown(const player_t *player)
{
    return player->underwater && player->shield_type != SH_WATERSHIELD ? max(0.0f, PLAYER_UNDERWATER_BREATH - player->underwater_timer) : INFINITY_FLT;
}


/*
 * player_lock_horizontally_for()
 * 수평 제어 잠금 타이머
 */
void player_lock_horizontally_for(player_t *player, float seconds)
{
    physicsactor_lock_horizontally_for(player->pa, seconds);
}


/*
 * player_is_attacking()
 * player가 공격하면 TRUE를 return 하고
 * 그렇지 앟으면 FALSE를 return 한다.
 */
int player_is_attacking(const player_t *player)
{
    return player->attacking || player->invincible || physicsactor_get_state(player->pa) == PAS_JUMPING || physicsactor_get_state(player->pa) == PAS_ROLLING;
}


/*
 * player_is_rolling()
 * player가 구르는 중이면 TRUE
 */
int player_is_rolling(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_ROLLING;
}


/*
 * player_is_getting_hit()
 * player가 맞으면 TRUE
 */
int player_is_getting_hit(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_GETTINGHIT;
}

/*
 * player_is_dying()
 * player가 죽으면 TRUE
 */
int player_is_dying(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_DEAD || physicsactor_get_state(player->pa) == PAS_DROWNED;
}


/*
 * player_is_stopped()
 * player가 멈추면 TRUE
 */
int player_is_stopped(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_STOPPED;
}

/*
 * player_is_walking()
 * player가 걷고있으면 TRUE
 */
int player_is_walking(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_WALKING;
}

/*
 * player_is_running()
 * player가 달리면 TRUE
 */
int player_is_running(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_RUNNING;
}

/*
 * player_is_jumping()
 * player가 점프하면 TRUE
 */
int player_is_jumping(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_JUMPING;
}

/*
 * player_is_springing()
 * player가 스프링을 밟으면 TRUE
 */
int player_is_springing(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_SPRINGING;
}

/*
 * player_is_pushing()
 * player가 (벽을)밀고 있으면 TRUE
 */
int player_is_pushing(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_PUSHING;
}

/*
 * player_is_braking()
 * player가 braking하면 TRUE
 */
int player_is_braking(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_BRAKING;
}

/*
 * player_is_at_ledge()
 * player가 ledge에 있으면 TRUE
 */
int player_is_at_ledge(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_LEDGE;
}

/*
 * player_is_drowning()
 * player가 익사하면 TRUE
 */
int player_is_drowning(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_DROWNED;
}

/*
 * player_is_breathing()
 * player가 air bubble로 숨쉬면 TRUE
 */
int player_is_breathing(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_BREATHING;
}

/*
 * player_is_ducking()
 * player가 ducking하면 TRUE
 */
int player_is_ducking(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_DUCKING;
}

/*
 * player_is_lookingup()
 * player가 위를 보면 TRUE
 */
int player_is_lookingup(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_LOOKINGUP;
}

/*
 * player_is_waiting()
 * player가 기다리고 있으면 TRUE
 */
int player_is_waiting(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_WAITING;
}

/*
 * player_is_winning()
 * player가 이기면 TRUE
 */
int player_is_winning(const player_t *player)
{
    return physicsactor_get_state(player->pa) == PAS_WINNING;
}

/*
 * player_is_in_the_air()
 * player가 공기중에 있으면 TRUE
 */
int player_is_in_the_air(const player_t *player)
{
    return physicsactor_is_in_the_air(player->pa);
}

/*
 * player_is_ultrafast()
 * player가 speed shoes를 착용한 경우 TRUE
 */
int player_is_ultrafast(const player_t* player)
{
    return player->got_speedshoes;
}

int player_is_invincible(const player_t* player)
{
    return player->invincible;
}

/*
 * player_get_collectibles()
 * player가 지금까지 수집한 수집품의 수를 return해준다.
 */
int player_get_collectibles()
{
    return collectibles;
}



/*
 * player_set_collectibles()
 * 수집품의 새로운 양을 설정한다.
 */
void player_set_collectibles(int c)
{
    collectibles = clip(c, 0, 999);

    /* (100+) * k collectibles (k integer) = new life! */
    if(c/100 > hundred_collectibles) {
        hundred_collectibles = c/100;
        player_set_lives( player_get_lives()+1 );
        level_override_music( soundfactory_get("1up") );
    }
}



/*
 * player_get_lives()
 * player의 생명 갯수를 return하는 함수
 */
int player_get_lives()
{
    return lives;
}



/*
 * player_set_lives()
 * player의 생명 갯수를 설정하는 함수
 */
void player_set_lives(int l)
{
    lives = max(0, l);
}



/*
 * player_get_score()
 * 점수를 return해주는 함수
 */
int player_get_score()
{
    return score;
}



/*
 * player_set_score()
 * 점수를 설정하는 함수
 */
void player_set_score(int s)
{
    score = max(0, s);
}




/* private functions */

/* 위치와 현재의 shield 애니메이션을 업데이트한다. */
void update_shield(player_t *p)
{
    actor_t *sh = p->shield, *act = p->actor;
    v2d_t off = v2d_new(0,0);
    sh->position = v2d_add(act->position, v2d_rotate(off, -old_school_angle(act->angle)));

    switch(p->shield_type) {

        case SH_SHIELD:
            actor_change_animation(sh, sprite_get_animation("SD_SHIELD", 0));
            break;

        case SH_FIRESHIELD:
            actor_change_animation(sh, sprite_get_animation("SD_FIRESHIELD", 0));
            break;

        case SH_THUNDERSHIELD:
            actor_change_animation(sh, sprite_get_animation("SD_THUNDERSHIELD", 0));
            break;

        case SH_WATERSHIELD:
            actor_change_animation(sh, sprite_get_animation("SD_WATERSHIELD", 0));
            break;

        case SH_ACIDSHIELD:
            actor_change_animation(sh, sprite_get_animation("SD_ACIDSHIELD", 0));
            break;

        case SH_WINDSHIELD:
            actor_change_animation(sh, sprite_get_animation("SD_WINDSHIELD", 0));
            break;
    }
}

/* player의 애니메이션을 업데이트한다. */
void update_animation(player_t *p)
{
    /* animations */
    if(!p->disable_animation_control) {
        switch(physicsactor_get_state(p->pa)) {
            case PAS_STOPPED:    CHANGE_ANIM(stopped);    break;
            case PAS_WALKING:    CHANGE_ANIM(walking);    break;
            case PAS_RUNNING:    CHANGE_ANIM(running);    break;
            case PAS_JUMPING:    CHANGE_ANIM(jumping);    break;
            case PAS_SPRINGING:  CHANGE_ANIM(springing);  break;
            case PAS_ROLLING:    CHANGE_ANIM(rolling);    break;
            case PAS_PUSHING:    CHANGE_ANIM(pushing);    break;
            case PAS_GETTINGHIT: CHANGE_ANIM(gettinghit); break;
            case PAS_DEAD:       CHANGE_ANIM(dead);       break;
            case PAS_BRAKING:    CHANGE_ANIM(braking);    break;
            case PAS_LEDGE:      CHANGE_ANIM(ledge);      break;
            case PAS_DROWNED:    CHANGE_ANIM(drowned);    break;
            case PAS_BREATHING:  CHANGE_ANIM(breathing);  break;
            case PAS_WAITING:    CHANGE_ANIM(waiting);    break;
            case PAS_DUCKING:    CHANGE_ANIM(ducking);    break;
            case PAS_LOOKINGUP:  CHANGE_ANIM(lookingup);  break;
            case PAS_WINNING:    CHANGE_ANIM(winning);    break;
        }
    }
    else
        p->disable_animation_control = FALSE; /* for set_player_animation (scripting) */

    /* sounds */
    ON_STATE(PAS_JUMPING) {
        sound_play( charactersystem_get(p->name)->sample.jump );
    }

    ON_STATE(PAS_ROLLING) {
        sound_play( charactersystem_get(p->name)->sample.roll );
    }

    ON_STATE(PAS_BRAKING) {
        sound_play( charactersystem_get(p->name)->sample.brake );
    }

    /* "gambiarra" */
    p->actor->hot_spot = v2d_new(image_width(actor_image(p->actor))/2, image_height(actor_image(p->actor))-20);
}

/* player_t 와 physicsactor_t 사이의 interface*/
void physics_adapter(player_t *player, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    actor_t *act = player->actor;
    physicsactor_t *pa = player->pa;
    obstaclemap_t *obstaclemap;

    /* 변수들을 변환한다. */
    physicsactor_set_position(pa, act->position);
    if(physicsactor_is_in_the_air(pa) || player_is_getting_hit(player) || player_is_dying(player)) {
        physicsactor_set_xsp(pa, act->speed.x);
        physicsactor_set_ysp(pa, act->speed.y);
    }
    else {
        physicsactor_set_gsp(pa, act->speed.x);
        physicsactor_set_ysp(pa, 0.0f);
    }

    /* capturing input */
    if(input_button_down(act->input, IB_RIGHT))
        physicsactor_walk_right(pa);
    if(input_button_down(act->input, IB_LEFT))
        physicsactor_walk_left(pa);
    if(input_button_down(act->input, IB_DOWN))
        physicsactor_duck(pa);
    if(input_button_down(act->input, IB_UP))
        physicsactor_look_up(pa);
    if(input_button_down(act->input, IB_FIRE1))
        physicsactor_jump(pa);

    /* obstacle map 생성 */
    obstaclemap = obstaclemap_create();
    for(; brick_list; brick_list = brick_list->next) {
        if(brick_list->data->brick_ref->property != BRK_NONE && brick_list->data->enabled && brick_image(brick_list->data) != NULL && !ignore_obstacle(brick_list->data->brick_ref->angle, player->disable_wall, brick_list->data->layer, player->layer))
            obstaclemap_add_obstacle(obstaclemap, brick2obstacle(brick_list->data));
    }
    for(; item_list; item_list = item_list->next) {
        if(item_list->data->obstacle && !ignore_obstacle(0, player->disable_wall, BRL_DEFAULT, player->layer))
            obstaclemap_add_obstacle(obstaclemap, item2obstacle(item_list->data));
    }
    for(; object_list; object_list = object_list->next) {
        if(object_list->data->obstacle && !ignore_obstacle(object_list->data->obstacle_angle, player->disable_wall, BRL_DEFAULT, player->layer))
            obstaclemap_add_obstacle(obstaclemap, object2obstacle(object_list->data));
    }

    /* physics actor 업데이트 */
    physicsactor_update(pa, obstaclemap);

    /* obstacle map 삭제 */
    obstaclemap = obstaclemap_destroy(obstaclemap);

    /* can't leave the screen */
    if(physicsactor_get_position(pa).x < 20) {
        physicsactor_set_position(pa, v2d_new(20, physicsactor_get_position(pa).y));
        physicsactor_set_xsp(pa, 0.0f);
        physicsactor_set_gsp(pa, 0.0f);
    }
    else if(physicsactor_get_position(pa).x > level_size().x - 20) {
        physicsactor_set_position(pa, v2d_new(level_size().x - 20, physicsactor_get_position(pa).y));
        physicsactor_set_xsp(pa, 0.0f);
        physicsactor_set_gsp(pa, 0.0f);
    }

    /* 변수 unconverting */
    act->position = physicsactor_get_position(pa);
    act->speed = physicsactor_is_in_the_air(pa) || player_is_getting_hit(player) || player_is_dying(player) ? v2d_new(physicsactor_get_xsp(pa), physicsactor_get_ysp(pa)) : v2d_new(physicsactor_get_gsp(pa), 0.0f);

    /* misc */
    act->mirror = !physicsactor_is_facing_right(pa) ? IF_HFLIP : IF_NONE;
    act->angle = ((int)((256 - physicsactor_get_angle(pa)) * 1.40625f) % 360) * PI / 180.0f;
}

/* brick을 장애물로 변환 */
obstacle_t* brick2obstacle(const brick_t *brick)
{
    const image_t *image = collisionmask_image(brick_collisionmask(brick));
    int angle = (int)(256 - (brick->brick_ref->angle % 360) / 1.40625f) % 256;
    v2d_t position = v2d_new(brick->x, brick->y);
    int cloud = brick->brick_ref->property == BRK_CLOUD;

    return (cloud ? obstacle_create_oneway : obstacle_create_solid)(image, angle, position);
}

/* built-in item을 장애물로 변환 */
obstacle_t* item2obstacle(const item_t *item)
{
    const image_t *image = actor_image(item->actor);
    int angle = 0;
    v2d_t position = v2d_subtract(item->actor->position, item->actor->hot_spot);

    return obstacle_create_solid(image, angle, position);
}

/* custom object를 장애물로 변환 */
obstacle_t* object2obstacle(const object_t *object)
{
    const image_t *image = actor_image(object->actor);
    int angle = (int)(256 - (object->obstacle_angle % 360) / 1.40625f) % 256;
    v2d_t position = v2d_subtract(object->actor->position, object->actor->hot_spot);

    return obstacle_create_solid(image, angle, position);
}

/* 장애물을 무시할 수 있는지 확인 */
int ignore_obstacle(int brick_angle, int old_loop_system_flags, bricklayer_t brick_layer, bricklayer_t player_layer)
{
    int f = old_loop_system_flags, a = brick_angle % 360;

    return (
        /* loop system */
        (brick_layer != BRL_DEFAULT && player_layer != brick_layer) ||

        /* old loop system */
        ((f & PLAYER_WALL_BOTTOM) && (a == 0)) ||
        ((f & PLAYER_WALL_TOP) && (a == 180)) ||
        ((f & PLAYER_WALL_LEFT) && (a > 180+30 && a < 360)) ||
        ((f & PLAYER_WALL_RIGHT) && (a > 0 && a < 180-30))
    );
}
