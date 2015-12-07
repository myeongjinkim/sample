
#include "enemy.h"
#include "../../core/soundfactory.h"
#include "../../core/util.h"
#include "../../scenes/level.h"

/* objectdecorator_enemy_t 클래스 */
typedef struct objectdecorator_enemy_t objectdecorator_enemy_t;
struct objectdecorator_enemy_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    expression_t *score;
};

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);



/* public methods */

/* class constructor */
/* class 구조 구성, 할당 */
objectmachine_t* objectdecorator_enemy_new(objectmachine_t *decorated_machine, expression_t *score)
{
    objectdecorator_enemy_t *me = mallocx(sizeof *me);
    objectdecorator_t *dec = (objectdecorator_t*)me;
    objectmachine_t *obj = (objectmachine_t*)dec;

    obj->init = init;
    obj->release = release;
    obj->update = update;
    obj->render = render;
    obj->get_object_instance = objectdecorator_get_object_instance; /* inherits from superclass */
    dec->decorated_machine = decorated_machine;

    me->score = score;

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
    objectdecorator_enemy_t *me = (objectdecorator_enemy_t*)obj;

    expression_destroy(me->score);

    decorated_machine->release(decorated_machine);
    free(obj);
}
/* objectmachine_t  변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectdecorator_enemy_t *me = (objectdecorator_enemy_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    object_t *object = obj->get_object_instance(obj);
    int i, score;

    score = (int)expression_evaluate(me->score);

    /* 플레이어 x 객체 충돌 */
    for(i=0; i<team_size; i++) {
        player_t *player = team[i];
        if(actor_pixelperfect_collision(object->actor, player->actor)) {
            if(player_is_attacking(player) || player->invincible) {
                /* 미션 실패 */
                player_bounce(player, object->actor);
                level_add_to_score(score);
                level_create_item(IT_EXPLOSION, v2d_add(object->actor->position, v2d_new(0,-15)));
                level_create_animal(object->actor->position);
                sound_play( soundfactory_get("destroy") );
                object->state = ES_DEAD;
            }
            else {
                /* 플레이어가 공격 시*/
                player_hit(player, object->actor);
            }
        }
    }

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
