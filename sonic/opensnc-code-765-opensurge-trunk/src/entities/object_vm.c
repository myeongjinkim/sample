/*
 * Open Surge Engine
 * object_vm.c - virtual machine of the objects
 * Copyright (C) 2010, 2012  Alexandre Martins <alemartf(at)gmail(dot)com>
 * http://opensnc.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "object_vm.h"
#include "../core/util.h"
#include "../core/stringutil.h"
#include "object_decorators/base/objectmachine.h"
#include "object_decorators/base/objectbasicmachine.h"

/* private stuff */
typedef struct objectmachine_list_t objectmachine_list_t;
typedef struct objectmachine_stack_t objectmachine_stack_t;

/* objectvm_t class */
struct objectvm_t
{
    enemy_t* owner; /* 이 가상머신의 주인을 표시하는 변수 */
    objectmachine_list_t* state_list; /* 상태의 리스트 */
    objectmachine_t** reference_to_current_state; /* 현재 상태의 reference */
    symboltable_t* symbol_table; /* 객체의 private sybol table (각각의 변수를 저장) */
    objectmachine_stack_t* history; /* 이전의 상태를 저장하는 변수 */
};

/* 객체 머신의 linked list */
struct objectmachine_list_t {
    char *name;
    objectmachine_t *data;
    objectmachine_list_t *next;
};

static objectmachine_list_t* objectmachine_list_new(objectmachine_list_t* list, const char *name, enemy_t* owner);
static objectmachine_list_t* objectmachine_list_delete(objectmachine_list_t* list);
static objectmachine_list_t* objectmachine_list_find(objectmachine_list_t* list, const char *name);
static objectmachine_list_t* objectmachine_list_findmachine(objectmachine_list_t* list, objectmachine_t* machine);

/* 객체 머신의 stack */
#define OBJECTMACHINE_STACK_CAPACITY 5 /* return_to_previous_state history */
struct objectmachine_stack_t {
    objectmachine_list_t *data[OBJECTMACHINE_STACK_CAPACITY]; /* nodes of objectmachine_list_t */
    int top, size;
};

static objectmachine_stack_t* objectmachine_stack_new();
static objectmachine_stack_t* objectmachine_stack_delete(objectmachine_stack_t* stack);
static void objectmachine_stack_push(objectmachine_stack_t* stack, objectmachine_list_t *list_node);
static objectmachine_list_t* objectmachine_stack_pop(objectmachine_stack_t* stack);
static void objectmachine_stack_clear(objectmachine_stack_t* stack);



/* public methods */
/* 새로운 virtual machine(VM)을 생성한다. */
objectvm_t* objectvm_create(enemy_t* owner)
{
    objectvm_t *vm = mallocx(sizeof *vm);
    vm->owner = owner;
    vm->state_list = NULL;
    vm->reference_to_current_state = NULL;
    vm->history = objectmachine_stack_new();
    vm->symbol_table = symboltable_new();
    return vm;
}
/* 존재하는 VM을 삭제한다. */
objectvm_t* objectvm_destroy(objectvm_t* vm)
{
    symboltable_destroy(vm->symbol_table);
    vm->history = objectmachine_stack_delete(vm->history);
    vm->state_list = objectmachine_list_delete(vm->state_list);
    vm->reference_to_current_state = NULL;
    vm->owner = NULL;
    free(vm);
    return NULL;
}
 /* 현재 상태의 reference를 return하는 함수. */
objectmachine_t** objectvm_get_reference_to_current_state(objectvm_t* vm)
{
    return vm->reference_to_current_state;
}
/* 대상의 symbol table을 return하는 함수. (variables support; nanocalc stuff...) */
symboltable_t* objectvm_get_symbol_table(objectvm_t *vm)
{
    return vm->symbol_table;
}
/* 사용하기전에 상태를 생성해주는 함수. */
void objectvm_create_state(objectvm_t* vm, const char *name)
{
    if(objectmachine_list_find(vm->state_list, name) == NULL)
        vm->state_list = objectmachine_list_new(vm->state_list, name, vm->owner);
    else
        fatal_error("Object script error: can't redefine state \"%s\" in object \"%s\".", name, vm->owner->name);
}

 /* 현재상태를 알려주는 함수. */
