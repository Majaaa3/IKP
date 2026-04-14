#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <atomic>
#include "task_queue.h"
#include <mutex> 

struct ThreadPool {
    std::thread* threads;   
    int numThreads;         
    TaskQueue* taskQueue;
    std::atomic<bool> shouldStop;

    ThreadPool(int numThreads);
    ~ThreadPool();

    void submit(Task* task);
    void shutdown();

private:
    void workerLoop();
};

#endif