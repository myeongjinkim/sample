
#include "children.h"
#include "../object_vm.h"
#include "../../core/util.h"
#include "../../core/stringutil.h"
#include "../../core/nanocalcext.h"
#include "../../scenes/level.h"

typedef struct objectdecorator_children_t objectdecorator_children_t;
typedef void (*childrenstrategy_t)(objectdecorator_children_t*,player_t**,int,brick_list_t*,item_list_t*,object_list_t*);

/* objectdecorator_children_t 클래스 */
struct objectdecorator_children_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    char *object_name; /*  OBJECT_NAME라는 개체를 생성 */
    expression_t *offset_x, *offset_y; /* x축, y축 상쇄 */
    char *child_name; /* child name */
    char *new_state_name; /* new state name */
    childrenstrategy_t strategy; /* strategy pattern */
};

/* private strategies */
static void createchild_strategy(objectdecorator_children_t *me, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void changechildstate_strategy(objectdecorator_children_t *me, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void changeparentstate_strategy(objectdecorator_children_t *me, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);

static objectmachine_t *make_decorator(objectmachine_t *decorated_machine, childrenstrategy_t strategy, expression_t *offset_x, expression_t *offset_y, const char *object_name, const char *child_name, const char *new_state_name);



/* public methods */
objectmachine_t* objectdecorator_createchild_new(objectmachine_t *decorated_machine, const char *object_name, expression_t *offset_x, expression_t *offset_y, const char *child_name)
{
    return make_decorator(decorated_machine, createchild_strategy, offset_x, offset_y, object_name, child_name, NULL);
}

objectmachine_t* objectdecorator_changechildstate_new(objectmachine_t *decorated_machine, const char *child_name, const char *new_state_name)
{
    return make_decorator(decorated_machine, changechildstate_strategy, NULL, NULL, NULL, child_name, new_state_name);
}

objectmachine_t* objectdecorator_changeparentstate_new(objectmachine_t *decorated_machine, const char *new_state_name)
{
    return make_decorator(decorated_machine, changeparentstate_strategy, NULL, NULL, NULL, NULL, new_state_name);
}


/* make a decorator... */
objectmachine_t *make_decorator(objectmachine_t *decorated_machine, childrenstrategy_t strategy, expression_t *offset_x, expression_t *offset_y, const char *object_name, const char *child_name, const char *new_state_name)
{
    objectdecorator_children_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;

    me->strategy = strategy;
    me->offset_x = offset_x;
    me->offset_y = offset_y;
    me->object_name = object_name != NULL ? str_dup(object_name) : NULL;
    me->child_name = child_name != NULL ? str_dup(child_name) : NULL;
    me->new_state_name = new_state_name != NULL ? str_dup(new_state_name) : NULL;

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
    objectdecorator_children_t *me = (objectdecorator_children_t*)obj;
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    if(me->child_name != NULL)
        free(me->child_name);

    if(me->object_name != NULL)
        free(me->object_name);

    if(me->new_state_name != NULL)
        free(me->new_state_name);

    if(me->offset_x != NULL)
        expression_destroy(me->offset_x);

    if(me->offset_y != NULL)
        expression_destroy(me->offset_y);

    decorated_machine->release(decorated_machine);
    free(obj);
}
/* objectmachine_t  변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_children_t *me = (objectdecorator_children_t*)obj;
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    me->strategy(me, team, team_size, brick_list, item_list, object_list);

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
void createchild_strategy(objectdecorator_children_t *me, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectmachine_t *obj = (objectmachine_t*)me;
    object_t *object = obj->get_object_instance(obj);
    object_t *child;
    v2d_t offset;
    /* 상쇄 식 */
    offset.x = expression_evaluate(me->offset_x);
    offset.y = expression_evaluate(me->offset_y);
    child = level_create_enemy(me->object_name, v2d_add(object->actor->position, offset));
    if(child != NULL) {
        enemy_add_child(object, me->child_name, child);
        enemy_update(child, team, team_size, brick_list, item_list, object_list); /* important to exchange data between objects */
        nanocalcext_set_target_object(object, brick_list, item_list, object_list); /* restore nanocalc's target object */
    }
}

void changechildstate_strategy(objectdecorator_children_t *me, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectmachine_t *obj = (objectmachine_t*)me;
    object_t *object = obj->get_object_instance(obj);
    object_t *child;

    child = enemy_get_child(object, me->child_name);
    if(child != NULL) {
        objectvm_set_current_state(child->vm, me->new_state_name);
        enemy_update(child, team, team_size, brick_list, item_list, object_list); /* important to exchange data between objects */
        nanocalcext_set_target_object(object, brick_list, item_list, object_list); /* restore nanocalc's target object */
    }
}

void changeparentstate_strategy(objectdecorator_children_t *me, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectmachine_t *obj = (objectmachine_t*)me;
    object_t *object = obj->get_object_instance(obj);
    object_t *parent;

    parent = enemy_get_parent(object);
    if(parent != NULL) {
        objectvm_set_current_state(parent->vm, me->new_state_name);
        enemy_update(parent, team, team_size, brick_list, item_list, object_list); /* important to exchange data between objects */
        nanocalcext_set_target_object(object, brick_list, item_list, object_list); /* restore nanocalc's target object */
    }
}
