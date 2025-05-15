#ifndef MUTEX_SYNC_H
#define MUTEX_SYNC_H

#include "synchronizer.h"
#include <atomic>
#include <thread>

class MutexSynchronizer : public Synchronizer {
private:
    std::atomic_flag lock_flag = ATOMIC_FLAG_INIT;

    void acquire();
    void release();

public:
    void lock(int thread_id) override;
    void unlock(int thread_id) override;
};

#endif