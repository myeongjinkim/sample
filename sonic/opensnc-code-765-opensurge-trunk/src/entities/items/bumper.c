
#include "bumper.h"
#include "../../core/util.h"
#include "../../core/audio.h"
#include "../../core/soundfactory.h"
#include "../player.h"
#include "../item.h"
#include "../enemy.h"
#include "../brick.h"

/* bumper 클래스 */
typedef struct bumper_t bumper_t;
struct bumper_t {
    item_t item; /* base class */
    int getting_hit;
};

static void bumper_init(item_t *item);
static void bumper_release(item_t* item);
static void bumper_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void bumper_render(item_t* item, v2d_t camera_position);

static void bump(item_t *bumper, player_t *player);


/* public methods */
item_t* bumper_create()
{
    item_t *item = mallocx(sizeof(bumper_t));

    item->init = bumper_init;
    item->release = bumper_release;
    item->update = bumper_update;
    item->render = bumper_render;

    return item;
}


/* bumper장애물 생성 */
void bumper_init(item_t *item)
{
    bumper_t *me = (bumper_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    me->getting_hit = FALSE;
    actor_change_animation(item->actor, sprite_get_animation("SD_BUMPER", 0));
}


/* bumper 삭제 */
void bumper_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* bumper 장애물 능력 */
void bumper_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    bumper_t *me = (bumper_t*)item;
    actor_t *act = item->actor;
    int i;

    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(!player->dying && actor_pixelperfect_collision(player->actor, act)) {
            if(!me->getting_hit) {
                me->getting_hit = TRUE;
                actor_change_animation(act, sprite_get_animation("SD_BUMPER", 1));
                sound_play( soundfactory_get("bumper") );
                bump(item, player);
            }
        }
    }
/* bumper 장애물 공격 모습, 횟수 저장 */
    if(me->getting_hit) {
        if(actor_animation_finished(act)) {
            me->getting_hit = FALSE;
            actor_change_animation(act, sprite_get_animation("SD_BUMPER", 0));
        }
    }
}

/* bumper 장애물 모습 */
void bumper_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}


void bump(item_t *bumper, player_t *player)
{
    /* 캐릭터가 장애물 지나갈 시 움직임 변화 계산하는 부분 */
    float ec = 1.0f; /* (계수 == 1.0) => 탄성 충돌 */
    float mass_player = 1.0f;
    float mass_bumper = 10000.0f;
    float mass_ratio = mass_bumper / mass_player;
    v2d_t v0, approximation_speed, separation_speed;
    actor_t *act = bumper->actor;



    v0 = player->actor->speed; /* 플레이어의 처음 속도 */
    v0.x = (v0.x < 0) ? min(-300, v0.x) : max(300, v0.x);


/* 움직임에 대한 변화 모습 */
    approximation_speed = v2d_multiply(
        v2d_normalize(
            v2d_subtract(act->position, player->actor->position)
        ),
        v2d_magnitude(v0)
    );

    separation_speed = v2d_multiply(approximation_speed, ec);


/* 캐릭터 움직임에 대한 속도 */
    player->actor->speed = v2d_multiply(
        v2d_add(
            v0,
            v2d_multiply(separation_speed, -mass_ratio)
        ),
        1.0f / (mass_ratio + 1.0f)
    );

    act->speed = v2d_multiply(
        v2d_add(v0, separation_speed),
        1.0f / (mass_ratio + 1.0f)
    );



    /* 캐릭터 움직임 */
    player->flying = FALSE;
    player->landing = FALSE;
    player->climbing = FALSE;
    player->spring = FALSE;
}
