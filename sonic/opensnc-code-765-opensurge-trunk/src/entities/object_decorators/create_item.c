
#include "create_item.h"
#include "../../core/util.h"
#include "../../scenes/level.h"

/* objectdecorator_createitem_t 클래스 */
typedef struct objectdecorator_createitem_t objectdecorator_createitem_t;
struct objectdecorator_createitem_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    expression_t *item_id; /*  ID가 item_id와하는 항목을 만드는 구조체. */
    expression_t *offset_x, *offset_y; /* this offset */
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);



/* public methods */
/* class 구조 구성, 할당 */
objectmachine_t* objectdecorator_createitem_new(objectmachine_t *decorated_machine, expression_t *item_id, expression_t *offset_x, expression_t *offset_y)
{
    objectdecorator_createitem_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;
    me->item_id = item_id;
    me->offset_x = offset_x;
    me->offset_y = offset_y;

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
/* 상쇄 식 파괴 */
void release(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_createitem_t *me = (objectdecorator_createitem_t*)obj;

    expression_destroy(me->item_id);
    expression_destroy(me->offset_x);
    expression_destroy(me->offset_y);

    decorated_machine->release(decorated_machine);
    free(obj);
}
/* objectmachine_t 상쇄 식 변화와 더불어서 단계 클리어시 아이템 생성 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_createitem_t *me = (objectdecorator_createitem_t*)obj;
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    object_t *object = obj->get_object_instance(obj);
    int item_id;
    v2d_t offset;

    item_id = (int)expression_evaluate(me->item_id);
    offset.x = expression_evaluate(me->offset_x);
    offset.y = expression_evaluate(me->offset_y);

    level_create_item(item_id, v2d_add(object->actor->position, offset));

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
