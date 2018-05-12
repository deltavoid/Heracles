#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include "network_driver.h"
using namespace std;



const string NetworkDriver::cgroup_path = "/sys/fs/cgroup/net_cls/";
const string NetworkDriver::procs_file = "cgroup.procs";
const string NetworkDriver::classid_file = "net_cls.classid";



bool NetworkDriver::init_dir(string path)
{
    int res = access(path.c_str(), F_OK);
    if  (res == 0)
    {   res = rmdir(path.c_str());
        if  (res != 0)
        {   //print_error();
            return false;
        } 
    }

    res = mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    if  (res != 0)
    {   //print_err();
        return false;
    }

    return true;
}

bool NetworkDriver::write_to_file(string file_path, string content)
{
    ofstream file;
    file.open(file_path.c_str(), std::ios::out);
    if  (!file)
    {   //print_err();
        return false;
    }
    
    file << content << endl;
    file.close();

    return true;
}



NetworkDriver::NetworkDriver(string name, string _device, u64 rate)
{
    device = _device;
    LC_path = cgroup_path + name + string("/LC/");
    BE_path = cgroup_path + name + string("/BE/");
    init_dir(cgroup_path);
    init_dir(LC_path);
    init_dir(BE_path);

    LC_classid = 0x10003;
    BE_classid = 0x10004;
    write_to_file(LC_path + classid_file, to_string(LC_classid));
    write_to_file(BE_path + classid_file, to_string(BE_classid));
    

    system((string("tc qdisc del dev ") + device + string(" root")).c_str());
    system((string("tc qdisc add dev ") + device + string(" root handle 1: htb")).c_str());

    rate *= 8;
    system((string("tc class add ") + 
            string(" dev ") + device + 
            string(" parent 1: ") + 
            string(" classid 1: ") + 
            string(" htb rate ") + to_string(rate) + 
            string(" ceil ") + to_string(rate)
            ).c_str());

    system((string("tc class add ") + 
            string(" dev ") + device + 
            string(" parent 1: ") + 
            string(" classid 1:") + to_string(LC_classid % (1 << 16)) + 
            string(" htb rate ") + to_string(rate)
            ).c_str());

    system((string("tc class add ") + 
            string(" dev ") + device + 
            string(" parent 1: ") + 
            string(" classid 1:") + to_string(BE_classid % (1 << 16)) + 
            string(" htb rate ") + to_string(rate)
            ).c_str());
    
    system((string("tc filter add dev ") + device + string(" protocol ip parent 1:0 prio 1 handle 1: cgroup")).c_str());

}

NetworkDriver::~NetworkDriver()
{
}

void NetworkDriver::set_LC_procs(int pid)
{
    write_to_file(LC_path + procs_file, to_string(pid));
}

void NetworkDriver::set_LC_bandwidth(u64 bw)
{
    bw *= 8;
    system((string("tc class change ") + 
            string(" dev ") + device + 
            string(" parent 1: ") + 
            string(" classid 1:") + to_string(LC_classid % (1 << 16)) + 
            string(" htb rate ") + to_string(bw)
            ).c_str());
}

void NetworkDriver::set_BE_procs(int pid)
{
    write_to_file(BE_path + procs_file, to_string(pid));
}

void NetworkDriver::set_BE_bandwidth(u64 bw)
{
    cout << "set bandwidth: " << bw << endl;
    bw *= 8;
    system((string("tc class change ") + 
            string(" dev ") + device + 
            string(" parent 1: ") + 
            string(" classid 1:") + to_string(BE_classid % (1 << 16)) + 
            string(" htb rate ") + to_string(bw)
            ).c_str());
}