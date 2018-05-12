#ifndef NETWORK_MONITOR_H
#define NETWORK_MONITOR_H

#include <string>
#include <vector>
#include <map>
#include <pthread.h>
#include "BPF.h"

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned long long u64;

struct info_t
{   u32 classid;
    char name[16];
};

class NetworkMonitor
{public:
    NetworkMonitor(std::string _device);
    ~NetworkMonitor();

    static void* run(void* arg);

    u64 get_class_bytes(u32 classid);

    ebpf::BPF* bpf;
    static const std::string BPF_PROGRAM;
    std::string device;
    pthread_t run_thread;

    std::vector<std::pair<info_t, u64> > class_dev_bytes;
    std::map<u32, u64> class_bytes;

};

#endif