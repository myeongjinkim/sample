#include "crushedbox.h"
#include "../../core/util.h"
#include "../player.h"
#include "../enemy.h"

/* crushedbox 클래스 */
typedef struct crushedbox_t crushedbox_t;
struct crushedbox_t {
    item_t item; /* base class */
};

static void crushedbox_init(item_t *item);
static void crushedbox_release(item_t* item);
static void crushedbox_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void crushedbox_render(item_t* item, v2d_t camera_position);
/* crushedbox 객체 생성 */
item_t* crushedbox_create()
{
    item_t *item = mallocx(sizeof(crushedbox_t));

    item->init = crushedbox_init;
    item->release = crushedbox_release;
    item->update = crushedbox_update;
    item->render = crushedbox_render;

    return item;
}
/* crushedbox 생성  */
void crushedbox_init(item_t *item)
{
    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation("SD_ITEMBOX", 10));
}


/* crushedbox 없애는 모습 */
void crushedbox_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* crushedbox 특성 생성 */
void crushedbox_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    ;
}

/* crushedbox 모습 */
void crushedbox_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
