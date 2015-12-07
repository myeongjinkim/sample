#include <math.h>
#include "spring.h"
#include "../../core/util.h"
#include "../../core/audio.h"
#include "../../core/timer.h"
#include "../../core/stringutil.h"
#include "../../core/soundfactory.h"
#include "../player.h"
#include "../enemy.h"

/* 상수*/
#define SPRING_BANG_TIMER           0.2 /* sfx control */

/* spring 클래스 */
typedef struct spring_t spring_t;
struct spring_t {
    item_t item; /* base class */
    v2d_t strength;
    char *sprite_name;
    float bang_timer;
    int is_bumping;
    void (*on_bump)(item_t*,player_t*); /* strategy pattern */
};

static item_t* spring_create(void (*strategy)(item_t*,player_t*), const char *sprite_name, v2d_t strength);
static void spring_init(item_t *item);
static void spring_release(item_t* item);
static void spring_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void spring_render(item_t* item, v2d_t camera_position);

static void classicspring_strategy(item_t *item, player_t *player); /* activates if you jump on it */
static void volatilespring_strategy(item_t *item, player_t *player); /* activates when touched */

static void springfy_player(player_t *player, v2d_t strength);
static void activate_spring(spring_t *spring, player_t *player);



/* public methods */
/* 객체 생성 */
item_t* yellowspring_create() /* regular spring */
{
    return spring_create(classicspring_strategy, "SD_YELLOWSPRING", v2d_new(0,-500));
}

item_t* tryellowspring_create() /* top-right */
{
    return spring_create(volatilespring_strategy, "SD_TRYELLOWSPRING", v2d_new(400,-600));
}

item_t* ryellowspring_create()  /* right-oriented spring */
{
    return spring_create(volatilespring_strategy, "SD_RYELLOWSPRING", v2d_new(600,0));
}

item_t* bryellowspring_create() /* bottom-right */
{
    return spring_create(volatilespring_strategy, "SD_BRYELLOWSPRING", v2d_new(400,600));
}

item_t* byellowspring_create() /* bottom-oriented spring */
{
    return spring_create(volatilespring_strategy, "SD_BYELLOWSPRING", v2d_new(0,500));
}

item_t* blyellowspring_create() /* bottom-left */
{
    return spring_create(volatilespring_strategy, "SD_BLYELLOWSPRING", v2d_new(-400,600));
}

item_t* lyellowspring_create() /* left-oriented spring */
{
    return spring_create(volatilespring_strategy, "SD_LYELLOWSPRING", v2d_new(-600,0));
}

item_t* tlyellowspring_create() /* top-left */
{
    return spring_create(volatilespring_strategy, "SD_TLYELLOWSPRING", v2d_new(-400,-600));
}

item_t* redspring_create() /* regular spring */
{
    return spring_create(classicspring_strategy, "SD_REDSPRING", v2d_new(0,-750));
}

item_t* trredspring_create() /* top-right */
{
    return spring_create(volatilespring_strategy, "SD_TRREDSPRING", v2d_new(800,-1000));
}

item_t* rredspring_create()  /* right-oriented spring */
{
    return spring_create(volatilespring_strategy, "SD_RREDSPRING", v2d_new(1200,0));
}

item_t* brredspring_create() /* bottom-right */
{
    return spring_create(volatilespring_strategy, "SD_BRREDSPRING", v2d_new(800,1000));
}

item_t* bredspring_create() /* bottom-oriented spring */
{
    return spring_create(volatilespring_strategy, "SD_BREDSPRING", v2d_new(0,750));
}

item_t* blredspring_create() /* bottom-left */
{
    return spring_create(volatilespring_strategy, "SD_BLREDSPRING", v2d_new(-800,1000));
}

item_t* lredspring_create() /* left-oriented spring */
{
    return spring_create(volatilespring_strategy, "SD_LREDSPRING", v2d_new(-1200,0));
}

item_t* tlredspring_create() /* top-left */
{
    return spring_create(volatilespring_strategy, "SD_TLREDSPRING", v2d_new(-800,-1000));
}

item_t* bluespring_create() /* regular spring */
{
    return spring_create(classicspring_strategy, "SD_BLUESPRING", v2d_new(0,-1500));
}

item_t* trbluespring_create() /* top-right */
{
    return spring_create(volatilespring_strategy, "SD_TRBLUESPRING", v2d_new(1800,-1500));
}

item_t* rbluespring_create()  /* right-oriented spring */
{
    return spring_create(volatilespring_strategy, "SD_RBLUESPRING", v2d_new(2000,0));
}

item_t* brbluespring_create() /* bottom-right */
{
    return spring_create(volatilespring_strategy, "SD_BRBLUESPRING", v2d_new(1800,1500));
}

item_t* bbluespring_create() /* bottom-oriented spring */
{
    return spring_create(volatilespring_strategy, "SD_BBLUESPRING", v2d_new(0,1500));
}

