#ifndef SYNCHRONIZER_PETERSON_H
#define SYNCHRONIZER_PETERSON_H

#include "synchronizer.h"
#include <atomic>

class PetersonSynchronizer : public Synchronizer {
private:
    std::atomic<bool> flag[2];
    std::atomic<int> turn;

public:
    PetersonSynchronizer();
    void lock(int thread_id) override;
    void unlock(int thread_id) override;
};

#endif 