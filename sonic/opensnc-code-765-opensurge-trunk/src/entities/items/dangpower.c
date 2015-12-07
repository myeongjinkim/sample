#include "dangpower.h"
#include "../../core/util.h"
#include "../../core/timer.h"
#include "../../core/soundfactory.h"
#include "../../scenes/level.h"

/* dangerouspower 클래스 */
typedef struct dangerouspower_t dangerouspower_t;
struct dangerouspower_t {
    item_t item; /* base class */
};

static void dangerouspower_init(item_t *item);
static void dangerouspower_release(item_t* item);
static void dangerouspower_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list);
static void dangerouspower_render(item_t* item, v2d_t camera_position);



/* 위험 상태 객체 생성 */
item_t* dangerouspower_create()
{
    item_t *item = mallocx(sizeof(dangerouspower_t));

    item->init = dangerouspower_init;
    item->release = dangerouspower_release;
    item->update = dangerouspower_update;
    item->render = dangerouspower_render;

    return item;
}
/* 위험 상태 속도 값 */
void dangerouspower_set_speed(item_t *dangpower, v2d_t speed)
{
    dangpower->actor->speed = speed;
}


/* 위험 상태 생성 */
void dangerouspower_init(item_t *item)
{
    item->obstacle = FALSE;
    item->bring_to_back = FALSE;
    item->preserve = FALSE;
    item->actor = actor_create();

    actor_change_animation(item->actor, sprite_get_animation("SD_DANGPOWER", 0));
}


/* 위험 상태 삭제 */
void dangerouspower_release(item_t* item)
{
    actor_destroy(item->actor);
}


/* 위험 상태 모습 효과*/
void dangerouspower_update(item_t* item, player_t** team, int team_size, brick_list_t* brick_list, item_list_t* item_list, enemy_list_t* enemy_list)
{
    int i;
    float sqrsize = 2, diff = -2;
    float dt = timer_get_delta();
    actor_t *act = item->actor;
    v2d_t ds = v2d_multiply(act->speed, dt);
    brick_t *bu, *bd, *bl, *br, *brk = NULL;

    /* 멈춤 상태 */
    if(level_editmode())
        return;

    /* 캐릭터를 공격 */
    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(!player->dying && actor_collision(act, player->actor)) {
            player_hit(player);
            item->state = IS_DEAD;
        }
    }

    /* 벽돌 공격 */
    actor_corners(act, sqrsize, diff, brick_list, &bu, NULL, &br, NULL, &bd, NULL, &bl, NULL);
    actor_handle_clouds(act, diff, &bu, NULL, &br, NULL, &bd, NULL, &bl, NULL);
    if( NULL != (brk = (bd ? bd : (br ? br : (bl ? bl : (bu ? bu : NULL))))) ) {
        /* destroy the brick */
        if(brk->brick_ref->angle == 0 && brk->y >= act->spawn_point.y+70) {
            image_t *brkimg = brk->brick_ref->image;
            int bw=brkimg->w/5, bh=brkimg->h/5, bi, bj;

            /* 공격 입자 */
            for(bi=0; bi<bw; bi++) {
                for(bj=0; bj<bh; bj++) {
                    v2d_t piecepos = v2d_new(brk->x + (bi*brkimg->w)/bw, brk->y + (bj*brkimg->h)/bh);
                    v2d_t piecespeed = v2d_new(-40+random(80), -70-random(70));
                    image_t *piece = image_create(brkimg->w/bw, brkimg->h/bh);

                    image_blit(brkimg, piece, (bi*brkimg->w)/bw, (bj*brkimg->h)/bh, 0, 0, piece->w, piece->h);
                    level_create_particle(piece, piecepos, piecespeed, FALSE);
                }
            }

            /* 깨지는 상태 */
            sound_play( soundfactory_get("break") );
            brk->state = BRS_DEAD;
        }

        /* 상태 능력 파괴 */
        item->state = IS_DEAD;
    }

    /* 움직임 */
    act->position = v2d_add(act->position, ds);
}

/* 위험 상태일 때 캐릭터 모습 */
void dangerouspower_render(item_t* item, v2d_t camera_position)
{
    actor_render(item->actor, camera_position);
}
