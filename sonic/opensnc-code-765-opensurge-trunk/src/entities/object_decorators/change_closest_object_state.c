
#include "change_closest_object_state.h"
#include "../object_vm.h"
#include "../../core/util.h"
#include "../../core/stringutil.h"

/* objectdecorator_changeclosestobjectstate_t 클래스 */
typedef struct objectdecorator_changeclosestobjectstate_t objectdecorator_changeclosestobjectstate_t;
struct objectdecorator_changeclosestobjectstate_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    char *object_name; /* 가장 가까운 개체가 OBJECT_NAME이라고 찾을 수 있다 */
    char *new_state_name; /* 그리고 new_state_name하기 위해 상태를 변경 */
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);

static object_t *find_closest_object(object_t *me, object_list_t *list, const char* desired_name, float *distance);



/* public methods */
/* class 구성, 할당 */
objectmachine_t* objectdecorator_changeclosestobjectstate_new(objectmachine_t *decorated_machine, const char *object_name, const char *new_state_name)
{
    objectdecorator_changeclosestobjectstate_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* superclass에 상속 */
    dec->decorated_machine = decorated_machine;

    me->object_name = str_dup(object_name);
    me->new_state_name = str_dup(new_state_name);

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
    objectdecorator_changeclosestobjectstate_t *me = (objectdecorator_changeclosestobjectstate_t*)obj;
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    free(me->object_name);
    free(me->new_state_name);

    decorated_machine->release(decorated_machine);
    free(obj);
}
/* objectmachine_t 캐릭터 위치, 가까운 객체 찾아 목표 변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_changeclosestobjectstate_t *me = (objectdecorator_changeclosestobjectstate_t*)obj;
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    object_t *object = obj->get_object_instance(obj);
    object_t *target = find_closest_object(object, object_list, me->object_name, NULL);
    /* target 이 NULL이 아닐 경우 최근 상태로 변화 */
    if(target != NULL)
        objectvm_set_current_state(target->vm, me->new_state_name);

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
/* list 작성 */
object_t *find_closest_object(object_t *me, object_list_t *list, const char* desired_name, float *distance)
{
    float min_dist = INFINITY_FLT;
    object_list_t *it;
    object_t *ret = NULL;
    v2d_t v;

    for(it=list; it; it=it->next) { /* 이 목록은 충분히 작아야한다 */
        if(str_icmp(it->data->name, desired_name) == 0) {
            v = v2d_subtract(it->data->actor->position, me->actor->position);
            /* 플레이어 캐릭터의 위치 변화 */
            if(v2d_magnitude(v) < min_dist) {
                ret = it->data;
                min_dist = v2d_magnitude(v);
            }
        }
    }

    if(distance)
        *distance = min_dist;

    return ret;
}
