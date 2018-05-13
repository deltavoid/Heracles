#ifndef HERACLES_H
#define HERACLES_H

#include "core_memory_controller.h"
#include "network_controller.h"
#include "info_puller.h"
#include "tap.h"
#include "console.h"
#include "top_controller.h"
#include "helpers.h"
#include <pthread.h>

void *run_cm_ctr(void *p) {
    CoreMemoryController *cm_ctr = reinterpret_cast<CoreMemoryController *>(p);
    cm_ctr->run();
}

/*
void *run_tap(void *p) {
    Tap *tap = reinterpret_cast<Tap *>(p);
    tap->run();
}*/

class Heracles {
  private:
    Tap *tap;
    InfoPuller *puller;
    Console *console;

    CoreMemoryController *cm_ctr;
    NetworkController *net_ctr;
    TopController *t_ctr;

  public:
    Heracles(pid_t lc_pid) {
        intel_mutex_init();
        
        tap = new Tap(lc_pid);
        puller = new InfoPuller();
        console = new Console(tap);

        //cm_ctr = new CoreMemoryController(tap, puller);
        net_ctr = new NetworkController(tap);
        t_ctr = new TopController(tap, puller);
    }

    void exec() {
        int errno;
        pthread_t t, cmc, con, net;

        /*errno = pthread_create(&t, nullptr, run_tap, tap);
        if (errno != 0) {
            print_err("[HARACLES] can't create core_memory_controller.");
            exit(-1);
        }*/

        errno = pthread_create(&con, nullptr, Console::run, console);
        if (errno != 0) {
            print_err("[HARACLES] can't create console.");
            exit(-1);
        }

        /*errno = pthread_create(&cmc, nullptr, run_cm_ctr, cm_ctr);
        if (errno != 0) {
            print_err("[HARACLES] can't create core_memory_controller.");
            exit(-1);
        }*/

        errno = pthread_create(&cmc, nullptr, NetworkController::run, net_ctr);
        if (errno != 0) {
            print_err("[HARACLES] can't create network_controller.");
            exit(-1);
        }

        t_ctr->run();
    }
};

#endif