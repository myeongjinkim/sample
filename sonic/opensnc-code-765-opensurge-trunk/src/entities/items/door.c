
#include "door.h"
#include "../../core/util.h"
#include "../../core/audio.h"
#include "../../core/timer.h"
#include "../../core/soundfactory.h"
#include "../player.h"
#include "../enemy.h"

/* door 클래스 */
typedef struct door_t door_t;
struct door_t {
    item_t item; /* base class */
    int is_closed; /* is the door closed? */
};

static void door_init(item_t *item);
static void door_release(item_t* item);
static void door_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void door_render(item_t* item, v2d_t camera_position);



/* 문 생성, 할당 */
item_t* door_create()
{
    item_t *item = mallocx(sizeof(door_t));

    item->init = door_init;
    item->release = door_release;
    item->update = door_update;
    item->render = door_render;

    return item;
}
/* 문이 열릴때의 효과음 */
void door_open(item_t *door)
{
    door_t *me = (door_t*)door;
    me->is_closed = FALSE;
    sound_play( soundfactory_get("open door") );
}
/* 문이 닫힐때의 효과음 */
void door_close(item_t *door)
{
    door_t *me = (door_t*)door;
    me->is_closed = TRUE;
    sound_play( soundfactory_get("close door") );
}


/* 문의 객체 생성 */
void door_init(item_t *item)
{
    door_t *me = (door_t*)item;

    item->obstacle = TRUE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    me->is_closed = TRUE;
    actor_change_animation(item->actor, sprite_get_animation("SD_DOOR", 0));
}


/* 문의 파괴 상태 */
void door_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* 문의 모습, 상태 */
void door_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    door_t *me = (door_t*)item;
    actor_t *act = item->actor;
    float speed = 2000.0f;
    float dt = timer_get_delta();

    if(me->is_closed)
        act->position.y = min(act->position.y + speed*dt, act->spawn_point.y);
    else
        act->position.y = max(act->position.y - speed*dt, act->spawn_point.y - actor_image(act)->h * 0.8);
}

/* 문의 캐릭터 모습 */
void door_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
