#include <cstdlib>
#include <ctime>
#include <iostream>
#include <pthread.h>

using namespace std;

void usage() {
    cerr << endl
         << "Usage: stream-llc-multithread [thread_num] [cache_space(KB)] [time(second)]" << endl
         << endl
         << "PS: cache_space must be a multiple of two." << endl
         << "    time must be a positive integer." << endl
         << endl;
}

struct Params {
    int *data;
    long long int length;
    int t;
    Params(int* _d, long long int _l, int _t): data(_d), length(_l), t(_t) {}
};

void interfere(int *data, long long int length, int t) {
    srand((unsigned)time(0));
    time_t begin = time(0);
    time_t end = time(0);
    while (end - begin < t) {
        long long int i = 0;
        while (i < length) {
            data[i] += 1;
            i += 16;
        }
        end = time(0);
    }
}

void* thread_entry(void* params) {
    Params* para = (Params*) params;
    interfere(para->data, para->length, para->t);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        usage();
        return -1;
    }

    int thread_num = atoi(argv[1]);
    int cache_space = atoi(argv[2]);
    int t = atoi(argv[3]);

    if (thread_num <= 0 || cache_space % 2 != 0 || time <= 0) {
        usage();
        return -1;
    }

    cout << thread_num << " threads | " << cache_space << " KB | " << t << " s" << endl;

    pthread_t tids[thread_num];
    long long int length = (long long int)cache_space * 1024 / 4;
    cout << "array length: " << length << endl;
    int *data = new int[length];

    Params params(data, length, t);

    for(int i = 0; i < thread_num; ++i) {
        int ret = pthread_create(&tids[i], NULL, thread_entry, &params);
        if (ret != 0) {
            cerr << "pthread_create failure. errno = " << endl;
            return -1;
        }
    }
    //cout << "interfering..." << endl;

    for(int i = 0; i < thread_num; ++i) {
	pthread_join(tids[i], NULL);
    }

    delete[] data;

    return 0;
}
