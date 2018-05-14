#include "cache_driver.h"
#include "helpers.h"
#include "pqos.h"
#include <cstring>

#define SYS_CLASS_ID (0)
#define LC_CLASS_ID (1)
#define BE_CLASS_ID (2)

CacheDriver::CacheDriver() {
    if (!intel_init()) {
        exit(-1);
    }

    const struct pqos_cpuinfo *p_cpu = nullptr;
    const struct pqos_cap *p_cap = nullptr;

    int ret = pqos_cap_get(&p_cap, &p_cpu);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[CACHEDRIVER] error retrieving PQoS capabilities.");
        exit(-1);
    }
    // check CMT capability and CPU info pointer

    ret = -1;
    const struct pqos_capability *l3ca = nullptr;
    (void)pqos_cap_get_type(p_cap, PQOS_CAP_TYPE_L3CA, &l3ca);
    print_log("[CACHEDRIVER] num_ways: %u", l3ca->u.l3ca->num_ways);
    print_log("[CACHEDRIVER] num_classes: %u", l3ca->u.l3ca->num_classes);
    print_log("[CACHEDRIVER] way_size: %u", l3ca->u.l3ca->way_size);
    min_bits = l3ca->u.l3ca->num_ways;
    ret = PQOS_RETVAL_OK;

    // min_bits should be 8/16/32 for most machines

    if (ret != PQOS_RETVAL_OK) {
        print_err("[CACHEDRIVER] error getting min_bits for CLOS.");
    } else {
        init_masks();
    }
    // get min_bits for CAT settings and init all masks
    // e.g. min_bits = 16, LC_mask = 0xfffc, sys_mask = 0x0003, BE_mask = 0

    if (!intel_fini() || ret != PQOS_RETVAL_OK) {
        exit(-1);
    }

    print_log("[CACHEDRIVER] LC_mask:0x%5x BE_mask: 0x%05x sys_mask:0x%05x "
              "min_bits:%d",
              LC_mask, BE_mask, sys_mask, min_bits);
    print_log("[CACHEDRIVER] inited.");
}

void CacheDriver::init_masks() {
    sys_bits = min_bits / 8;
    BE_bits = 0;
    LC_bits = min_bits - sys_bits;

    update_masks();
}

bool CacheDriver::update_association(size_t BE_core_num, size_t sys_core_num,
                                     size_t total_core_num) {
    if (!intel_init()) {
        return false;
    }

    int ret;
    for (size_t i = 0; i < total_core_num; ++i) {
        if (0 <= i && i < BE_core_num) {
            ret = pqos_alloc_assoc_set(i, LC_CLASS_ID);
        } else if (BE_core_num <= i && i < total_core_num - sys_core_num) {
            ret = pqos_alloc_assoc_set(i, BE_CLASS_ID);
        } else {
            ret = pqos_alloc_assoc_set(i, SYS_CLASS_ID);
        }
        if (ret != PQOS_RETVAL_OK) {
            print_err("[CACHEDRIVER] update_association() setting allocation "
                      "class of service "
                      "association failed.");
            return false;
        }
    }
    // set core [0, BE-1]:              for LC  class - CLOS 1
    // set core [BE, total-sys-1]:      for BE  class - CLOS 2
    // set core [total-sys, total-1]:   for sys class - CLOS 0 (default CLOS)

    if (!intel_fini()) {
        return false;
    }
    return true;
}

bool CacheDriver::update_allocation() {
    if (!intel_init()) {
        return false;
    }
    unsigned sock_count, *sockets = NULL;
    const struct pqos_cpuinfo *p_cpu = NULL;
    const struct pqos_cap *p_cap = NULL;

    int ret = pqos_cap_get(&p_cap, &p_cpu);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[CACHEDRIVER] error retrieving PQoS capabilities!");
        return false;
    }

    sockets = pqos_cpu_get_sockets(p_cpu, &sock_count);
    if (sockets == NULL) {
        print_err("[CACHEDRIVER] update_allocation() error retrieving CPU "
                  "socket information.");
        return false;
    }

    pqos_l3ca tab[3];

    tab[0].class_id = SYS_CLASS_ID;
    tab[0].cdp = 0;
    tab[0].u.ways_mask = sys_mask;

    tab[1].class_id = BE_CLASS_ID;
    tab[1].cdp = 0;
    tab[1].u.ways_mask = BE_mask;

    tab[2].class_id = LC_CLASS_ID;
    tab[2].cdp = 0;
    tab[2].u.ways_mask = LC_mask;

    print_log("[CACHEDRIVER] allocation: LC_mask:0x%5x BE_mask: 0x%05x "
              "sys_mask:0x%05x",
              LC_mask, BE_mask, sys_mask);
    print_log("[CACHEDRIVER] allocation: LC_bits:%u BE_bits: %u sys_bits: %u",
              LC_bits, BE_bits, sys_bits);

    ret = pqos_l3ca_set(*sockets, 3, tab);
    if (ret != PQOS_RETVAL_OK) {
        print_err(
            "[CACHEDRIVER] update_allocation() error setting CLOS masks.");
    }

    if (!intel_fini()) {
        return false;
    }
    return true;
}

bool CacheDriver::BE_cache_grow() {
    size_t max_BE_bits = min_bits - sys_bits - 1;
    if (BE_bits + 1 >= max_BE_bits) {
        BE_bits = max_BE_bits;
    } else {
        BE_bits += 1;
    }

    size_t min_LC_bits = 1;
    if (LC_bits - 1 <= min_LC_bits) {
        LC_bits = 1;
    } else {
        LC_bits -= 1;
    }

    update_masks();
    if (!update_allocation())
        return false;

    return true;
}

bool CacheDriver::BE_cache_roll_back() {
    size_t min_BE_bits = 1;
    if (BE_bits > min_BE_bits) {
        BE_bits -= 1;
    }

    size_t max_LC_bits = min_bits - sys_bits - 1;
    if (LC_bits + 1 >= max_LC_bits) {
        LC_bits = max_LC_bits;
    } else {
        LC_bits += 1;
    }

    update_masks();
    if (!update_allocation())
        return false;

    return true;
}

void CacheDriver::update_masks() {
    /*
     *  CLOS mask layout:
     *  +------------------------------------+
     *  | BE(CLOS2) | LC(CLOS1) | SYS(CLOS0) |
     *  +------------------------------------+
     */

    uint64_t mask = 0;
    for (size_t i = 0; i < sys_bits; ++i)
        mask = (mask << 1) + 1;
    sys_mask = mask;

    mask = 0;
    for (size_t i = 0; i < LC_bits; ++i)
        mask = (mask << 1) + 1;
    mask = mask << sys_bits;
    LC_mask = mask;

    mask = 0;
    for (size_t i = 0; i < BE_bits; ++i)
        mask = (mask << 1) + 1;
    mask = mask << (sys_bits + LC_bits);
    BE_mask = mask;
}

void CacheDriver::clear() { init_masks(); }