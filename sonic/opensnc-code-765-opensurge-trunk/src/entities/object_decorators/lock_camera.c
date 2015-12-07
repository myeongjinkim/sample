

#include <math.h>
#include "lock_camera.h"
#include "../../core/util.h"
#include "../../core/image.h"
#include "../../core/video.h"
#include "../../scenes/level.h"
#include "../../entities/player.h"

/* objectdecorator_lockcamera_t 클래스 */
typedef struct objectdecorator_lockcamera_t objectdecorator_lockcamera_t;
struct objectdecorator_lockcamera_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    expression_t *x1, *y1, *x2, *y2;
    int has_locked_somebody;
    int _x1, _y1, _x2, _y2;
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);

static void get_rectangle_coordinates(objectdecorator_lockcamera_t *me, int *x1, int *y1, int *x2, int *y2);
static void update_rectangle_coordinates(objectdecorator_lockcamera_t *me, int x1, int y1, int x2, int y2);


/* public methods */

/* class constructor */
/* class 구조 구성, 할당 */
objectmachine_t* objectdecorator_lockcamera_new(objectmachine_t *decorated_machine, expression_t *x1, expression_t *y1, expression_t *x2, expression_t *y2)
{
    objectdecorator_lockcamera_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;

    me->x1 = x1;
    me->y1 = y1;
    me->x2 = x2;
    me->y2 = y2;

    return obj;
}





/* private methods */
/* objectmachine_t 상속 받아서 생성 */
void init(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_lockcamera_t *me = (objectdecorator_lockcamera_t*)obj;
    int x1, x2, y1, y2;

    me->has_locked_somebody = FALSE;
    get_rectangle_coordinates(me, &x1, &y1, &x2, &y2);
    update_rectangle_coordinates(me, x1, y1, x2, y2);

    decorated_machine->init(decorated_machine);
}
/* objectmachine_t 해제 */
void release(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_lockcamera_t *me = (objectdecorator_lockcamera_t*)obj;
    player_t *player = enemy_get_observed_player(obj->get_object_instance(obj));

    if(me->has_locked_somebody) {
        player->in_locked_area = FALSE;
        level_unlock_camera();
    }

    expression_destroy(me->x1);
    expression_destroy(me->x2);
    expression_destroy(me->y1);
    expression_destroy(me->y2);

    decorated_machine->release(decorated_machine);
    free(obj);
}

/* objectmachine_t  변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    object_t *object = obj->get_object_instance(obj);
    player_t *player = enemy_get_observed_player(object);
    objectdecorator_lockcamera_t *me = (objectdecorator_lockcamera_t*)obj;
    actor_t *act = object->actor, *ta;
    float rx, ry, rw, rh;
    int x1, x2, y1, y2;
    int i;

    /* 직사각형을 구성 */
    get_rectangle_coordinates(me, &x1, &y1, &x2, &y2);
    update_rectangle_coordinates(me, x1, y1, x2, y2);

    /* 직사각형 배경에서의 배치, 구성 */
    rx = act->position.x + x1;
    ry = act->position.y + y1;
    rw = x2 - x1;
    rh = y2 - y1;

    /* 이지역에 다다른 플레이어는 입력 가능 */
    for(i=0; i<team_size; i++) {
        ta = team[i]->actor;

        if(team[i] != player) {
            /* 입력 불가 */
            float border = 30.0f;
            if(ta->position.x > rx - border && ta->position.x < rx) {
                ta->position.x = rx - border;
                ta->speed.x = 0.0f;
            }
            if(ta->position.x > rx + rw && ta->position.x < rx + rw + border) {
                ta->position.x = rx + rw + border;
                ta->speed.x = 0.0f;
            }
        }
        else {
            /* test 플레이어는 사각형 안에 있는 경우 */
            float a[4], b[4];

            a[0] = ta->position.x;
            a[1] = ta->position.y;
            a[2] = ta->position.x + 1;
            a[3] = ta->position.y + 1;

            b[0] = rx;
            b[1] = ry;
            b[2] = rx + rw;
            b[3] = ry + rh;

            if(bounding_box(a, b)) {
                /* 플레이어가 지역안에서 잠겨 있을 때 */
                me->has_locked_somebody = TRUE;
                team[i]->in_locked_area = TRUE;
                level_lock_camera(rx, ry, rx+rw, ry+rh);
            }
        }
    }


    /* cage */
    if(me->has_locked_somebody) {
        ta = player->actor;
        if(ta->position.x < rx) {
            ta->position.x = rx;
            ta->speed.x = max(0.0f, ta->speed.x);
            player->at_some_border = TRUE;
        }
        if(ta->position.x > rx + rw) {
            ta->position.x = rx + rw;
            ta->speed.x = min(0.0f, ta->speed.x);
            player->at_some_border = TRUE;
        }
        ta->position.y = clip(ta->position.y, ry, ry + rh);
    }

    decorated_machine->update(decorated_machine, team, team_size, brick_list, item_list, object_list);
}
/*  변화에 대한 모습 (이미지) */
void render(objectmachine_t *obj, v2d_t camera_position)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    if(level_editmode()) {
        objectdecorator_lockcamera_t *me = (objectdecorator_lockcamera_t*)obj;
        actor_t *act = obj->get_object_instance(obj)->actor;
        uint32 color = image_rgb(255, 0, 0);
        int x1, y1, x2, y2;

        x1 = (act->position.x + me->_x1) - (camera_position.x - VIDEO_SCREEN_W/2);
        y1 = (act->position.y + me->_y1) - (camera_position.y - VIDEO_SCREEN_H/2);
        x2 = (act->position.x + me->_x2) - (camera_position.x - VIDEO_SCREEN_W/2);
        y2 = (act->position.y + me->_y2) - (camera_position.y - VIDEO_SCREEN_H/2);

        image_line(video_get_backbuffer(), x1, y1, x2, y1, color);
        image_line(video_get_backbuffer(), x2, y1, x2, y2, color);
        image_line(video_get_backbuffer(), x2, y2, x1, y2, color);
        image_line(video_get_backbuffer(), x1, y2, x1, y1, color);
    }

    decorated_machine->render(decorated_machine, camera_position);
}


/* 보조 기능 */

void get_rectangle_coordinates(objectdecorator_lockcamera_t *me, int *x1, int *y1, int *x2, int *y2)
{
    int mi, ma;

    *x1 = (int)expression_evaluate(me->x1);
    *x2 = (int)expression_evaluate(me->x2);
    *y1 = (int)expression_evaluate(me->y1);
    *y2 = (int)expression_evaluate(me->y2);

    if(*x1 == *x2) (*x2)++;
    if(*y1 == *y2) (*y2)++;

    mi = min(*x1, *x2);
    ma = max(*x1, *x2);
    *x1 = mi;
    *x2 = ma;

    mi = min(*y1, *y2);
    ma = max(*y1, *y2);
    *y1 = mi;
    *y2 = ma;
}

void update_rectangle_coordinates(objectdecorator_lockcamera_t *me, int x1, int y1, int x2, int y2)
{
    me->_x1 = x1;
    me->_y1 = y1;
    me->_x2 = x2;
    me->_y2 = y2;
}
