/*
 * Copyright (c) 2011 and 2012, Dustin Lundquist <dustin@null-ptr.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <signal.h>
#include <string.h> /* memset() */
#include <errno.h>
#include <uv.h>
/*\TODO burlak: remove #include <ev.h>*/
#include <ev.h>
#include "server.h"
#include "listener.h"
#include "connection.h"

static void signal_cb(uv_signal_t *, int);

static struct Config *config;
static uv_signal_t sighup_watcher;
static uv_signal_t sigusr1_watcher;
static uv_signal_t sigint_watcher;
static uv_signal_t sigterm_watcher;

void
init_server(struct Config *c) {
    config = c;
    uv_signal_init(uv_loop, &sighup_watcher);
    uv_signal_init(uv_loop, &sigusr1_watcher);
    uv_signal_init(uv_loop, &sigint_watcher);
    uv_signal_init(uv_loop, &sigterm_watcher);
    uv_signal_start(&sighup_watcher, signal_cb, SIGHUP);
    uv_signal_start(&sigusr1_watcher, signal_cb, SIGUSR1);
    uv_signal_start(&sigint_watcher, signal_cb, SIGINT);
    uv_signal_start(&sigterm_watcher, signal_cb, SIGTERM);

    /* ignore SIGPIPE, or it will kill us */
    signal(SIGPIPE, SIG_IGN);

    init_listeners(&config->listeners, &config->tables);
}

/**\TODO burlak: remove int running*/
int running = 1;

void
run_server() {
    init_connections();

    /**\TODO burlak: remove while*/
    do{
        ev_run(EV_DEFAULT, EVRUN_NOWAIT);
        uv_run(uv_loop, UV_RUN_NOWAIT);
    }while(running);

    free_connections(EV_DEFAULT);
}

static void
signal_cb(uv_signal_t* handle, int signum) {
    switch (signum) {
        case SIGHUP:
            break;
        case SIGUSR1:
            print_connections();
            break;
        case SIGINT:
        case SIGTERM:
            uv_stop(uv_loop);
            /*\TODO burlak: delete running = 0;*/
            running = 0;
    }
}
