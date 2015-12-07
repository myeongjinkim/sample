#include "icon.h"
#include "../../core/util.h"
#include "../../core/timer.h"
#include "../../scenes/level.h"

/* icon 클래스 */
typedef struct icon_t icon_t;
struct icon_t {
    item_t item; /* base class */
    float elapsed_time; /* 이 객체가 생성된 후 경과 시간 */
};

static void icon_init(item_t *item);
static void icon_release(item_t* item);
static void icon_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void icon_render(item_t* item, v2d_t camera_position);



/* icon 객체 생성 */
item_t* icon_create()
{
    item_t *item = mallocx(sizeof(icon_t));

    item->init = icon_init;
    item->release = icon_release;
    item->update = icon_update;
    item->render = icon_render;

    return item;
}
/* icon 변화 모습 */
void icon_change_animation(item_t *item, int anim_id)
{
    actor_change_animation(item->actor, sprite_get_animation("SD_ICON", anim_id));
}


/* icon  생성 */
void icon_init(item_t *item)
{
    icon_t *me = (icon_t*)item;

    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = FALSE;
    item->actor = actor_create();

    me->elapsed_time = 0.0f;
    icon_change_animation(item, 0);
}


/* icon 없애는 모습 */
void icon_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* icon 특성 생성 */
void icon_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    icon_t *me = (icon_t*)item;
    actor_t *act = item->actor;
    float dt = timer_get_delta();

    me->elapsed_time += dt;
    if(me->elapsed_time < 1.0f) {
        /* 캐릭터의 위치 변화*/
        act->position.y -= 40.0f * dt;
    }
    else if(me->elapsed_time >= 2.5f) {
        /*캐릭터가 죽었을 시 모습*/
        int i, j;
        int x = (int)(act->position.x-act->hot_spot.x);
        int y = (int)(act->position.y-act->hot_spot.y);
        image_t *img = actor_image(act), *particle;

        /* 이미지 생성, 삭제 */
        for(i=0; i<img->h; i++) {
            for(j=0; j<img->w; j++) {
                particle = image_create(1,1);
                image_clear(particle, image_getpixel(img, j, i));
                level_create_particle(particle, v2d_new(x+j, y+i), v2d_new((j-img->w/2) + (random(img->w)-img->w/2), i-random(img->h/2)), FALSE);
            }
        }

        item->state = IS_DEAD;
    }
}

/* icon 모습 */
void icon_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
