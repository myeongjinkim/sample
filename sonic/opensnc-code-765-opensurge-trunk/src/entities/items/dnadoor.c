
#include "dnadoor.h"
#include "../../core/util.h"
#include "../../core/timer.h"
#include "../player.h"
#include "../enemy.h"

/* dnadoor 클래스 */
typedef struct dnadoor_t dnadoor_t;
struct dnadoor_t {
    item_t item; /* base class */
    int authorized_player_type;
    int is_vertical_door;
};

static item_t *dnadoor_create(int authorized_player_type, int is_vertical_door);
static void dnadoor_init(item_t *item);
static void dnadoor_release(item_t* item);
static void dnadoor_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void dnadoor_render(item_t* item, v2d_t camera_position);

static int hittest(player_t *player, item_t *dnadoor);


/* 큰 파도모양 dnadoor 생성 */
item_t* surge_dnadoor_create()
{
    return dnadoor_create(PL_SONIC, TRUE);
}
/* 네온모양 dnadoor 생성 */
item_t* neon_dnadoor_create()
{
    return dnadoor_create(PL_TAILS, TRUE);
}
/*  dnadoor 생성 */
item_t* charge_dnadoor_create()
{
    return dnadoor_create(PL_KNUCKLES, TRUE);
}
/* 큰 파도모양 수평선 dnadoor 생성 */
item_t* surge_horizontal_dnadoor_create()
{
    return dnadoor_create(PL_SONIC, FALSE);
}
/* 네온모양 수평선 dnadoor 생성 */
item_t* neon_horizontal_dnadoor_create()
{
    return dnadoor_create(PL_TAILS, FALSE);
}
/* 수평선 dnadoor 생성 */
item_t* charge_horizontal_dnadoor_create()
{
    return dnadoor_create(PL_KNUCKLES, FALSE);
}


/*  dnadoor 객체 생성 */
item_t* dnadoor_create(int authorized_player_type, int is_vertical_door)
{
    item_t *item = mallocx(sizeof(dnadoor_t));
    dnadoor_t *me = (dnadoor_t*)item;

    item->init = dnadoor_init;
    item->release = dnadoor_release;
    item->update = dnadoor_update;
    item->render = dnadoor_render;

    me->authorized_player_type = authorized_player_type;
    me->is_vertical_door = is_vertical_door;

    return item;
}

/* dnadoor 생성 */
void dnadoor_init(item_t *item)
{
    dnadoor_t *me = (dnadoor_t*)item;
    char *sprite_name;
    int anim_id;

    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = TRUE;
    item->actor = actor_create();

    anim_id = (int)me->authorized_player_type;
    sprite_name = me->is_vertical_door ? "SD_DNADOOR" : "SD_HORIZONTALDNADOOR";
    actor_change_animation(item->actor, sprite_get_animation(sprite_name, anim_id));
}


/* dnadoor 파괴 */
void dnadoor_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* dnadoor 모습, 상태  */
void dnadoor_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    dnadoor_t *me = (dnadoor_t*)item;
    actor_t *act = item->actor;
    item_list_t *it;
    int block_anyway = FALSE;
    int perfect_collision = FALSE;
    float dt = timer_get_delta();
    float a[4], b[4], diff=5;
    int i;

    /* dnadoor 장애물 상태  */
    item->obstacle = TRUE;
    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(!player->dying && !player->actor->carrying && !player->actor->carried_by && hittest(player, item)) {
            if(player->type == me->authorized_player_type) {
                item->obstacle = FALSE;
                perfect_collision = actor_pixelperfect_collision(act, player->actor);
            }
            else
                block_anyway = TRUE;
        }
    }
    if(block_anyway)
        item->obstacle = TRUE;

    /* 효과 */
    if(item->obstacle)
        act->alpha = min(1.0f, act->alpha + 2.0f * dt);
    else if(perfect_collision)
        act->alpha = max(0.4f, act->alpha - 2.0f * dt);

    /* 효과 상태 캐릭터에 입히는 부분 */
    if(perfect_collision) {
        a[0] = act->position.x - act->hot_spot.x - diff;
        a[1] = act->position.y - act->hot_spot.y - diff;
        a[2] = a[0] + actor_image(act)->w + 2*diff;
        a[3] = a[1] + actor_image(act)->h + 2*diff;
        for(it = item_list; it != NULL; it = it->next) {
            if(it->data->type == item->type) {
                b[0] = it->data->actor->position.x - it->data->actor->hot_spot.x - diff;
                b[1] = it->data->actor->position.y - it->data->actor->hot_spot.y - diff;
                b[2] = b[0] + actor_image(it->data->actor)->w + 2*diff;
                b[3] = b[1] + actor_image(it->data->actor)->h + 2*diff;
                if(bounding_box(a,b)) {
                    if(it->data->actor->alpha < act->alpha)
                        act->alpha = it->data->actor->alpha;
                    else
                        it->data->actor->alpha = act->alpha;
                }
            }
        }
    }
}

/* dnadoor 캐릭터 모습 */
void dnadoor_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
/*
보통 상자보다 조금 더 크고 충돌이 일어날 시 논리적 상태 출력
*/
int hittest(player_t *player, item_t *dnadoor)
{
    float a[4], b[4];
    int offset = 3;
    actor_t *pl = player->actor;
    actor_t *act = dnadoor->actor;

    a[0] = pl->position.x - pl->hot_spot.x;
    a[1] = pl->position.y - pl->hot_spot.y;
    a[2] = a[0] + actor_image(pl)->w;
    a[3] = a[1] + actor_image(pl)->h;

    b[0] = act->position.x - act->hot_spot.x;
    b[1] = act->position.y - act->hot_spot.y - offset;
    b[2] = b[0] + actor_image(act)->w;
    b[3] = b[1] + actor_image(act)->h + offset;

    return bounding_box(a, b);
}
