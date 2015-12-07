#include "checkpointorb.h"
#include "../../core/util.h"
#include "../../core/soundfactory.h"
#include "../../scenes/level.h"

/* checkpointorb class */
typedef struct checkpointorb_t checkpointorb_t;
struct checkpointorb_t {
    item_t item; /* base class */
    int is_active; /* checkpointorb 에 캐릭터가 이동 했는지 여부 */
};

static void checkpointorb_init(item_t *item);
static void checkpointorb_release(item_t* item);
static void checkpointorb_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void checkpointorb_render(item_t* item, v2d_t camera_position);



/* checkpointorb 생성 객체 */
item_t* checkpointorb_create()
{
    item_t *item = mallocx(sizeof(checkpointorb_t));

    item->init = checkpointorb_init;
    item->release = checkpointorb_release;
    item->update = checkpointorb_update;
    item->render = checkpointorb_render;

    return item;
}


/* checkpointorb 생성 */
void checkpointorb_init(item_t *item)
{
    checkpointorb_t *me = (checkpointorb_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    me->is_active = FALSE;
    actor_change_animation(item->actor, sprite_get_animation("SD_CHECKPOINT", 0));
}


/* checkpointorb 삭제 */
void checkpointorb_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* checkpointorb 캐릭터가 이동시 변화 */
void checkpointorb_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    checkpointorb_t *me = (checkpointorb_t*)item;
    actor_t *act = item->actor;
    int i;

    if(!me->is_active) {
        /* checkpointorb 활성화 */
        for(i=0; i<team_size; i++) {
            player_t *player = team[i];
            if(!player->dying && actor_pixelperfect_collision(player->actor, act)) {
                me->is_active = TRUE;
                sound_play( soundfactory_get("checkpoint orb") );
                level_set_spawn_point(act->position);
                actor_change_animation(act, sprite_get_animation("SD_CHECKPOINT", 1));
                break;
            }
        }
    }
    else {
        if(actor_animation_finished(act))
            actor_change_animation(act, sprite_get_animation("SD_CHECKPOINT", 2));
    }
}

/* checkpointorb 모습 */
void checkpointorb_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
