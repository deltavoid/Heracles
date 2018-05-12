#ifndef NETWORK_CONTROLLER_H
#define NETWORK_CONTROLLER_H

#include "network_driver.h"
#include "network_monitor.h"
#include <pthread.h>
#include <string>

typedef unsigned u32;
typedef unsigned long long u64;

class NetworkController
{public:
    NetworkDriver* network_driver;
    NetworkMonitor* network_monitor;
    std::string default_device;
    u64 default_bandwidth;
    u64 usable_bandwidth;
    int LC_pid;
    int BE_pid;
    pthread_t run_thread;

    NetworkController();
    ~NetworkController();

    void set_LC_procs(int pid);
    void set_BE_procs(int pid);
    static void* run(void* arg);

};

#endif