#include "helpers.h"
#include <cstdarg>
#include <cstring>
#include <pqos.h>
#include <string>
#include <pthread.h>

bool is_debug = false;
bool is_log = false;

void set_debug(bool d) { is_debug = d; }

void set_log(bool l) { is_log = l; }

void print_err(const char *fmt, ...) {
    if (is_debug) {
        std::cout << "[ERROR] - ";
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        std::cout << std::endl;
    }
}

void print_log(const char *fmt, ...) {
    if (is_log) {
        std::cout << "[ LOG ] - ";
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        std::cout << std::endl;
    }
}

pthread_mutex_t mutex;

void intel_mutex_init() {
    pthread_mutex_init(&mutex, nullptr);
}

bool intel_init(bool mbm) {
    pthread_mutex_lock(&mutex);
    pqos_config cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.fd_log = STDOUT_FILENO;
    cfg.verbose = 0;
    if (mbm) {
        cfg.interface = PQOS_INTER_MSR;
    }
    /* PQoS Initialization - Check and initialize CAT and CMT capability */
    int ret = pqos_init(&cfg);
    if (ret != PQOS_RETVAL_OK) {
        print_err("[FUNC] intel_init() error initializing PQoS library.\n");
        pthread_mutex_unlock(&mutex);
        return false;
    }
    return true;
}

bool intel_fini() {
    int ret = pqos_fini();
    if (ret != PQOS_RETVAL_OK) {
        print_err("[FUNC] intel_fini() error shutting down PQoS library.\n");
        pthread_mutex_unlock(&mutex);
        return false;
    }
    pthread_mutex_unlock(&mutex);
    return true;

}