#include "flyingtext.h"
#include "../../core/util.h"
#include "../../core/timer.h"
#include "../font.h"
#include "../player.h"
#include "../enemy.h"

/* flyingtext 클래스 */
typedef struct flyingtext_t flyingtext_t;
struct flyingtext_t {
    item_t item; /* base class */
    font_t *font;
    float elapsed_time;
};

static void flyingtext_init(item_t *item);
static void flyingtext_release(item_t* item);
static void flyingtext_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void flyingtext_render(item_t* item, v2d_t camera_position);



/* flyingtext 객체 생성, 할당 */
item_t* flyingtext_create()
{
    item_t *item = mallocx(sizeof(flyingtext_t));

    item->init = flyingtext_init;
    item->release = flyingtext_release;
    item->update = flyingtext_update;
    item->render = flyingtext_render;

    return item;
}
/* text 설정 */
void flyingtext_set_text(item_t *item, const char *text)
{
    flyingtext_t *me = (flyingtext_t*)item;
    font_set_text(me->font, text);
}


/* flyingtext 생성  */
void flyingtext_init(item_t *item)
{
    flyingtext_t *me = (flyingtext_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = FALSE;
    item->actor = actor_create();

    me->elapsed_time = 0.0f;
    me->font = font_create(0);
    font_set_text(me->font, "0");

    actor_change_animation(item->actor, sprite_get_animation("SD_RING", 0));
    item->actor->visible = FALSE;
}
/* flyingtext 없애는 모습 */
void flyingtext_release(item_t* item)
{
    flyingtext_t *me = (flyingtext_t*)item;

    actor_destroy(item->actor);
    font_destroy(me->font);
}


/* flyingtext 특성 생성 */
void flyingtext_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    flyingtext_t *me = (flyingtext_t*)item;
    float dt = timer_get_delta();

    me->elapsed_time += dt;
    if(me->elapsed_time < 0.5f)
        item->actor->position.y -= 100.0f * dt;
    else if(me->elapsed_time > 2.0f)
        item->state = IS_DEAD;
    me->font->position = item->actor->position;
}

/* flyingtext 모습 */
void flyingtext_render(item_t* item, v2d_t camera_position)
{
    flyingtext_t *me = (flyingtext_t*)item;
    font_render(me->font, camera_position);
}
