#include <math.h>
#include "ring.h"
#include "../../scenes/level.h"
#include "../../core/util.h"
#include "../../core/timer.h"
#include "../../core/audio.h"
#include "../../core/soundfactory.h"

/* ring 클래스 */
typedef struct ring_t ring_t;
struct ring_t {
    item_t item; /* base class */
    int is_disappearing; /* 플레이어가 획득하고있는 ring 변수 */
    int is_moving; /* 플레이어가 건들이지 않은 ring 변수 */
    float life_time; /* 수명시간 ( 시간이 지남에 따라 반지를 파괴하는 시간 계산에 이용하는 변수) */
};

static void ring_init(item_t *item);
static void ring_release(item_t* item);
static void ring_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void ring_render(item_t* item, v2d_t camera_position);



/* ring 객체 생성, 할당 */
item_t* ring_create()
{
    item_t *item = mallocx(sizeof(ring_t));

    item->init = ring_init;
    item->release = ring_release;
    item->update = ring_update;
    item->render = ring_render;

    return item;
}
/* ring 획득시에 캐릭터 움직임 변화 */
void ring_start_bouncing(item_t *ring)
{
    ring_t *me = (ring_t*)ring;

    me->is_moving = TRUE;
    ring->actor->speed.x = ring->actor->maxspeed * (random(100)-50)/100;
    ring->actor->speed.y = -ring->actor->jump_strength + random(ring->actor->jump_strength);
}



/* ring 생성  */
void ring_init(item_t *item)
{
    ring_t *me = (ring_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = TRUE;
    item->actor = actor_create();
    item->actor->maxspeed = 220 + random(140);
    item->actor->jump_strength = (350 + random(50)) * 1.2;
    item->actor->input = input_create_computer();

    me->is_disappearing = FALSE;
    me->is_moving = FALSE;
    me->life_time = 0.0f;

    actor_change_animation(item->actor, sprite_get_animation("SD_RING", 0));
}


/* ring 없애는 모습 */
void ring_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* ring 특성 생성 */
void ring_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    int i;
    float dt = timer_get_delta();
    ring_t *me = (ring_t*)item;
    actor_t *act = item->actor;

    /* 플레이어가 ring 획득 할 시 ring 갯수 증가, 소리 생성 */
    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(
            (!me->is_moving || (me->is_moving && !player->getting_hit && me->life_time >= 0.5f)) &&
            !me->is_disappearing &&
            !player->dying &&
            actor_collision(act, player->actor)
        ) {
            player_set_rings(player_get_rings() + 1);
            me->is_disappearing = TRUE;
            sound_play( soundfactory_get("ring") );
            break;
        }
    }

    /* 움직임 사라지는 부분 */
    if(me->is_disappearing) {
        actor_change_animation(act, sprite_get_animation("SD_RING", 1));
        if(actor_animation_finished(act))
            item->state = IS_DEAD;
    }

    /* ring이 배경 주변에 움직이는 부분 */
    else if(me->is_moving) {
        float sqrsize = 2, diff = -2;
        brick_t *left = NULL, *right = NULL, *down = NULL;
        actor_corners(act, sqrsize, diff, brick_list, NULL, NULL, &right, NULL, &down, NULL, &left, NULL);
        actor_handle_clouds(act, diff, NULL, NULL, &right, NULL, &down, NULL, &left, NULL);
        input_simulate_button_down(act->input, IB_FIRE1);
        item->preserve = FALSE;

        /* 캐릭터 생명치 */
        me->life_time += dt;
        if(me->life_time > 5.0f) {
            int val = 240 + random(20);
            act->visible = ((int)(timer_get_ticks() % val) < val/2);
            if(me->life_time > 8.0f)
                item->state = IS_DEAD;
        }

        /* 움직임 유지하는 부분 */
        if(right && act->speed.x > 0.0f)
            act->speed.x = -fabs(act->speed.x);

        if(left && act->speed.x < 0.0f)
            act->speed.x = fabs(act->speed.x);

        if(down && act->speed.y > 0.0f)
            act->jump_strength *= 0.95f;

        actor_move(act, actor_platform_movement(act, brick_list, level_gravity()));
    }

    /* ring 천둥 방패 획득시 속도 증가 */
    else if(level_player()->shield_type == SH_THUNDERSHIELD) {
        float threshold = 120.0f;
        v2d_t diff = v2d_subtract(level_player()->actor->position, act->position);
        if(v2d_magnitude(diff) < threshold) {
            float speed = 320.0f;
            v2d_t d = v2d_multiply(v2d_normalize(diff), speed);
            act->position.x += d.x * dt;
            act->position.y += d.y * dt;
        }
    }
}

/* ring 모습 */
void ring_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
