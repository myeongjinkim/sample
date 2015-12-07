
#include "audio.h"
#include "../../core/util.h"
#include "../../core/audio.h"
#include "../../core/soundfactory.h"
#include "../../scenes/level.h"

typedef struct objectdecorator_audio_t objectdecorator_audio_t;
typedef struct audiostrategy_t audiostrategy_t;
typedef struct playsamplestrategy_t playsamplestrategy_t;
typedef struct playmusicstrategy_t playmusicstrategy_t;
typedef struct playlevelmusicstrategy_t playlevelmusicstrategy_t;
typedef struct setmusicvolumestrategy_t setmusicvolumestrategy_t;

/* objectdecorator_audio_t 클래스 */
struct objectdecorator_audio_t {
    objectdecorator_t base; /* objectdecorator_t에 상속 */
    audiostrategy_t *strategy;
};

/* strategy 구조체 */
struct audiostrategy_t {
    void (*update)(audiostrategy_t*);
};

/* play_sample strategy 구조체 */
struct playsamplestrategy_t {
    audiostrategy_t base;
    sound_t *sfx; /* sample to be played 약자*/
    float vol, pan, freq;
    int loop;
};

static audiostrategy_t* playsamplestrategy_new(const char *sample_name, float vol, float pan, float freq, int loop);
static void playsamplestrategy_update(audiostrategy_t *s);

/* play_music strategy 구조체 */
struct playmusicstrategy_t {
    audiostrategy_t base;
    music_t *mus; /* music to be played 약자 */
    int loop;
};

static audiostrategy_t* playmusicstrategy_new(const char *music_name, int loop);
static void playmusicstrategy_update(audiostrategy_t *s);

/* set_music_volume strategy 구조체 */
struct setmusicvolumestrategy_t {
    audiostrategy_t base;
    float vol;
};

static audiostrategy_t* setmusicvolumestrategy_new(float vol);
static void setmusicvolumestrategy_update(audiostrategy_t *s);

/* play_level_music strategy 구조체 */
struct playlevelmusicstrategy_t {
    audiostrategy_t base;
};

static audiostrategy_t* playlevelmusicstrategy_new();
static void playlevelmusicstrategy_update(audiostrategy_t *s);

/* private methods */
static void init(objectmachine_t *obj);
static void release(objectmachine_t *obj);
static void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list);
static void render(objectmachine_t *obj, v2d_t camera_position);

static objectmachine_t* make_decorator(objectmachine_t *decorated_machine, audiostrategy_t *strategy);



/* public methods */

/* 구조체 클래스들 */
objectmachine_t* objectdecorator_playsample_new(objectmachine_t *decorated_machine, const char *sample_name, float vol, float pan, float freq, int loop)
{
    return make_decorator(decorated_machine, playsamplestrategy_new(sample_name, vol, pan, freq, loop));
}

objectmachine_t* objectdecorator_playmusic_new(objectmachine_t *decorated_machine, const char *music_name, int loop)
{
    return make_decorator(decorated_machine, playmusicstrategy_new(music_name, loop));
}

objectmachine_t* objectdecorator_playlevelmusic_new(objectmachine_t *decorated_machine)
{
    return make_decorator(decorated_machine, playlevelmusicstrategy_new());
}

objectmachine_t* objectdecorator_setmusicvolume_new(objectmachine_t *decorated_machine, float vol)
{
    return make_decorator(decorated_machine, setmusicvolumestrategy_new(vol));
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
/* objectmachine_t 객체를 자유롭게 함 */
void release(objectmachine_t *obj)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_audio_t *me = (objectdecorator_audio_t*)obj;

    free(me->strategy);

    decorated_machine->release(decorated_machine);
    free(obj);
}
/* objectmachine_t 사운드 변화 */
void update(objectmachine_t *obj, player_t **team, int team_size, brick_list_t *brick_list, item_list_t *item_list, object_list_t *object_list)
{
    objectdecorator_t *dec = (objectdecorator_t*)obj;
    objectmachine_t *decorated_machine = dec->decorated_machine;
    objectdecorator_audio_t *me = (objectdecorator_audio_t*)obj;

    me->strategy->update(me->strategy);

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






/* private stuff */
objectmachine_t* make_decorator(objectmachine_t *decorated_machine, audiostrategy_t *strategy)
{
    objectdecorator_audio_t *me = mallocx(sizeof *me);
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
/* 오디오 조절에 대한 크기 할당, 객체 생성 */
audiostrategy_t* playsamplestrategy_new(const char *sample_name, float vol, float pan, float freq, int loop)
{
    playsamplestrategy_t *s = mallocx(sizeof *s);
    ((audiostrategy_t*)s)->update = playsamplestrategy_update;

    s->sfx = soundfactory_get(sample_name);
    s->vol = clip(vol, 0.0f, 1.0f);
    s->pan = clip(pan, -1.0f, 1.0f);
    s->freq = freq;
    s->loop = (loop >= 0) ? loop : INFINITY;

    return (audiostrategy_t*)s;
}
/* 사운드에 대한 조절 함수 생성 */
void playsamplestrategy_update(audiostrategy_t *s)
{
    playsamplestrategy_t *me = (playsamplestrategy_t*)s;
    sound_play_ex(me->sfx, me->vol, me->pan, me->freq, me->loop);
}
/* 사운드에 대한 크기 할당 */
audiostrategy_t* playmusicstrategy_new(const char *music_name, int loop)
{
    playmusicstrategy_t *s = mallocx(sizeof *s);
    ((audiostrategy_t*)s)->update = playmusicstrategy_update;

    s->mus = music_load(music_name);
    s->loop = (loop >= 0) ? loop : INFINITY;

    return (audiostrategy_t*)s;
}

void playmusicstrategy_update(audiostrategy_t *s)
{
    playmusicstrategy_t *me = (playmusicstrategy_t*)s;
    music_play(me->mus, me->loop);
}

audiostrategy_t* playlevelmusicstrategy_new()
{
    playmusicstrategy_t *s = mallocx(sizeof *s);
    ((audiostrategy_t*)s)->update = playlevelmusicstrategy_update;
    return (audiostrategy_t*)s;
}

void playlevelmusicstrategy_update(audiostrategy_t *s)
{
    level_restore_music();
}
/* 사운드에 대한 조절 부분 생성 */
audiostrategy_t* setmusicvolumestrategy_new(float vol)
{
    setmusicvolumestrategy_t *s = mallocx(sizeof *s);
    ((audiostrategy_t*)s)->update = setmusicvolumestrategy_update;

    s->vol = clip(vol, 0.0f, 1.0f);

    return (audiostrategy_t*)s;
}
/* 사운드에 대한 조절 함수 생성 */
void setmusicvolumestrategy_update(audiostrategy_t *s)
{
    setmusicvolumestrategy_t *me = (setmusicvolumestrategy_t*)s;
    music_set_volume(me->vol);
}
/* 각기 다른 종류의 사운드 조절 */
