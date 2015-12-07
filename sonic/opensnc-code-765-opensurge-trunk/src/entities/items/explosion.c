
#include "explosion.h"
#include "../../core/util.h"
#include "../player.h"
#include "../enemy.h"

/* explosion 클래스 */
typedef struct explosion_t explosion_t;
struct explosion_t {
    item_t item; /* base class */
};

static void explosion_init(item_t *item);
static void explosion_release(item_t* item);
static void explosion_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void explosion_render(item_t* item, v2d_t camera_position);



/* explosion(폭발 상태) 겍체 생성, 할당 */
item_t* explosion_create()
{
    item_t *item = mallocx(sizeof(explosion_t));

    item->init = explosion_init;
    item->release = explosion_release;
    item->update = explosion_update;
    item->render = explosion_render;

    return item;
}


/* explosion 상태 생성 */
void explosion_init(item_t *item)
{
    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = FALSE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation("SD_EXPLOSION", random(2)));
}


/* explosion 상태 삭제, 파괴 */
void explosion_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* explosion 특성 생성 */
void explosion_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    if(actor_animation_finished(item->actor))
        item->state = IS_DEAD;
}

/* explosion 상태 모습 */
void explosion_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
