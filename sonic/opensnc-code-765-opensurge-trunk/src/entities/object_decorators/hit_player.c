
#include "hit_player.h"
#include "../../core/util.h"
#include "../../entities/player.h"

/* objectdecorator_hitplayer_t 클래스 */
typedef struct objectdecorator_hitplayer_t objectdecorator_hitplayer_t;
struct objectdecorator_hitplayer_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    int (*should_hit_the_player)(player_t*); /* strategy pattern */
};

/* private strategies */
static int hit_strategy(player_t *p);
static int burn_strategy(player_t *p);
static int shock_strategy(player_t *p);
static int acid_strategy(player_t *p);

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);
static objectmachine_t *make_decorator(objectmachine_t *decorated_machine, int (*strategy)(player_t*));


/* public methods */

/* class constructor */
/* class 구조 구성, 할당 */
objectmachine_t* objectdecorator_hitplayer_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, hit_strategy);
}

objectmachine_t* objectdecorator_burnplayer_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, burn_strategy);
}

objectmachine_t* objectdecorator_shockplayer_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, shock_strategy);
}

objectmachine_t* objectdecorator_acidplayer_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, acid_strategy);
}


/* private methods */

/* builder */
/* class 구조 구성, 할당 */
objectmachine_t *make_decorator(objectmachine_t *decorated_machine, int (*strategy)(player_t*))
{
    objectdecorator_hitplayer_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;
    me->should_hit_the_player = strategy;

    return obj;
}
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
/* objectmachine_t  가격한 캐릭터의 변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_hitplayer_t *me = (objectdecorator_hitplayer_t*)obj;
    object_t *object = obj->get_object_instance(obj);
    player_t *player = enemy_get_observed_player(obj->get_object_instance(obj));

    if(!player->invincible && me->should_hit_the_player(player))
        player_hit(player, object->actor);

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
int hit_strategy(player_t *p)
{
    return TRUE;
}

int burn_strategy(player_t *p)
{
    return p->shield_type != SH_FIRESHIELD && p->shield_type != SH_WATERSHIELD;
}

int shock_strategy(player_t *p)
{
    return p->shield_type != SH_THUNDERSHIELD;
}

int acid_strategy(player_t *p)
{
    return p->shield_type != SH_ACIDSHIELD;
}
