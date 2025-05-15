#include "mutex_sync.h"

void MutexSynchronizer::acquire() {
    while (lock_flag.test_and_set(std::memory_order_acquire)) {
        std::this_thread::yield(); // Espera activa
    }
}

void MutexSynchronizer::release() {
    lock_flag.clear(std::memory_order_release);
}

void MutexSynchronizer::lock(int thread_id) {
    acquire();
}

void MutexSynchronizer::unlock(int thread_id) {
    release();
}