#include "semaphore_sync.h"

SemaphoreSynchronizer::SemaphoreSynchronizer(int initial_count)
    : count(initial_count)
{}

void SemaphoreSynchronizer::lock(int thread_id) {
    std::unique_lock<std::mutex> lock(mtx);
    // Espera hasta que haya “count > 0”
    cv.wait(lock, [this]() { return count > 0; });
    // Una vez que count > 0, decrementamos para 'adquirir' el semáforo
    --count;
}

void SemaphoreSynchronizer::unlock(int thread_id) {
    {
        std::unique_lock<std::mutex> lock(mtx);
        // Liberamos (incrementamos count)
        ++count;
    }
    // Notificamos a un thread que esté en espera
    cv.notify_one();
}
