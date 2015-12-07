#include "itembox.h"
#include "icon.h"
#include "../../scenes/level.h"
#include "../../core/audio.h"
#include "../../core/util.h"
#include "../../core/soundfactory.h"

/* itembox 클래스 */
typedef struct itembox_t itembox_t;
struct itembox_t {
    item_t item; /* base class */
    int anim_id; /* animation id */
    void (*on_destroy)(item_t*,player_t*); /* 전략 패턴 */
};

static item_t* itembox_create(void (*on_destroy)(item_t*,player_t*), int anim_id);
static void itembox_init(item_t *item);
static void itembox_release(item_t* item);
static void itembox_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void itembox_render(item_t* item, v2d_t camera_position);
static int itembox_player_collision(item_t *box, player_t *player);

static void lifebox_strategy(item_t *item, player_t *player);
static void ringbox_strategy(item_t *item, player_t *player);
static void starbox_strategy(item_t *item, player_t *player);
static void speedbox_strategy(item_t *item, player_t *player);
static void glassesbox_strategy(item_t *item, player_t *player);
static void shieldbox_strategy(item_t *item, player_t *player);
static void fireshieldbox_strategy(item_t *item, player_t *player);
static void thundershieldbox_strategy(item_t *item, player_t *player);
static void watershieldbox_strategy(item_t *item, player_t *player);
static void acidshieldbox_strategy(item_t *item, player_t *player);
static void windshieldbox_strategy(item_t *item, player_t *player);
static void trapbox_strategy(item_t *item, player_t *player);
static void emptybox_strategy(item_t *item, player_t *player);
/* 객체들에 대한 캐릭터와 장애물들 호출 */


/* lifebox 객체 생성 */
item_t* lifebox_create()
{
    return itembox_create(lifebox_strategy, 0); /* (strategy, animation id) */
}
/* ringbox 객체 생성 */
item_t* ringbox_create()
{
    return itembox_create(ringbox_strategy, 3);
}
/* starbox 객체 생성 */
item_t* starbox_create()
{
    return itembox_create(starbox_strategy, 4);
}
/* speedbox 객체 생성 */
item_t* speedbox_create()
{
   return itembox_create(speedbox_strategy, 5);
}
/* glassesbox 객체 생성 */
item_t* glassesbox_create()
{
    return itembox_create(glassesbox_strategy, 6);
}
/* shieldbox 객체 생성 */
item_t* shieldbox_create()
{
    return itembox_create(shieldbox_strategy, 7);
}
/* trapbox 객체 생성 */
item_t* trapbox_create()
{
    return itembox_create(trapbox_strategy, 8);
}
/* emptybox 객체 생성 */
item_t* emptybox_create()
{
    return itembox_create(emptybox_strategy, 9);
}
/* fireshieldbox 객체 생성 */
item_t* fireshieldbox_create()
{
    return itembox_create(fireshieldbox_strategy, 11);
}
/* thundershieldbox 객체 생성 */
item_t* thundershieldbox_create()
{
    return itembox_create(thundershieldbox_strategy, 12);
}
/* watershieldbox 객체 생성 */
item_t* watershieldbox_create()
{
    return itembox_create(watershieldbox_strategy, 13);
}
/* acidshieldbox 객체 생성 */
item_t* acidshieldbox_create()
{
    return itembox_create(acidshieldbox_strategy, 14);
}
/* windshieldbox 객체 생성 */
item_t* windshieldbox_create()
{
    return itembox_create(windshieldbox_strategy, 15);
}


