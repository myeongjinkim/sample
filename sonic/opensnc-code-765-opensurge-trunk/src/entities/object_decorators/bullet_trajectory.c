
#include "bullet_trajectory.h"
#include "../../core/util.h"
#include "../../core/timer.h"

/* objectdecorator_bullettrajectory_t 클래스 */
typedef struct objectdecorator_bullettrajectory_t objectdecorator_bullettrajectory_t;
struct objectdecorator_bullettrajectory_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    v2d_t speed; /* 총알 속도 */
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);





/* public methods */

/* class 구성, 할당 */
objectmachine_t* objectdecorator_bullettrajectory_new(objectmachine_t *decorated_machine, float speed_x, float speed_y)
{
    objectdecorator_bullettrajectory_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* superclass에 상속 */
    dec->decorated_machine = decorated_machine;
    me->speed = v2d_new(speed_x, speed_y);

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
/* objectmachine_t 캐릭터 위치, 시간 변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_bullettrajectory_t *me = (objectdecorator_bullettrajectory_t*)obj;
    object_t *object = obj->get_object_instance(obj);
    float dt = timer_get_delta();
    v2d_t ds;

    ds = v2d_multiply(me->speed, dt);
    object->actor->position = v2d_add(object->actor->position, ds);

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
