
#include "bigring.h"
#include "../../core/util.h"
#include "../../core/soundfactory.h"
#include "../../scenes/level.h"
#include "../../scenes/quest.h"

/* bigring 클래스 */
typedef struct bigring_t bigring_t;
struct bigring_t {
    item_t item; /* base class */
};

static void bigring_init(item_t *item);
static void bigring_release(item_t* item);
static void bigring_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void bigring_render(item_t* item, v2d_t camera_position);



/* bigring 아이템 생성, 삭제, 장착 부분*/
item_t* bigring_create()
{
    item_t *item = mallocx(sizeof(bigring_t));

    item->init = bigring_init;
    item->release = bigring_release;
    item->update = bigring_update;
    item->render = bigring_render;

    return item;
}


/* bigring 아이템 생성 */
void bigring_init(item_t *item)
{
    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = TRUE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation("SD_BIGRING", 0));
}


/* bigring 삭제 */
void bigring_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* bigring 아이템 능력치 조정 */
void bigring_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    int i;

    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(!player->dying && actor_collision(player->actor, item->actor)) {
            item->state = IS_DEAD;
            player_set_rings( player_get_rings() + 50 );
            level_add_to_secret_bonus(5000);
            sound_play( soundfactory_get("big ring") );
            level_call_dialogbox("$BONUSMSG_TITLE", "$BONUSMSG_TEXT");
            quest_setvalue(QUESTVALUE_BIGRINGS, quest_getvalue(QUESTVALUE_BIGRINGS) + 1);
        }
    }
}

/* bigring 아이템 장착시에 모습 */
void bigring_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