item_t* blbluespring_create() /* bottom-left */
{
    return spring_create(volatilespring_strategy, "SD_BLBLUESPRING", v2d_new(-1800,1500));
}

item_t* lbluespring_create() /* left-oriented spring */
{
    return spring_create(volatilespring_strategy, "SD_LBLUESPRING", v2d_new(-2000,0));
}

item_t* tlbluespring_create() /* top-left */
{
    return spring_create(volatilespring_strategy, "SD_TLBLUESPRING", v2d_new(-1800,-1500));
}
/* 여기 까지 객체 생성 */

/* private strategies */

/* 플레이어가 그것들을 건들이면 잠시동안 속성이 도출되어 spring이 활성화 된다.*/
void volatilespring_strategy(item_t *item, player_t *player)
{
    activate_spring((spring_t*)item, player);
}

/* 플레이어가 spring을 뛰었을때 본래의 속성이 도출되고 spring이 활성화 된다. */
void classicspring_strategy(item_t *item, player_t *player)
{
    if(player->actor->speed.y >= 1.0f && !player->actor->carrying && !player->actor->carried_by)
        activate_spring((spring_t*)item, player);
}

/* private methods */
/* spring 객체 생성 */
item_t* spring_create(void (*strategy)(item_t*,player_t*), const char *sprite_name, v2d_t strength)
{
    item_t *item = mallocx(sizeof(spring_t));
    spring_t *me = (spring_t*)item;

    item->init = spring_init;
    item->release = spring_release;
    item->update = spring_update;
    item->render = spring_render;

    me->on_bump = strategy;
    me->sprite_name = str_dup(sprite_name);
    me->strength = strength;

    return item;
}


/* private methods */
/* spring 생성  */
void spring_init(item_t *item)
{
    spring_t *me = (spring_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    me->is_bumping = FALSE;
    me->bang_timer = 0.0f;
    actor_change_animation(item->actor, sprite_get_animation(me->sprite_name, 0));
}

/* spring 없애는 모습 */
void spring_release(item_t* item)
{
    spring_t *me = (spring_t*)item;

    actor_destroy(item->actor);
    free(me->sprite_name);
}

/* spring 특성 생성 */
void spring_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    spring_t *me = (spring_t*)item;
    float dt = timer_get_delta();
    int i;

    /* 반동 */
    me->bang_timer += dt;
    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(!player->dying && actor_pixelperfect_collision(player->actor, item->actor))
            me->on_bump(item, player);
    }

    /* 기존 움직임을 복원 */
    if(me->is_bumping) {
        if(actor_animation_finished(item->actor)) {
            actor_change_animation(item->actor, sprite_get_animation(me->sprite_name, 0));
            me->is_bumping = FALSE;
        }
    }
}

/* crushedbox 모습 */

void spring_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}


/* 'springfy' player
스프링에 뛰어 올랐을 시 플레이어 캐릭터의 움직임(날아오르기, 등반, 공격, 더 높은 점프 등)*/
void springfy_player(player_t *player, v2d_t strength)
{
    float dt = timer_get_delta();
    int same_signal[2], different_signal[2];

    same_signal[0] = (strength.x * player->actor->speed.x >= 0.0f);
    same_signal[1] = (strength.y * player->actor->speed.y >= 0.0f);
    different_signal[0] = (strength.x * player->actor->speed.x <= 0.0f) && (fabs(strength.x) > EPSILON);
    different_signal[1] = (strength.y * player->actor->speed.y <= 0.0f) && (fabs(strength.y) > EPSILON);

    if(fabs(strength.y) > EPSILON)
        player->spring = TRUE;

    player->flying = FALSE;
    player->climbing = FALSE;
    player->landing = FALSE;
    player->getting_hit = FALSE;
    player->is_fire_jumping = FALSE;

    if(
        (fabs(strength.x) > fabs(player->actor->speed.x) && same_signal[0]) ||
        different_signal[0]
    )
        player->actor->speed.x = strength.x;

    if(
        (fabs(strength.y) > fabs(player->actor->speed.y) && same_signal[1]) ||
        different_signal[1]
    ) {
        player->actor->speed.y = strength.y;
        player->actor->position.y += player->actor->speed.y * dt; /* hack: leave the ground! */
    }
}

/* spring 활성화, 소리 생성 */
void activate_spring(spring_t *spring, player_t *player)
{
    item_t *item = (item_t*)spring;

    spring->is_bumping = TRUE;
    springfy_player(player, spring->strength);
    actor_change_animation(item->actor, sprite_get_animation(spring->sprite_name, 1));

    if(spring->strength.x > EPSILON)
        player->actor->mirror &= ~IF_HFLIP;
    else if(spring->strength.x < -EPSILON)
        player->actor->mirror |= IF_HFLIP;

    if(spring->bang_timer > SPRING_BANG_TIMER) {
        sound_play( soundfactory_get("spring") );
        spring->bang_timer = 0.0f;
    }
}
