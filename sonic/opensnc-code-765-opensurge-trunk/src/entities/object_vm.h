/*
 * Open Surge Engine
 * object_vm.h - virtual machine of the objects
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

#ifndef _OBJECT_VM_H
#define _OBJECT_VM_H

#include "enemy.h"
#include "object_decorators/base/objectmachine.h"
#include "../core/nanocalc/nanocalc.h"

/* an objectvm_t is a finite state machine.
   Every state has a name an can be decorated
   (in terms of the Decorator Design Pattern) */

typedef struct objectvm_t objectvm_t;

/* public methods */

objectvm_t* objectvm_create(enemy_t* owner); /* 새로운 virtual machine(VM)을 생성한다. */
objectvm_t* objectvm_destroy(objectvm_t* vm); /* 존재하는 VM을 삭제한다. */
objectmachine_t** objectvm_get_reference_to_current_state(objectvm_t* vm); /* 현재 상태의 reference를 return하는 함수. */
symboltable_t* objectvm_get_symbol_table(objectvm_t *vm); /* 대상의 symbol table을 return하는 함수. (variables support; nanocalc stuff...) */
void objectvm_create_state(objectvm_t* vm, const char *name); /* 사용하기전에 상태를 생성해주는 함수. */
const char* objectvm_get_current_state(objectvm_t* vm); /* 현재상태를 알려주는 함수. */
void objectvm_set_current_state(objectvm_t* vm, const char *name); /* 현재 상태를 설정하는 함수 */
void objectvm_return_to_previous_state(objectvm_t *vm); /* 이전 상태를 return해주는 함수 */
void objectvm_reset_history(objectvm_t *vm);  /* 상태의 기록을 재설정하는 함수. ( 이전 상태로 되돌릴 수 없음 ) */
objectmachine_t* objectvm_get_state_by_name(objectvm_t* vm, const char *name); /* 이름으로 이전 상태를 검색하는 함수. */

#endif
