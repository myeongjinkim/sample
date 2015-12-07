/
#include "add_lives.h"
#include "../../core/util.h"
#include "../../scenes/level.h"

/* objectdecorator_addlives_t 클래스 */
typedef struct objectdecorator_addlives_t objectdecorator_addlives_t;
struct objectdecorator_addlives_t {
    objectdecorator_t base; /*  objectdecorator_t에 상속  */
    expression_t *lives; /* 얼마나 많은 생명을 추가 할 것인가? */
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);



/* public methods */

/* class 구조 구성 */
objectmachine_t* objectdecorator_addlives_new(objectmachine_t *decorated_machine, expression_t *lives)
{
    objectdecorator_addlives_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;
    me->lives = lives;

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
    objectdecorator_addlives_t *me = (objectdecorator_addlives_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;

    expression_destroy(me->lives);

    decorated_machine->release(decorated_machine);
    free(obj);
}
/* objectmachine_t 캐릭터 live 모습 변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_addlives_t *me = (objectdecorator_addlives_t*)obj;

    player_set_lives( player_get_lives() + (int)expression_evaluate(me->lives) );

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
