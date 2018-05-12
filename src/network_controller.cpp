#include "network_controller.h"
#include <iostream>


NetworkController::NetworkController()
{
    default_device = "eno1";
    default_bandwidth = 1e9 / 8; //bytes
    usable_bandwidth = default_bandwidth - default_bandwidth / 10;

    network_driver = new NetworkDriver("heracles", default_device, default_bandwidth);
    network_monitor = new NetworkMonitor(default_device);

    int res = pthread_create(&run_thread, NULL, run, (void*)this);

}

NetworkController::~NetworkController()
{
    pthread_cancel(run_thread);
    delete network_monitor;
    delete network_driver;
}

void NetworkController::set_LC_procs(int pid)
{
    LC_pid = pid;
    network_driver->set_LC_procs(pid);
    std::cout << "NetworkController set_LC_procs " << LC_pid << std::endl;
}

void NetworkController::set_BE_procs(int pid)
{
    BE_pid = pid;
    network_driver->set_BE_procs(pid);
    std::cout << "NetworkController set_BE_procs " << BE_pid << std::endl;
}

void* NetworkController::run(void* arg)
{
    pthread_detach(pthread_self());
    NetworkController* This = (NetworkController*) arg;

    while (true)
    {
        sleep(1);

        u64 LC_bandwidth = This->network_monitor->get_class_bytes(This->network_driver->LC_classid);
        u64 BE_bandwidth = This->network_monitor->get_class_bytes(This->network_driver->BE_classid);
        std::cout << " LC_bandwidth: " << LC_bandwidth 
                  << " BE_bandwidth: " << BE_bandwidth
                  << std::endl;


        if  (LC_bandwidth < This->usable_bandwidth)
        {
            This->network_driver->set_BE_bandwidth(This->usable_bandwidth - LC_bandwidth);
        }
        else
        {
            This->network_driver->set_BE_bandwidth(1);
        }

    }

}