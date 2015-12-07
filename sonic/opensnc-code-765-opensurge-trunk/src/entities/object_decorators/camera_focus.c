#include "camera_focus.h"
#include "../../core/util.h"
#include "../../scenes/level.h"

/* objectdecorator_camerafocus_t 클래스 */
typedef struct objectdecorator_camerafocus_t objectdecorator_camerafocus_t;
struct objectdecorator_camerafocus_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    void (*strategy)(objectmachine_t*); /* strategy pattern */
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);

static objectmachine_t* make_decorator(objectmachine_t *decorated_machine, void (*strategy)(objectmachine_t*));

/* private strategies */
static void request_camera_focus(objectmachine_t *obj);
static void drop_camera_focus(objectmachine_t *obj);



/* public methods */
/* class 구조 구성, 할당 */
objectmachine_t* objectdecorator_requestcamerafocus_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, request_camera_focus);
}

objectmachine_t* objectdecorator_dropcamerafocus_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, drop_camera_focus);
}

objectmachine_t* make_decorator(objectmachine_t *decorated_machine, void (*strategy)(objectmachine_t*))
{
    objectdecorator_camerafocus_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;
    me->strategy = strategy;

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
/* objectmachine_t  objectdecorator_camerafocus_t 구조체 변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_camerafocus_t *me = (objectdecorator_camerafocus_t*)obj;

    me->strategy(obj);

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


/* private strategies */
/* 카메라 위치 요청을 불러들이는 함수 */
void request_camera_focus(objectmachine_t *obj)
{
    level_set_camera_focus( obj->get_object_instance(obj)->actor );
}
/* 카메라 위치를 떨어뜰이는 요청을 불러들이는 함수 */
void drop_camera_focus(objectmachine_t *obj)
{
    if(level_get_camera_focus() == obj->get_object_instance(obj)->actor)
        level_set_camera_focus( level_player()->actor );
}
