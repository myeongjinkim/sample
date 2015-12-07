
#include "gravity.h"
#include "../../core/util.h"
#include "../../core/timer.h"
#include "../../core/video.h"
#include "../../scenes/level.h"

/* objectdecorator_gravity_t 클래스 */
typedef struct objectdecorator_gravity_t objectdecorator_gravity_t;
struct objectdecorator_gravity_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);

static int hit_test(int x, int y, const image_t *brk_image, int brk_x, int brk_y);
static int sticky_test(const actor_t *act, const brick_list_t *brick_list);


/* public methods */

/* class constructor */
/* class 구조 구성, 할당 */
objectmachine_t* objectdecorator_gravity_new(objectmachine_t *decorated_machine)
{
    objectdecorator_gravity_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;

    return obj;
}




/* private methods */
/* objectmachine_t 상속 받아서 생성 */
void init(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->init(decorated_machine);
}
/* objectmachine_t 해제 */
void release(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->release(decorated_machine);
    free(obj);
}
/* objectmachine_t  변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    object_t *object = obj->get_object_instance(obj);
    actor_t *act = object->actor;
    float dt = timer_get_delta();

    /* --------------------------- */

    /*  프로세서 부하를 피하기 위해,
          이 단순화 된 플랫폼 시스템을 채택 */
    int rx, ry, rw, rh, bx, by, bw, bh, j;
    const image_t *ri, *bi;
    brick_list_t *it;
    enum { NONE, FLOOR, CEILING } collided = NONE;
    int i, sticky_max_offset = 3;

    ri = actor_image(act);
    rx = (int)(act->position.x - act->hot_spot.x);
    ry = (int)(act->position.y - act->hot_spot.y);
    rw = image_width(ri);
    rh = image_height(ri);

    /* 충돌 체크 */
    for(it = brick_list; it != NULL && collided == NONE; it = it->next) {
        if(it->data->brick_ref->property != BRK_NONE) {
            bi = it->data->brick_ref->image;
            bx = it->data->x;
            by = it->data->y;
            bw = image_width(bi);
            bh = image_height(bi);

            if(rx<bx+bw && rx+rw>bx && ry<by+bh && ry+rh>by) {
                if(image_pixelperfect_collision(ri, bi, rx, ry, bx, by)) {
                    if(hit_test(rx+rw/2, ry, bi, bx, by)) {
                        /* ceiling */
                        collided = CEILING;
                        for(j=1; j<=bh; j++) {
                            if(!image_pixelperfect_collision(ri, bi, rx, ry+j, bx, by)) {
                                act->position.y += j-1;
                                break;
                            }
                        }
                    }
                    else if(hit_test(rx+rw/2, ry+rh-1, bi, bx, by)) {
                        /* floor */
                        collided = FLOOR;
                        for(j=1; j<=bh && hit_test(rx+rw/2, ry+rh-j, bi, bx, by); j++)
                            act->position.y -= 1;
                        if(j > 1) act->position.y += 1;
                    }
                }
            }
        }
    }

    /* 충돌과 중력 */
    switch(collided) {
        case FLOOR:
            if(act->speed.y > 0.0f)
                act->speed.y = 0.0f;
            break;

        case CEILING:
            if(act->speed.y < 0.0f)
                act->speed.y = 0.0f;
            break;

        default:
            act->speed.y += (0.21875f * 60.0f * 60.0f) * dt;
            break;
    }

    /* 움직임 */
    act->position.y += act->speed.y * dt;

    /* 물리학 */
    if(!sticky_test(act, brick_list)) {
        for(i=sticky_max_offset; i>0; i--) {
            act->position.y += i;
            if(!sticky_test(act, brick_list)) {
                act->position.y += (i == sticky_max_offset) ? -i : 1;
                break;
            }
            else
                act->position.y -= i;
        }
    }

    /* --------------------------- */

    decorated_machine->update(decorated_machine, team, team_size, brick_list, item_list, object_list);
}
/*  변화에 대한 모습 */
void render(objectmachine_t *obj, v2d_t camera_position)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    ; /* empty */

    decorated_machine->render(decorated_machine, camera_position);
}




/* (x,y) 벽돌과 충돌 */
int hit_test(int x, int y, const image_t *brk_image, int brk_x, int brk_y)
{
    if(x >= brk_x && x < brk_x + image_width(brk_image) && y >= brk_y && y < brk_y + image_height(brk_image))
        return image_getpixel(brk_image, x - brk_x, y - brk_y) != video_get_maskcolor();

    return FALSE;
}

/* 벽돌과 충돌함으로 인한 행동 */
int sticky_test(const actor_t *act, const brick_list_t *brick_list)
{
    const brick_list_t *it;
    const brick_t *b;
    const image_t *ri;
    int rx, ry, rw, rh;

    ri = actor_image(act);
    rx = (int)(act->position.x - act->hot_spot.x);
    ry = (int)(act->position.y - act->hot_spot.y);
    rw = image_width(ri);
    rh = image_height(ri);

    for(it = brick_list; it; it = it->next) {
        b = it->data;
        if(b->brick_ref->property != BRK_NONE) {
            if(hit_test(rx+rw/2, ry+rh-1, brick_image(b), b->x, b->y))
                return TRUE;
        }
    }

    return FALSE;
}
