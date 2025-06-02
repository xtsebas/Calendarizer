#ifndef SEMAPHORE_SYNC_H
#define SEMAPHORE_SYNC_H

#include "synchronizer.h"
#include <mutex>
#include <condition_variable>

class SemaphoreSynchronizer : public Synchronizer {
public:
    // Por defecto, semáforo binario (count = 1).
    // Si quisieras un semáforo con conteo mayor, puedes pasar initial_count.
    explicit SemaphoreSynchronizer(int initial_count = 1);

    // lock() bloquea al thread hasta que count > 0, luego decrementa count.
    void lock(int thread_id) override;

    // unlock() incrementa count y notifica a un thread en espera.
    void unlock(int thread_id) override;

private:
    std::mutex              mtx;
    std::condition_variable cv;
    int                     count;
};

#endif // SEMAPHORE_SYNC_H