const char* objectvm_get_current_state(objectvm_t* vm)
{
    objectmachine_list_t *m = objectmachine_list_findmachine(vm->state_list, *(vm->reference_to_current_state));

    if(m == NULL) {
        fatal_error("Object script error: can't get current state name in object \"%s\". This shouldn't happen.", vm->owner->name);
        return NULL;
    }
    else
        return m->name;
}
/* 현재 상태를 설정하는 함수 */
void objectvm_set_current_state(objectvm_t* vm, const char *name)
{
    objectmachine_list_t *m = objectmachine_list_find(vm->state_list, name);

    if(m != NULL) {
        if(vm->reference_to_current_state != &(m->data)) {
            vm->reference_to_current_state = &(m->data);
            objectmachine_stack_push(vm->history, m);
        }
    }
    else
        fatal_error("Object script error: can't find state \"%s\" in object \"%s\".", name, vm->owner->name);
}
/* 이전 상태를 return해주는 함수 */
void objectvm_return_to_previous_state(objectvm_t *vm)
{
    objectmachine_list_t *m;

    objectmachine_stack_pop(vm->history); /* discard current state */
    m = objectmachine_stack_pop(vm->history); /* previous state */

    if(m != NULL) {
        vm->reference_to_current_state = &(m->data);
        objectmachine_stack_push(vm->history, m);
    }
    else
        fatal_error("Object script error: can't return to previous state in object \"%s\".", vm->owner->name);
}
 /* 상태의 기록을 재설정하는 함수. ( 이전 상태로 되돌릴 수 없음 ) */
void objectvm_reset_history(objectvm_t *vm)
{
    objectmachine_stack_clear(vm->history);
}
/* 이름으로 이전 상태를 검색하는 함수. */
objectmachine_t* objectvm_get_state_by_name(objectvm_t* vm, const char *name)
{
    objectmachine_list_t *m = objectmachine_list_find(vm->state_list, name);

    if(m == NULL) {
        fatal_error("Object script error: can't find state \"%s\" in object \"%s\".", name, vm->owner->name);
        return NULL;
    }
    else
        return m->data;
}

/* objectmachine_list_t: private methods */
/* objectmachine_list를 새로 생성한다. */
objectmachine_list_t* objectmachine_list_new(objectmachine_list_t* list, const char *name, enemy_t *owner)
{
    objectmachine_list_t *l = mallocx(sizeof *l);
    l->name = str_dup(name);
    l->data = objectbasicmachine_new(owner);
    l->next = list;
    return l;
}
/* objectmachine_list를 삭제한다. */
objectmachine_list_t* objectmachine_list_delete(objectmachine_list_t* list)
{
    if(list != NULL) {
        objectmachine_t *machine = list->data;
        objectmachine_list_delete(list->next);
        free(list->name);
        machine->release(machine);
        free(list);
    }

    return NULL;
}
/* objectmachine_list의 내용을 객체의 이름으로 검색한다. */
objectmachine_list_t* objectmachine_list_find(objectmachine_list_t* list, const char *name)
{
    if(list != NULL) {
        if(str_icmp(list->name, name) != 0)
            return objectmachine_list_find(list->next, name);
        else
            return list;
    }
    else
        return NULL;
}
/* objectmachine_list의 내용을 machine이름 으로 검색한다. */
objectmachine_list_t* objectmachine_list_findmachine(objectmachine_list_t* list, objectmachine_t* machine)
{
     if(list != NULL) {
        if(list->data != machine)
            return objectmachine_list_findmachine(list->next, machine);
        else
            return list;
    }
    else
        return NULL;
}

/* objectmachine_stack_t: private methods */
/* objectmachine_stack_t 을 새로 생성한다. */
objectmachine_stack_t* objectmachine_stack_new()
{
    objectmachine_stack_t *s = mallocx(sizeof *s);
    s->top = s->size = 0;
    return s;
}
/* objectmachine_stack_t 을 삭제한다. */
objectmachine_stack_t* objectmachine_stack_delete(objectmachine_stack_t* stack)
{
    free(stack);
    return NULL;
}
/* objectmachine_stack_t에 내용을 추가한다. */
void objectmachine_stack_push(objectmachine_stack_t* stack, objectmachine_list_t *list_node)
{
    int n = OBJECTMACHINE_STACK_CAPACITY;

    stack->size = min(n, 1 + stack->size);
    stack->data[stack->top] = list_node;
    stack->top = (stack->top+1) % n; /* circular stack */
}
/* objectmachine_stack_t의 내용을 가져온다. */
objectmachine_list_t* objectmachine_stack_pop(objectmachine_stack_t* stack)
{
    int n = OBJECTMACHINE_STACK_CAPACITY;

    if(stack->size > 0) {
        --(stack->size);
        stack->top = ((stack->top-1)+n)%n;
        return stack->data[stack->top];
    }
    else
        return NULL; /* empty stack */
}
/* objectmachine_stack_t 을 비운다. */
void objectmachine_stack_clear(objectmachine_stack_t* stack)
{
    stack->size = stack->top = 0;
}
