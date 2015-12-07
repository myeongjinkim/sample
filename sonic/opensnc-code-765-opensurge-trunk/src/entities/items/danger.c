#include "danger.h"
#include "../../core/util.h"
#include "../../core/stringutil.h"
#include "../../scenes/level.h"

/* danger 클래스 */
typedef struct danger_t danger_t;
struct danger_t {
    item_t item; /* base class */
    char *sprite_name;
    int (*player_is_vulnerable)(player_t*);
};

static item_t* danger_create(const char *sprite_name, int (*player_is_vulnerable)(player_t*));
static void danger_init(item_t *item);
static void danger_release(item_t* item);
static void danger_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void danger_render(item_t* item, v2d_t camera_position);

static int always_vulnerable(player_t *player);
static int can_defend_against_fire(player_t *player);


/* 위험 경고 알려주는 경계수평선 반환 */
item_t* horizontaldanger_create()
{
    return danger_create("SD_DANGER", always_vulnerable);
}
/* 위험 경고 알려주는 경계수직선 반환 */
item_t* verticaldanger_create()
{
    return danger_create("SD_VERTICALDANGER", always_vulnerable);
}
/* 위험 경고 알려주는 경계수평선 반환 */
item_t* horizontalfiredanger_create()
{
    return danger_create("SD_FIREDANGER", can_defend_against_fire);
}
/* 위험 경고 알려주는 경계수직선 반환 */
item_t* verticalfiredanger_create()
{
    return danger_create("SD_VERTICALFIREDANGER", can_defend_against_fire);
}


/* 위험신호 객체 생성 */
item_t* danger_create(const char *sprite_name, int (*player_is_vulnerable)(player_t*))
{
    item_t *item = mallocx(sizeof(danger_t));
    danger_t *me = (danger_t*)item;

    item->init = danger_init;
    item->release = danger_release;
    item->update = danger_update;
    item->render = danger_render;

    me->sprite_name = str_dup(sprite_name);
    me->player_is_vulnerable = player_is_vulnerable;

    return item;
}
/* 위험신호 생성 */
void danger_init(item_t *item)
{
    danger_t *me = (danger_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation(me->sprite_name, 0));
}


/* 위험신호 삭제 움직임이 자유로워짐 */
void danger_release(item_t* item)
{
    danger_t *me = (danger_t*)item;

    actor_destroy(item->actor);
    free(me->sprite_name);
}


/* 위험신호 모습*/
void danger_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    int i;
    danger_t *me = (danger_t*)item;
    actor_t *act = item->actor;

    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(!player->dying && !player->blinking && !player->invincible && actor_collision(act, player->actor)) {
            if(me->player_is_vulnerable(player))
                player_hit(player);
        }
    }

    act->visible = level_editmode();
}

/* 위험신호 나타내는 모습 */
void danger_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}

/* 캐릭터가 상처입음을 표시 반환 */
int always_vulnerable(player_t *player)
{
    return TRUE;
}
/* 캐릭터의 방어막 생성 반환 */
int can_defend_against_fire(player_t *player)
{
    return (player->shield_type != SH_FIRESHIELD);
}
