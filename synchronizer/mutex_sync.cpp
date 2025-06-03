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

bool MutexSynchronizer::try_lock(int thread_id) {
    // Si no estaba bloqueado, lo bloqueamos y retornamos true.
    // Si ya estaba bloqueado, retornamos false sin bloquear.
    return !lock_flag.test_and_set(std::memory_order_acquire);
}


void MutexSynchronizer::unlock(int thread_id) {
    release();
}