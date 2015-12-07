
#include "move_player.h"
#include "../../core/util.h"
#include "../../core/timer.h"
#include "../../entities/player.h"

/* objectdecorator_moveplayer_t 클래스 */
typedef struct objectdecorator_moveplayer_t objectdecorator_moveplayer_t;
struct objectdecorator_moveplayer_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    expression_t *speed_x, *speed_y; /* speed */
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);





/* public methods */

/* class constructor */
/* class 구조 구성, 할당 */
objectmachine_t* objectdecorator_moveplayer_new(objectmachine_t *decorated_machine, expression_t *speed_x, expression_t *speed_y)
{
    objectdecorator_moveplayer_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;
    me->speed_x = speed_x;
    me->speed_y = speed_y;

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
    objectdecorator_moveplayer_t *me = (objectdecorator_moveplayer_t*)obj;

    expression_destroy(me->speed_x);
    expression_destroy(me->speed_y);

    decorated_machine->release(decorated_machine);
    free(obj);
}

/* objectmachine_t  플레이어 캐릭터 위치 변화, 적 비추어지는 모습 설정 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_moveplayer_t *me = (objectdecorator_moveplayer_t*)obj;
    float dt = timer_get_delta();
    v2d_t speed = v2d_new(expression_evaluate(me->speed_x), expression_evaluate(me->speed_y));
    v2d_t ds = v2d_multiply(speed, dt);
    player_t *player = enemy_get_observed_player(obj->get_object_instance(obj));

    player->actor->position = v2d_add(player->actor->position, ds);

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