/* lifebox_strategy 함수 생성
점수가 100점 일 시 life 1씩 추가 , 소리 생성 */
void lifebox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player_set_lives( player_get_lives()+1 );
    level_override_music( soundfactory_get("1up") );
}
/* ringbox_strategy 함수 생성
점수가 100점 일 시 링 +10 추가, 소리 생성 */
void ringbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player_set_rings( player_get_rings()+10 );
    sound_play( soundfactory_get("ring") );
}
/* starbox_strategy 함수 생성
점수가 100점 일 시 무적 상태, 소리 생성 */
void starbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->invincible = TRUE;
    player->invtimer = 0;
    music_play( music_load("musics/invincible.ogg"), 0 );
}
/* speedbox_strategy 함수 생성
점수가 100점 일 시 캐릭터 속도 증가 신발 생성, 소리 생성 */
void speedbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->got_speedshoes = TRUE;
    player->speedshoes_timer = 0;
    music_play( music_load("musics/speed.ogg"), 0 );
}
/* glassesbox_strategy 함수 생성
점수가 100점 일 시 캐릭터 고글 안경 생성 */
void glassesbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->got_glasses = TRUE;
}
/* shieldbox_strategy 함수 생성
점수가 100점 일 시 캐릭터 보호막 생성, 소리 생성 */
void shieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_SHIELD;
    sound_play( soundfactory_get("shield") );
}
/* fireshieldbox_strategy 함수 생성
점수가 100점 일 시 캐릭터 불꽃 보호막 생성, 소리 생성 */
void fireshieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_FIRESHIELD;
    sound_play( soundfactory_get("fire shield") );
}
/* thundershieldbox_strategy 함수 생성
점수가 100점 일 시 캐릭터 천둥 보호막 생성, 소리 생성 */
void thundershieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_THUNDERSHIELD;
    sound_play( soundfactory_get("thunder shield") );
}
/* watershieldbox_strategy 함수 생성
점수가 100점 일 시 캐릭터 물 보호막 생성, 소리 생성 */
void watershieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_WATERSHIELD;
    sound_play( soundfactory_get("water shield") );
}
/* acidshieldbox_strategy 함수 생성
점수가 100점 일 시 캐릭터 산 보호막 생성, 소리 생성 */
void acidshieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_ACIDSHIELD;
    sound_play( soundfactory_get("acid shield") );
}
/* windshieldbox_strategy 함수 생성
점수가 100점 일 시 캐릭터 바람 보호막 생성, 소리 생성 */
void windshieldbox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
    player->shield_type = SH_WINDSHIELD;
    sound_play( soundfactory_get("wind shield") );
}
/* trapbox_strategy 함수 생성
플레이어 캐릭터 가격 */
void trapbox_strategy(item_t *item, player_t *player)
{
    player_hit(player);
}
/* emptybox_strategy 함수 생성
점수 100점 추가 */
void emptybox_strategy(item_t *item, player_t *player)
{
    level_add_to_score(100);
}


/* itembox 객체 생성 */
item_t* itembox_create(void (*on_destroy)(item_t*,player_t*), int anim_id)
{
    item_t *item = mallocx(sizeof(itembox_t));
    itembox_t *me = (itembox_t*)item;

    item->init = itembox_init;
    item->release = itembox_release;
    item->update = itembox_update;
    item->render = itembox_render;

    me->on_destroy = on_destroy;
    me->anim_id = anim_id;

    return item;
}
/* itembox 생성  */
void itembox_init(item_t *item)
{
    itembox_t *me = (itembox_t*)item;

    item->obstacle = TRUE;
    item->bring_to_back = FALSE;
    item->preserve = TRUE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation("SD_ITEMBOX", me->anim_id));
}
/* itembox 없애는 모습 */
void itembox_release(item_t* item)
{
    actor_destroy(item->actor);
}
/* itembox 특성 생성 */
void itembox_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    int i;
    actor_t *act = item->actor;
    itembox_t *me = (itembox_t*)item;

    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(!(player->actor->is_jumping && player->actor->speed.y < -10)) {
           /* 플레이어 캐릭터가 점프하지 않거나 속도가 감소시 */
            /* 이 item 객체들의 내용들을 파괴하는 속성 */
            if(item->state == IS_IDLE && itembox_player_collision(item, player) && player_attacking(player)) {
                item_t *icon = level_create_item(IT_ICON, v2d_add(act->position, v2d_new(0,-5)));
                icon_change_animation(icon, me->anim_id);
                level_create_item(IT_EXPLOSION, v2d_add(act->position, v2d_new(0,-20)));
                level_create_item(IT_CRUSHEDBOX, act->position);

                sound_play( soundfactory_get("destroy") );
                if(player->actor->is_jumping)
                    player_bounce(player);

                me->on_destroy(item, player);
                item->state = IS_DEAD;
            }

        }
    }

    /* 캐릭터 움직임 */
    me->anim_id = me->anim_id < 3 ? level_player_id() : me->anim_id;
    actor_change_animation(item->actor, sprite_get_animation("SD_ITEMBOX", me->anim_id));
}
/* itembox 모습 */
void itembox_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}

/* 플레이어가 상자 와 충돌 하는 경우 TRUE를 반환 ,
또는 그렇지 않으면 false를 반환합니다
그리고 가짜 벽돌이 생성되어 박스가 작동하지 않습니다 */
int itembox_player_collision(item_t *box, player_t *player)
{
    /* 플레이어 캐릭터가 충돌을 처리하기위해 아이템 상자,
    상자들을 조절해야 합니다 가짜벽돌이 상단에 생성 */
    int collided;
    actor_t *act = box->actor;
    v2d_t oldpos = act->position;


    act->position.y -= 5; /* 점프시 수정 */
    if(player->spin) { /* 여러 상자들을 돌아가게 함 */
        if(player->actor->position.x < act->position.x && player->actor->speed.x > 0)
            act->position.x -= 15;
        else if(player->actor->position.x > act->position.x && player->actor->speed.x < 0)
            act->position.x += 15;
    }

    /* 충돌 감지 */
    collided = actor_collision(box->actor, player->actor);


    box->actor->position = oldpos;


    return collided;
}
