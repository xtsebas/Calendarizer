#ifndef SEMAPHORE_SYNC_H
#define SEMAPHORE_SYNC_H

#include "synchronizer.h"
#include <mutex>
#include <condition_variable>
#include <queue>

class SemaphoreSynchronizer : public Synchronizer {
public:
    // Por defecto, semáforo binario (count = 1).
    explicit SemaphoreSynchronizer(int initial_count = 1);

    // lock() bloquea al thread hasta que count > 0 y sea su turno FIFO.
    void lock(int thread_id) override;

    // try_lock() no bloqueante: regresa true si pudo decrementar count, false en otro caso.
    bool try_lock(int thread_id) override;

    // unlock() incrementa count y notifica a hilos en espera.
    void unlock(int thread_id) override;

private:
    std::mutex              mtx;
    std::condition_variable cv;
    int                     count;
    std::queue<int>         waiting_queue;
};

#endif // SEMAPHORE_SYNC_H
