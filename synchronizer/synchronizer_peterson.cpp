#include "synchronizer_peterson.h"
#include <thread>

PetersonSynchronizer::PetersonSynchronizer() {
    flag[0] = false;
    flag[1] = false;
    turn = 0;
}

void PetersonSynchronizer::lock(int thread_id) {
    int other = 1 - thread_id;
    flag[thread_id] = true;
    turn = other;

    while (flag[other] && turn == other) {
        std::this_thread::yield();
    }

}

void PetersonSynchronizer::unlock(int thread_id) {
    flag[thread_id] = false;

}
