#include "teleporter.h"
#include "../../core/util.h"
#include "../../core/input.h"
#include "../../core/audio.h"
#include "../../core/timer.h"
#include "../../core/soundfactory.h"
#include "../../scenes/level.h"

/* teleporter 클래스 */
typedef struct teleporter_t teleporter_t;
struct teleporter_t {
    item_t item; /* base class */
    int is_disabled; /* teleporter 사용할 수 없는지 여부 변수 */
    int is_active; /* 이 개체가 팀을 순간이동 시키는지 여부 변수 */
    float timer; /* time counter */
    player_t *who; /* who has activated me? */
};

static void teleporter_init(item_t *item);
static void teleporter_release(item_t* item);
static void teleporter_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void teleporter_render(item_t* item, v2d_t camera_position);

static void teleport_player_to(player_t *player, v2d_t position);



/* teleporter 객체 생성, 할당 */
item_t* teleporter_create()
{
    item_t *item = mallocx(sizeof(teleporter_t));

    item->init = teleporter_init;
    item->release = teleporter_release;
    item->update = teleporter_update;
    item->render = teleporter_render;

    return item;
}
/* teleporter  활성화 함수
활성 가능시 TRUE 값 , 카메라 위치 조정, 사운드 생성*/
void teleporter_activate(item_t *teleporter, player_t *who)
{
    teleporter_t *me = (teleporter_t*)teleporter;
    actor_t *act = teleporter->actor;

    if(!me->is_active && !me->is_disabled) {
        me->is_active = TRUE;
        me->who = who;

        input_ignore(who->actor->input);
        level_set_camera_focus(act);
        sound_play( soundfactory_get("teleporter") );
    }
}

/* teleporter 생성  */
void teleporter_init(item_t *item)
{
    teleporter_t *me = (teleporter_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    me->is_disabled = FALSE;
    me->is_active = FALSE;
    me->timer = 0.0f;

    actor_change_animation(item->actor, sprite_get_animation("SD_TELEPORTER", 0));
}


/* teleporter 없애는 모습 */
void teleporter_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* teleporter 특성 생성 */
void teleporter_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    teleporter_t *me = (teleporter_t*)item;
    actor_t *act = item->actor;
    float dt = timer_get_delta();
    int i, k=0;

    if(me->is_active) {
        me->timer += dt;
        if(me->timer >= 3.0f) {
            /* 플레이어 팀원 모드 텔레포트 */
            player_t *who = me->who; /* 어느 플레이어가 텔레포트를 활성화 하였는지 */

            input_restore(who->actor->input);
            level_set_camera_focus(who->actor);

            for(i=0; i<team_size; i++) {
                player_t *player = team[i];
                if(player != who) {
                    v2d_t position = v2d_add(act->position, v2d_new(-20 + 40*(k++), -30));
                    teleport_player_to(player, position);
                }
            }

            me->is_active = FALSE;
            me->is_disabled = TRUE; /* 텔레 포터는 한번만 작동 */
        }
        else {
            ; /* 플레이어들은 조금 기다리게 되는 상태, 텔레포트 되는 중간 상태 */
        }

        actor_change_animation(act, sprite_get_animation("SD_TELEPORTER", 1));
    }
    else
        actor_change_animation(act, sprite_get_animation("SD_TELEPORTER", 0));
}

/* teleporter 모습 */
void teleporter_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}

/* 지정된 위치 에 플레이어를 텔레포트 */
void teleport_player_to(player_t *player, v2d_t position)
{
    player->actor->position = position;
    player->actor->speed = v2d_new(0,0);
    player->actor->is_jumping = FALSE;
    player->flying = FALSE;
    player->climbing = FALSE;
    player->getting_hit = FALSE;
    player->spring = FALSE;
    player->actor->angle = 0;
    player->disable_wall = PLAYER_WALL_NONE;
    player->entering_loop = FALSE;
    player->at_loopfloortop = FALSE;
    player->bring_to_back = FALSE;
}
