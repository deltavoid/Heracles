#include "network_controller.h"
#include <iostream>


NetworkController::NetworkController(Tap* tap) : tap(tap)
{
    default_device = "ens33";
    default_bandwidth = 1e9; //bits
    usable_bandwidth = default_bandwidth - default_bandwidth / 10;
    print_log("[NET] default_bandwidth: %llu", default_bandwidth);
    print_log("[NET] usable_bandwidth: %llu", usable_bandwidth);

    network_driver = new NetworkDriver(default_device, default_bandwidth);
    network_monitor = new NetworkMonitor(default_device);

    //int res = pthread_create(&run_thread, NULL, run, (void*)this);

}

NetworkController::~NetworkController()
{
    pthread_cancel(run_thread);
    delete network_monitor;
    delete network_driver;
}

void NetworkController::set_LC_procs(int pid)
{
    if  (LC_pid == pid)  return ;
    LC_pid = pid;
    network_driver->set_LC_procs(pid);
    print_log("[NET] set_LC_procs: %d", pid);
}

void NetworkController::set_BE_procs(int pid)
{
    if  (BE_pid == pid)  return ;
    BE_pid = pid;
    network_driver->set_BE_procs(pid);
    print_log("[NET] set_BE_procs: %d", pid);
}

void* NetworkController::run(void* arg)
{
    pthread_detach(pthread_self());
    NetworkController* This = (NetworkController*) arg;

    while (true)
    {
        This->set_LC_procs(This->tap->LC_pid());
        This->set_BE_procs(This->tap->BE_pid());

        sleep(1);

        u64 LC_bandwidth = This->network_monitor->get_class_bytes(This->network_driver->LC_classid);
        u64 BE_bandwidth = This->network_monitor->get_class_bytes(This->network_driver->BE_classid);
        print_log("[NET] LC_bandwidth: %llu  BE_bandwidth: %llu", LC_bandwidth, BE_bandwidth);

        u64 new_bandwidth = LC_bandwidth < This->usable_bandwidth? This->usable_bandwidth - LC_bandwidth : 1;
        print_log("[NET] set BE bandwidth: %llu", new_bandwidth);
        This->network_driver->set_BE_bandwidth(new_bandwidth);

    }
}