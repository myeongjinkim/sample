
#include "load_level.h"
#include "../../core/util.h"
#include "../../core/stringutil.h"
#include "../../scenes/level.h"

/* objectdecorator_loadlevel_t 클래스 */
typedef struct objectdecorator_loadlevel_t objectdecorator_loadlevel_t;
struct objectdecorator_loadlevel_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    char *level_path; /* level to be loaded */
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);





/* public methods */

/* class constructor */
/* class 구조 구성, 할당 */
objectmachine_t* objectdecorator_loadlevel_new(objectmachine_t *decorated_machine, const char *level_path)
{
    objectdecorator_loadlevel_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;
    me->level_path = str_dup(level_path);

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
    objectdecorator_loadlevel_t *me = (objectdecorator_loadlevel_t*)obj;

    free(me->level_path);

    decorated_machine->release(decorated_machine);
    free(obj);
}
/* objectmachine_t  변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    /*objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;*/
    objectdecorator_loadlevel_t *me = (objectdecorator_loadlevel_t*)obj;

    level_change(me->level_path);

    /*decorated_machine->update(decorated_machine, team, team_size, brick_list, item_list, object_list);*/
}
/*  변화에 대한 모습 */
void render(objectmachine_t *obj, v2d_t camera_position)
{
    /*objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;*/

    ; /* empty */

    /*decorated_machine->render(decorated_machine, camera_position);*/
}
