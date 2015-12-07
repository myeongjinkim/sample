#include "switch.h"
#include "door.h"
#include "teleporter.h"
#include "util/itemutil.h"
#include "../../core/v2d.h"
#include "../../core/util.h"
#include "../../core/audio.h"
#include "../../core/video.h"
#include "../../core/image.h"
#include "../../core/soundfactory.h"
#include "../../scenes/level.h"

/* switch 클래스 */
typedef struct switch_t switch_t;
struct switch_t {
    item_t item; /* base class */
    int is_pressed; /* 스위치를 누르고 있는지 여부를 저장하는 변수 */
    item_t *partner; /* 객체와 플레이어 캐릭터가 결합하고 있는지 (ex NULL, 문, 텔레포트 등) */
};

static void switch_init(item_t *item);
static void switch_release(item_t* item);
static void switch_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void switch_render(item_t* item, v2d_t camera_position);

static int pressed_the_switch(item_t *item, player_t *player);
static void handle_logic(item_t *item, item_t *other, player_t **team, int team_size, void (*stepin)(item_t*,player_t*), void (*stepout)(item_t*));

static void stepin_nothing(item_t *door, player_t *who);
static void stepout_nothing(item_t *door);
static void stepin_door(item_t *door, player_t *who);
static void stepout_door(item_t *door);
static void stepin_teleporter(item_t *teleporter, player_t *who);
static void stepout_teleporter(item_t *teleporter);



/* public methods */
/* switch 객체 생성 */
item_t* switch_create()
{
    item_t *item = mallocx(sizeof(switch_t));

    item->init = switch_init;
    item->release = switch_release;
    item->update = switch_update;
    item->render = switch_render;

    return item;
}


/* switch 생성  */
void switch_init(item_t *item)
{
    switch_t *me = (switch_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    me->is_pressed = FALSE;
    me->partner = NULL;
    actor_change_animation(item->actor, sprite_get_animation("SD_SWITCH", 0));
}


/* switch 없애는 모습 */
void switch_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* switch 특성 생성 */
void switch_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    switch_t *me = (switch_t*)item;
    item_t *door, *teleporter;
    float d1, d2;

    /* 캐릭터 파트너는 없다 */
    me->partner = NULL;

    /* 누가 내 파트너인지 알아내는 요소 */
    door = find_closest_item(item, item_list, IT_DOOR, &d1);
    teleporter = find_closest_item(item, item_list, IT_TELEPORTER, &d2);
    if(door != NULL && d1 < d2)
        me->partner = door;
    if(teleporter != NULL && d2 < d1)
        me->partner = teleporter;

    /* logic을 처리합니다. 플레이어 캐릭터 파트너가 누가 되는지에 의존하는 논리 */
    if(me->partner == NULL)
        handle_logic(item, door, team, team_size, stepin_nothing, stepout_nothing);
    else if(me->partner == door)
        handle_logic(item, door, team, team_size, stepin_door, stepout_door);
    else if(me->partner == teleporter)
        handle_logic(item, teleporter, team, team_size, stepin_teleporter, stepout_teleporter);
}

/* switch 모습 */
void switch_render(item_t* item, v2d_t camera_position)
{
    switch_t *me = (switch_t*)item;
    /* level 편집 모드에서 파트너가 있을경우 */
    if(level_editmode() && me->partner != NULL) {
        v2d_t p1, p2, offset;
        offset = v2d_subtract(camera_position, v2d_new(VIDEO_SCREEN_W/2, VIDEO_SCREEN_H/2));
        p1 = v2d_subtract(item->actor->position, offset);
        p2 = v2d_subtract(me->partner->actor->position, offset);
        image_line(video_get_backbuffer(), (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, image_rgb(255, 0, 0));
    }

    actor_render(item->actor, camera_position);
}



/* logic을 다루는 함수 */
void handle_logic(item_t *item, item_t *other, player_t **team, int team_size, void (*stepin)(item_t*,player_t*), void (*stepout)(item_t*))
{
    int i;
    int nobody_is_pressing_me = TRUE;
    switch_t *me = (switch_t*)item;
    actor_t *act = item->actor;

    /* switch 압력 여부에 따라 소리 생성, 화면 움직임 변화 */
    for(i=0; i<team_size; i++) {
        player_t *player = team[i];

        if(pressed_the_switch(item, player)) {
            nobody_is_pressing_me = FALSE;
            if(!me->is_pressed) {
                stepin(other, player);
                sound_play( soundfactory_get("switch") );
                actor_change_animation(act, sprite_get_animation("SD_SWITCH", 1));
                me->is_pressed = TRUE;
            }
        }
    }

    /* step에서 나왔을 때 */
    if(nobody_is_pressing_me) {
        if(me->is_pressed) {
            stepout(other);
            actor_change_animation(act, sprite_get_animation("SD_SWITCH", 0));
            me->is_pressed = FALSE;
        }
    }
}
/* stepin 아무것도 없을 때 */
void stepin_nothing(item_t *door, player_t *who)
{
    ; /* empty */
}
/* stepout 아무것도 없을 때 */
void stepout_nothing(item_t *door)
{
    ; /* empty */
}
/* stepin 문 열림 */
void stepin_door(item_t *door, player_t *who)
{
    door_open(door);
}
/* stepout 문 닫힘 */
void stepout_door(item_t *door)
{
    door_close(door);
}
/* stepin 텔리포터 활동 */
void stepin_teleporter(item_t *teleporter, player_t *who)
{
    teleporter_activate(teleporter, who);
}
/* stepout 텔레포터 빈 여백*/
void stepout_teleporter(item_t *teleporter)
{
    ; /* empty */
}

/* 플레이어가 스위치 (항목 )를 누를 경우 true를 반환 */
int pressed_the_switch(item_t *item, player_t *player)
{
    float a[4], b[4];

    a[0] = item->actor->position.x - item->actor->hot_spot.x;
    a[1] = item->actor->position.y - item->actor->hot_spot.y;
    a[2] = a[0] + actor_image(item->actor)->w;
    a[3] = a[1] + actor_image(item->actor)->h;

    b[0] = player->actor->position.x - player->actor->hot_spot.x + actor_image(player->actor)->w * 0.3;
    b[1] = player->actor->position.y - player->actor->hot_spot.y + actor_image(player->actor)->h * 0.5;
    b[2] = b[0] + actor_image(player->actor)->w * 0.4;
    b[3] = b[1] + actor_image(player->actor)->h * 0.5;

    return (!player->dying && !player->climbing && !player->flying && bounding_box(a,b));
}
