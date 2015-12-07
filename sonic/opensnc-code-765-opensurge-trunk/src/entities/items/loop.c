#include "loop.h"
#include "util/itemutil.h"
#include "../../scenes/level.h"
#include "../../core/stringutil.h"
#include "../../core/util.h"

/* loop 클래스 */
typedef struct loop_t loop_t;
struct loop_t {
    item_t item; /* base class */
    char *sprite_name; /* sprite name */
    void (*on_collision)(player_t*); /* strategy pattern */
};

static item_t* loop_create(void (*strategy)(player_t*), const char *sprite_name);
static void loop_init(item_t *item);
static void loop_release(item_t* item);
static void loop_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void loop_render(item_t* item, v2d_t camera_position);

static int is_player_at_closest_loopfloortop(item_t *item, item_list_t *item_list, player_t *player);

static void loopright_strategy(player_t *player);
static void looptop_strategy(player_t *player);
static void loopleft_strategy(player_t *player);
static void loopnone_strategy(player_t *player);
static void loopfloor_strategy(player_t *player);
static void loopfloornone_strategy(player_t *player);
static void loopfloortop_strategy(player_t *player);

/* loopright 객체 생성 */
item_t* loopright_create()
{
    return loop_create(loopright_strategy, "SD_LOOPRIGHT");
}
/* looptop 객체 생성 */
item_t* looptop_create()
{
    return loop_create(looptop_strategy, "SD_LOOPMIDDLE");
}
/* loopleft 객체 생성 */
item_t* loopleft_create()
{
    return loop_create(loopleft_strategy, "SD_LOOPLEFT");
}
/* loopnone 객체 생성 */
item_t* loopnone_create()
{
    return loop_create(loopnone_strategy, "SD_LOOPNONE");
}
/* loopfloor 객체 생성 */
item_t* loopfloor_create()
{
    return loop_create(loopfloor_strategy, "SD_LOOPFLOOR");
}
/* loopfloornone 객체 생성 */
item_t* loopfloornone_create()
{
    return loop_create(loopfloornone_strategy, "SD_LOOPFLOORNONE");
}
/* loopfloortop 객체 생성 */
item_t* loopfloortop_create()
{
    return loop_create(loopfloortop_strategy, "SD_LOOPFLOORTOP");
}

/* loopright: 오른쪽 루프의 입구 */
void loopright_strategy(player_t *player)
{
    player->disable_wall |= PLAYER_WALL_LEFT;
    player->entering_loop = TRUE;
    player->bring_to_back = FALSE;
}

/* looptop: 왼쪽 또는 오른쪽에 벽 생성 */
void looptop_strategy(player_t *player)
{
    if(!player->flying) {
        int b = (player->actor->speed.x > 0);
        player->disable_wall &= ~(PLAYER_WALL_LEFT | PLAYER_WALL_RIGHT);
        player->disable_wall |= b ? PLAYER_WALL_RIGHT : PLAYER_WALL_LEFT;
        player->bring_to_back = b;
    }
}

/* loopleft: 왼쪽 루프의 입구 */
void loopleft_strategy(player_t *player)
{
    player->disable_wall |= PLAYER_WALL_RIGHT;
    player->entering_loop = TRUE;
    player->bring_to_back = TRUE;
}

/* loopnone(x축): 벽을 복원루프로 비활성화 하지 않음 */
void loopnone_strategy(player_t *player)
{
    if(!player->entering_loop) {
        player->disable_wall = PLAYER_WALL_NONE;
        player->bring_to_back = FALSE;
    }
}

/* loopfloor(y축): 아래쪽 루프의 입구 */
void loopfloor_strategy(player_t *player)
{
    if(!player->at_loopfloortop && !player->flying) {
        player->disable_wall |= PLAYER_WALL_BOTTOM;
        player->entering_loop = TRUE;
        player->bring_to_back = TRUE;
    }
}

/* loopfloornone: 복원루프(y축), 바닥 복원 */
void loopfloornone_strategy(player_t *player)
{
    if(!player->at_loopfloortop && !player->entering_loop && !player->flying) {
        player->disable_wall &= ~PLAYER_WALL_BOTTOM;
        player->bring_to_back = FALSE;
    }
}

/* loopfloortop: 왼쪽과 우측벽 활성화 (x축) */
void loopfloortop_strategy(player_t *player)
{
    if(!player->flying) {
        if(player->disable_wall & PLAYER_WALL_BOTTOM) {
            /*  looptop과 같은 행동 */
            int b = (player->actor->speed.x > 0.0f);
            player->disable_wall &= ~(PLAYER_WALL_LEFT | PLAYER_WALL_RIGHT);
            player->disable_wall |= b ? PLAYER_WALL_RIGHT : PLAYER_WALL_LEFT;
            player->bring_to_back = TRUE;
        }
        else {
            /* 좌우 벽을 고정 */
            player->disable_wall &= ~(PLAYER_WALL_LEFT | PLAYER_WALL_RIGHT);
            player->bring_to_back = TRUE;
        }
    }
}

/* loop 객체 생성, 할당 */
item_t* loop_create(void (*strategy)(player_t*), const char *sprite_name)
{
    item_t *item = mallocx(sizeof(loop_t));
    loop_t *me = (loop_t*)item;

    item->init = loop_init;
    item->release = loop_release;
    item->update = loop_update;
    item->render = loop_render;

    me->on_collision = strategy;
    me->sprite_name = str_dup(sprite_name);

    return item;
}

/* loop 생성  */
void loop_init(item_t *item)
{
    loop_t *me = (loop_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = TRUE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation(me->sprite_name, 0));
}


/* loop 없애는 모습 */
void loop_release(item_t* item)
{
    loop_t *me = (loop_t*)item;

    free(me->sprite_name);
    actor_destroy(item->actor);
}


/* loop 특성 생성 */
void loop_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    loop_t *me = (loop_t*)item;
    actor_t *act = item->actor;
    int i;

    act->visible = level_editmode();
    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(actor_collision(act, player->actor)) {
            player->at_loopfloortop = is_player_at_closest_loopfloortop(item, item_list, player);
            me->on_collision(player);
        }
    }
}

/* loop 모습 */
void loop_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
/* 가장 가까운 물체를 플레이어가 항목을 기준으로 도달 했는지 여부 확인 */
int is_player_at_closest_loopfloortop(item_t *item, item_list_t *item_list, player_t *player)
{
    item_t *obj = find_closest_item(item, item_list, IT_LOOPFLOORTOP, NULL);
    return (obj != NULL) ? actor_collision(player->actor, obj->actor) : FALSE;
}
