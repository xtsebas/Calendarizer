#include "semaphore_sync.h"

SemaphoreSynchronizer::SemaphoreSynchronizer(int initial_count)
    : count(initial_count)
{}

// lock bloqueante que respeta orden FIFO de thread_id
void SemaphoreSynchronizer::lock(int thread_id) {
    {
        // Paso 1: encolar nuestro thread_id en espera
        std::lock_guard<std::mutex> guard(mtx);
        waiting_queue.push(thread_id);
    }

    // Paso 2: esperar hasta que (a) haya recurso disponible y (b) sea nuestro turno (front de la cola)
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this, thread_id]() {
        return (count > 0
                && !waiting_queue.empty()
                && waiting_queue.front() == thread_id);
    });

    // Paso 3: Ya puedo tomar el semáforo
    waiting_queue.pop();
    --count;
}

// try_lock no bloqueante: si count>0 y no hay hilos FIFO por delante, tomo el recurso; si no, devuelvo false.
bool SemaphoreSynchronizer::try_lock(int thread_id) {
    std::lock_guard<std::mutex> guard(mtx);
    // Solo permitimos "tomar" si hay recursos y además no hay cola de espera anterior
    if (count > 0 && waiting_queue.empty()) {
        --count;
        return true;
    }
    return false;
}

// unlock: incrementa contador y notifica a todos, pero solo el que esté primero en cola podrá continuar
void SemaphoreSynchronizer::unlock(int thread_id) {
    {
        std::lock_guard<std::mutex> guard(mtx);
        ++count;
    }
    cv.notify_all();
}
