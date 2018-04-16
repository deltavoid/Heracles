#include "cpu_driver.h"
#include "helpers.h"
#include "pqos.h"
#include <cassert>
#include <csignal>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

CpuDriver::CpuDriver(Tap *t, CacheDriver *cd) : tap(t), cc_d(cd) {
    path = get_opt<std::string>("CGROUPS_DIR", "/sys/fs/cgroups");
    assert(init_cgroups_dir() == true);
    // init cgroups VFS

    assert(init_core_num() == true);
    // init core nums

    if (!update()) {
        print_err("[CPU_DRIVER] CpuDriver() init update failed.");
        exit(-1);
    }

    if (!cc_d->update_association(BE_cores, sys_cores, total_cores)) {
        print_err("[CPU_DRIVER] CpuDriver() init cache_driver failed.");
        exit(-1);
    }
}

bool CpuDriver::init_cgroups_dir() {
    std::string paths[2] = {path + "/cpuset/LC", path + "/cpuset/BE"};
    for (int i = 0; i < 2; ++i) {
        int res = access(paths[i].c_str(), F_OK);
        if (res == 0) {
            res = rmdir(paths[i].c_str());
            if (res != 0) {
                print_err("[CPU_DRIVER] can't remove existed cgroups_cpuset dir.");
                return false;
            }
        }
        res = mkdir(paths[i].c_str(),
                    S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        if (res != 0) {
            print_err("[CPU_DRIVER] can't create new cgroups_cpuset dir.");
            return false;
        }
    }
    return true;
}

bool CpuDriver::init_core_num() {

    if (!cc_d->intel_init()) {
        return false;
    }

    size_t *sockets, sock_count;
    sockets = pqos_cpu_get_sockets(p_cpu, &sock_count);
    if (sockets == NULL) {
        print_err("[CPUDRIVER] error retrieving CPU socket information.\n");
        return false;
    }
    assert(&sock_count >= 1);
    // assert: sock_count >= 1

    unsigned *lcores = NULL;
    unsigned lcount = 0;
    lcores = pqos_cpu_get_cores(p_cpu, sockets[0], &lcount);
    if (lcores == NULL || lcount == 0) {
        print_err("[CPUDRIVER] error retrieving core information.\n");
        free(lcores);
        free(sockets);
        return false;
    }
    // get core_count on socket 0 (this program is only for single socket structure)

    BE_cores = 0;
    sys_cores = get_opt<size_t>("HERACLES_IDLE_CORE_NUM", 1);
    total_cores = lcount;

    assert(total_cores > sys_cores + 2);

    if(!cc_d->update_association(BE_cores, sys_cores, total_cores)) {
        return false;
    }
    // set core_num and check bounds, update core CLOS mapping

    if (!cc_d->intel_fini()) {
        return false;
    }
    return true;
}

bool CpuDriver::set_cores_for_pid(size_t type, size_t left, size_t right) {
    std::string p;
    if (type == OPT_LC)
        p = path + "/cpuset/LC";
    else if (type == OPT_BE)
        p = path + "/cpuset/BE";
    else
        return false;

    std::ofstream proc;
    proc.open((p + "/procs").c_str(), std::ios::out);
    if (!proc) {
        print_err("[CPU_DRIVER] can't echo to cpuset.procs file.");
        return false;
    }
    proc << BE_pid << std::endl;
    proc.close();

    std::ofstream cpu_file;
    cpu_file.open((p + "/cpus").c_str(), std::ios::out);
    if (!cpu_file) {
        print_err("[CPU_DRIVER] can't echo to cpuset.cpus file.");
        return false;
    }
    cpu_file << left << "-" << right << std::endl;
    cpu_file.close();

    std::ofstream ex_file;
    ex_file.open((p + "/cpu_exclusive").c_str(), std::ios::out);
    if (!ex_file) {
        print_err("[CPURDRIVER] can't echo to cpuset.exclusive file.");
        return false;
    }
    ex_file << 1 << std::endl;
    ex_file.close();
    return true;
}

size_t CpuDriver::total_core_num() const { return total_cores; }

bool CpuDriver::update() {
    if (tap->LC_pid() == -1)
        return false;
    if (!set_cores_for_pid(OPT_BE, 0, BE_cores - 1)) {
        print_err("[CPU_DRIVER] update() set BE cores error.");
        return false;
    }
    if (!set_cores_for_pid(OPT_LC, BE_cores, total_cores - sys_cores - 1)) {
        print_err("[CPU_DRIVER] update() set LC cores error.");
        return false;
    }
    if (!cc_d->update_association(BE_cores, sys_cores, total_cores)) {
        print_err("[CPU_DRIVER] update() init cache_driver failed.");
        exit(-1);
    }
    return true;
}

void CpuDriver::clear() {
    BE_cores = 0;
    update();
}

bool CpuDriver::BE_cores_inc(size_t inc) {
    size_t tmp = BE_cores;
    if (BE_cores + inc >= total_cores - sys_cores - 1) {
        return false;
    }
    BE_cores += inc;
    if (update()) {
        return true;
    } else {
        BE_cores = tmp;
        return false;
    }
}

bool CpuDriver::BE_cores_dec(size_t dec) {
    size_t tmp = BE_cores;
    if (dec >= BE_cores - sys_cores - 1) {
        BE_cores = 0;
    } else {
        BE_cores -= dec;
    }
    if (update()) {
        return true;
    } else {
        BE_cores = tmp;
        return false;
    }
}
