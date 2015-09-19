/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2015 Treasure Data Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef FLB_THREAD_H
#define FLB_THREAD_H

#include <limits.h>
#include <inttypes.h>

#include <fluent-bit/flb_macros.h>
#include <fluent-bit/flb_utils.h>

pthread_key_t flb_thread_key;

struct flb_thread {
    int status;
    ucontext_t parent;
    ucontext_t caller;
    ucontext_t callee;
    ucontext_t context;

    //int (*start)(void);
    //int (*function) (struct flb_thread *, struct flb_output_plugin *,
    //                 void *data, size_t, size_t *);
};

#define FLB_THREAD_STACK(p)    (((char *) p) + sizeof(struct flb_thread))
#define FLB_THREAD_STACK_SIZE  ((3 * PTHREAD_STACK_MIN) / 2)

#define flb_thread_resume(th)  swapcontext(&th->caller, &th->context)
#define flb_thread_yield(th)   swapcontext(&th->context, &th->caller)

static struct flb_thread *flb_thread_new()
{
    int ret;
    void *p;
    struct flb_thread *th;

    /* Create a thread context and initialize */
    p = malloc(sizeof(struct flb_thread) + FLB_THREAD_STACK_SIZE);
    if (!p) {
        perror("malloc");
        return NULL;
    }

    th = (struct flb_thread *) p;
    ret = getcontext(&th->context);
    if (ret == -1) {
        perror("getcontext");
        free(th);
        return NULL;
    }

    th->context.uc_stack.ss_sp    = FLB_THREAD_STACK(p);
    th->context.uc_stack.ss_size  = FLB_THREAD_STACK_SIZE;
    th->context.uc_stack.ss_flags = 0;
    th->context.uc_link           = &th->caller;

    return th;
}

#endif
