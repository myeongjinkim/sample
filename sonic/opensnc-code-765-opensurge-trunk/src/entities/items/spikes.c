#include "spikes.h"
#include "../../core/util.h"
#include "../../core/stringutil.h"
#include "../../core/audio.h"
#include "../../core/timer.h"
#include "../../core/soundfactory.h"
#include "../player.h"
#include "../enemy.h"

/* spikes 클래스 */
typedef struct spikes_t spikes_t;
struct spikes_t {
    item_t item; /* base class */
    int (*collision)(item_t*,player_t*); /* strategy pattern */
    int anim_id; /* animation number */
    float timer; /* periodic spikes */
    float cycle_length; /* periodic spikes */
    int hidden; /* 모든 cycle 표현 */
};

static item_t* spikes_create(int (*collision)(item_t*,player_t*), int anim_id, float cycle_length);
static void spikes_init(item_t *item);
static void spikes_release(item_t* item);
static void spikes_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void spikes_render(item_t* item, v2d_t camera_position);

static int hittest(player_t *player, float rect[4]);

static int floor_strategy(item_t *spikes, player_t *player);
static int ceiling_strategy(item_t *spikes, player_t *player);
static int leftwall_strategy(item_t *spikes, player_t *player);
static int rightwall_strategy(item_t *spikes, player_t *player);



/* floorspikes 객체 생성 */
item_t* floorspikes_create()
{
    return spikes_create(floor_strategy, 0, INFINITY_FLT);
}
/* ceilingspikes 객체 생성 */
item_t* ceilingspikes_create()
{
    return spikes_create(ceiling_strategy, 2, INFINITY_FLT);
}
/* leftwallspikes 객체 생성 */
item_t* leftwallspikes_create()
{
    return spikes_create(leftwall_strategy, 1, INFINITY_FLT);
}
/* rightwallspikes 객체 생성 */
item_t* rightwallspikes_create()
{
    return spikes_create(rightwall_strategy, 3, INFINITY_FLT);
}
/* periodic_floorspikes 객체 생성 */
item_t* periodic_floorspikes_create()
{
    return spikes_create(floor_strategy, 0, 5.0f);
}
/* periodic_ceilingspikes 객체 생성 */
item_t* periodic_ceilingspikes_create()
{
    return spikes_create(ceiling_strategy, 2, 5.0f);
}
/* periodic_leftwallspikes 객체 생성 */
item_t* periodic_leftwallspikes_create()
{
    return spikes_create(leftwall_strategy, 1, 5.0f);
}
/* periodic_rightwallspikes 객체 생성 */
item_t* periodic_rightwallspikes_create()
{
    return spikes_create(rightwall_strategy, 3, 5.0f);
}

/* spikes 객체 생성, 할당 */
item_t* spikes_create(int (*collision)(item_t*,player_t*), int anim_id, float cycle_length)
{
    item_t *item = mallocx(sizeof(spikes_t));
    spikes_t *me = (spikes_t*)item;

    item->init = spikes_init;
    item->release = spikes_release;
    item->update = spikes_update;
    item->render = spikes_render;

    me->collision = collision;
    me->anim_id = anim_id;
    me->cycle_length = cycle_length;

    return item;
}
/* spikes 생성  */
void spikes_init(item_t *item)
{
    spikes_t *me = (spikes_t*)item;

    item->obstacle = TRUE;
    item->bring_to_back = TRUE;
    item->preserve = TRUE;
    item->actor = actor_create();

    me->timer = 0.0f;
    me->hidden = FALSE;

    actor_change_animation(item->actor, sprite_get_animation("SD_SPIKES", me->anim_id));
}


/* spikes 없애는 모습 */
void spikes_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* spikes 특성 생성 */

void spikes_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    spikes_t *me = (spikes_t*)item;
    float dt = timer_get_delta();

    /* 상태 변화 */
    me->timer += dt;
    if(me->timer >= me->cycle_length * 0.5f) {
        me->timer = 0.0f;
        me->hidden = !me->hidden;
        sound_play(
            soundfactory_get(
                me->hidden ? "spikes disappearing" : "spikes appearing"
            )
        );
    }
    item->obstacle = !me->hidden;
    item->actor->visible = !me->hidden;

    /* spike 충돌 */
    if(!me->hidden) {
        int i;

        for(i=0; i<team_size; i++) {
            player_t *player = team[i];
            if(!player->dying && !player->blinking && !player->invincible) {
                if(me->collision(item, player)) {
                    sound_t *s = soundfactory_get("spikes hit");
                    if(!sound_is_playing(s))
                        sound_play(s);
                    player_hit(player);
                }
            }
        }
    }
}

/* spikes 모습 */
void spikes_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}

/* private strategies
spike에 캐릭터가 닿았을 시 캐릭터 움직임에 변화,
이미지 깜빡임 변화 */
int floor_strategy(item_t *spikes, player_t *player)
{
    float b[4];
    float feet;
    actor_t *act = spikes->actor;

    b[0] = act->position.x - act->hot_spot.x + 5;
    b[1] = act->position.y - act->hot_spot.y - 5;
    b[2] = b[0] + actor_image(act)->w - 10;
    b[3] = b[1] + 10;

    feet = player->actor->position.y - player->actor->hot_spot.y + actor_image(player->actor)->h;
    return hittest(player, b) && feet < (act->position.y - act->hot_spot.y + actor_image(act)->h/2);
}
/* private strategies
천장 벽에 달린 spike에 캐릭터가 닿았을 시 캐릭터 움직임에 변화,
이미지 깜빡임 변화 */
int ceiling_strategy(item_t *spikes, player_t *player)
{
    float b[4];
    actor_t *act = spikes->actor;

    b[0] = act->position.x - act->hot_spot.x + 5;
    b[1] = act->position.y - act->hot_spot.y + actor_image(act)->h - 5;
    b[2] = b[0] + actor_image(act)->w - 10;
    b[3] = b[1] + 10;

    return hittest(player, b);
}
/* private strategies
왼쪽 벽에 달린 spike에 캐릭터가 닿았을 시 캐릭터 움직임에 변화,
이미지 깜빡임 변화 */
int leftwall_strategy(item_t *spikes, player_t *player)
{
    float b[4];
    actor_t *act = spikes->actor;

    b[0] = act->position.x - act->hot_spot.x + actor_image(act)->w - 5;
    b[1] = act->position.y - act->hot_spot.y + 5;
    b[2] = b[0] + 10;
    b[3] = b[1] + actor_image(act)->h - 10;

    return hittest(player, b);
}
/* private strategies
오른쪽 벽에 달린 spike에 캐릭터가 닿았을 시 캐릭터 움직임에 변화,
이미지 깜빡임 변화 */
int rightwall_strategy(item_t *spikes, player_t *player)
{
    float b[4];
    actor_t *act = spikes->actor;

    b[0] = act->position.x - act->hot_spot.x - 5;
    b[1] = act->position.y - act->hot_spot.y + 5;
    b[2] = b[0] + 10;
    b[3] = b[1] + actor_image(act)->h - 10;

    return hittest(player, b);
}

/* r플레이어가 주어진 사각형 과 충돌 하는 경우 true를 반환 */
int hittest(player_t *player, float rect[4])
{
    float a[4];
    actor_t *pl = player->actor;

    a[0] = pl->position.x - pl->hot_spot.x;
    a[1] = pl->position.y - pl->hot_spot.y;
    a[2] = a[0] + actor_image(pl)->w;
    a[3] = a[1] + actor_image(pl)->h;

    return bounding_box(a, rect);
}
