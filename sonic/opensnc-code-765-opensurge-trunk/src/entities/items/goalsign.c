#include "goalsign.h"
#include "util/itemutil.h"
#include "../../core/util.h"
#include "../player.h"
#include "../enemy.h"

/* goalsign 클래스 */
typedef struct goalsign_t goalsign_t;
struct goalsign_t {
    item_t item; /* base class */
};

static void goalsign_init(item_t *item);
static void goalsign_release(item_t* item);
static void goalsign_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void goalsign_render(item_t* item, v2d_t camera_position);

/* goalsign 객체 생성 */
item_t* goalsign_create()
{
    item_t *item = mallocx(sizeof(goalsign_t));

    item->init = goalsign_init;
    item->release = goalsign_release;
    item->update = goalsign_update;
    item->render = goalsign_render;

    return item;
}


/* goalsign 생성 */
void goalsign_init(item_t *item)
{
    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation("SD_GOAL", 0));
}


/* goalsign 삭제 모습 */
void goalsign_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* goalsign 특성 생성 */
void goalsign_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    item_t *endsign;
    int anim;

    endsign = find_closest_item(item, item_list, IT_ENDSIGN, NULL);
    if(endsign != NULL) {
        if(endsign->actor->position.x > item->actor->position.x)
            anim = 0;
        else
            anim = 1;
    }
    else
        anim = 0;

    actor_change_animation(item->actor, sprite_get_animation("SD_GOAL", anim));
}

/* goalsign 모습 */
void goalsign_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
