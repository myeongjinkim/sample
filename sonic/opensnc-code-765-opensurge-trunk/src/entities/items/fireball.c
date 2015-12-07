#include "fireball.h"
#include "../../core/util.h"
#include "../../core/soundfactory.h"
#include "../../scenes/level.h"

/* fireball 클래스 */
typedef struct fireball_t fireball_t;
struct fireball_t {
    item_t item; /* base class */
    void (*run)(item_t*,brick_list_t*);
};

static void fireball_init(item_t *item);
static void fireball_release(item_t* item);
static void fireball_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void fireball_render(item_t* item, v2d_t camera_position);

static void fireball_set_behavior(item_t *fireball, void (*behavior)(item_t*,brick_list_t*));

static void falling_behavior(item_t *fireball, brick_list_t *brick_list);
static void disappearing_behavior(item_t *fireball, brick_list_t *brick_list);
static void smallfire_behavior(item_t *fireball, brick_list_t *brick_list);



/* fireball 객체 생성 */
item_t* fireball_create()
{
    item_t *item = mallocx(sizeof(fireball_t));

    item->init = fireball_init;
    item->release = fireball_release;
    item->update = fireball_update;
    item->render = fireball_render;

    return item;
}


/* fireball 행동 설정 */
void fireball_set_behavior(item_t *fireball, void (*behavior)(item_t*,brick_list_t*))
{
    fireball_t *me = (fireball_t*)fireball;
    me->run = behavior;
}
/* fireball 생성 */
void fireball_init(item_t *item)
{
    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = FALSE;
    item->actor = actor_create();

    fireball_set_behavior(item, falling_behavior);
    actor_change_animation(item->actor, sprite_get_animation("SD_FIREBALL", 0));
}


/* fireball 파괴, 삭제 */
void fireball_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* fireball 특성 생성, 설정 (방어막 감소)*/
void fireball_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    int i;
    actor_t *act = item->actor;
    fireball_t *me = (fireball_t*)item;

    /* 캐릭터를 공격 */
    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(!player->dying && actor_collision(act, player->actor)) {
            item->state = IS_DEAD;
            if(player->shield_type != SH_FIRESHIELD)
                player_hit(player);
        }
    }

    /* 캐릭터 행동*/
    me->run(item, brick_list);
}

/* fireball 모습 */
void fireball_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}


/* fireball 떨어질때의 행동  */
void falling_behavior(item_t *fireball, brick_list_t *brick_list)
{
    int i, n;
    float sqrsize = 2, diff = -2;
    actor_t *act = fireball->actor;
    brick_t *down;

    /* 그에 따른 움직임, 카메라 시점 */
    act->speed.x = 0.0f;
    act->mirror = (act->speed.y < 0.0f) ? IF_VFLIP : IF_NONE;
    actor_move(act, actor_particle_movement(act, level_gravity()));
    actor_change_animation(act, sprite_get_animation("SD_FIREBALL", 0));

    /* 파괴하기 위한 목표 */
    actor_corners(act, sqrsize, diff, brick_list, NULL, NULL, NULL, NULL, &down, NULL, NULL, NULL);
    actor_handle_clouds(act, diff, NULL, NULL, NULL, NULL, &down, NULL, NULL, NULL);
    if(down) {
        /* 지면에 닿을 때의 소리 */
        fireball_set_behavior(fireball, disappearing_behavior);
        sound_play( soundfactory_get("fire2") );

        /* 좀 더 작은 fireball 생성 */
        n = 2 + random(3);
        for(i=0; i<n; i++) {
            item_t *obj = level_create_item(IT_FIREBALL, act->position);
            fireball_set_behavior(obj, smallfire_behavior);
            obj->actor->speed = v2d_new(((float)i/(float)n)*400.0f-200.0f, -120.0f-random(240.0f));
        }
    }
}
/* fireball이 사라진 후의 행동*/
void disappearing_behavior(item_t *fireball, brick_list_t *brick_list)
{
    actor_t *act = fireball->actor;

    actor_change_animation(act, sprite_get_animation("SD_FIREBALL", 1));
    if(actor_animation_finished(act))
        fireball->state = IS_DEAD;
}
/* 작은 fireball 행동 설정 */
void smallfire_behavior(item_t *fireball, brick_list_t *brick_list)
{
    float sqrsize = 2, diff = -2;
    actor_t *act = fireball->actor;
    brick_t *down;

    /* 움직임 */
    actor_move(act, actor_particle_movement(act, level_gravity()));
    actor_change_animation(act, sprite_get_animation("SD_FIREBALL", 2));

    /* 파괴할 때의 방법 */
    actor_corners(act, sqrsize, diff, brick_list, NULL, NULL, NULL, NULL, &down, NULL, NULL, NULL);
    actor_handle_clouds(act, diff, NULL, NULL, NULL, NULL, &down, NULL, NULL, NULL);
    if(down && act->speed.y > 0.0f)
        fireball->state = IS_DEAD;
}
