
#include "falglasses.h"
#include "../../core/util.h"
#include "../../core/timer.h"
#include "../../scenes/level.h"

/* falglasses 클래스 */
typedef struct falglasses_t falglasses_t;
struct falglasses_t {
    item_t item; /* base class */
};

static void falglasses_init(item_t *item);
static void falglasses_release(item_t* item);
static void falglasses_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void falglasses_render(item_t* item, v2d_t camera_position);



/* falglasses 객체 생성 */
item_t* falglasses_create()
{
    item_t *item = mallocx(sizeof(falglasses_t));

    item->init = falglasses_init;
    item->release = falglasses_release;
    item->update = falglasses_update;
    item->render = falglasses_render;

    return item;
}
/* falglasses 캐릭터 속도 설정 */
void falglasses_set_speed(item_t *item, v2d_t speed)
{
    if(item->actor != NULL)
        item->actor->speed = speed;
}


/* falglasses 생성  */
void falglasses_init(item_t *item)
{
    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = FALSE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation("SD_GLASSES", 4));
    item->actor->hot_spot.y *= 0.5f;
}


/* falglasses 없애는 함수 */
void falglasses_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* falglasses 특성 생성 */
void falglasses_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    actor_t *act = item->actor;
    float dt = timer_get_delta();

    act->angle += sign(act->speed.x) * (6.0f * PI * dt);
    act->position = v2d_add(act->position, actor_particle_movement(act, level_gravity()));
}

/* falglasses 모습 */
void falglasses_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
