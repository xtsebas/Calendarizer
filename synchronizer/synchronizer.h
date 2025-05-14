#ifndef SYNCHRONIZER_H
#define SYNCHRONIZER_H

class Synchronizer {
public:
    virtual ~Synchronizer() {}

    // Llamado antes de entrar a la sección crítica
    virtual void lock(int thread_id) = 0;

    // Llamado después de salir de la sección crítica
    virtual void unlock(int thread_id) = 0;
};

#endif 