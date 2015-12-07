
#include "endsign.h"
#include "../../core/util.h"
#include "../../core/soundfactory.h"
#include "../../scenes/level.h"

/* endsign 클래스 */
typedef struct endsign_t endsign_t;
struct endsign_t {
    item_t item; /* base class */
    player_t *who;
    /* endsign을 누가 먼저 접근 하는지 */
};

static void endsign_init(item_t *item);
static void endsign_release(item_t* item);
static void endsign_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void endsign_render(item_t* item, v2d_t camera_position);



/* endsign 객체 생성 */
item_t* endsign_create()
{
    item_t *item = mallocx(sizeof(endsign_t));

    item->init = endsign_init;
    item->release = endsign_release;
    item->update = endsign_update;
    item->render = endsign_render;

    return item;
}


/* endsign 상태 생성 */
void endsign_init(item_t *item)
{
    endsign_t *me = (endsign_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = TRUE;
    item->actor = actor_create();

    me->who = NULL;
    actor_change_animation(item->actor, sprite_get_animation("SD_ENDSIGN", 0));
}


/* endsign 상태 파괴 */
void endsign_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* endsign 특성 생성 */
void endsign_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    endsign_t *me = (endsign_t*)item;
    actor_t *act = item->actor;

    if(me->who == NULL) {
        /* 접촉 하지 않은 상태*/
        int i;

        for(i=0; i<team_size; i++) {
            player_t *player = team[i];
            if(!player->dying && actor_pixelperfect_collision(player->actor, act)) {
                me->who = player;
                /* 플레이어가 접근 하였는지 */
                sound_play( soundfactory_get("end sign") );
                actor_change_animation(act, sprite_get_animation("SD_ENDSIGN", 1));
                level_clear(item->actor);
            }
        }
    }
    else {
        /* 누가 접근 하였는지 */
        if(actor_animation_finished(act)) {
            int anim_id = 2 + me->who->type;
            /* 안전 한 상태 */
            actor_change_animation(act, sprite_get_animation("SD_ENDSIGN", anim_id));
        }
    }
}

/* endsign 모습 */
void endsign_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
