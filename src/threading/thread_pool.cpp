#include "thread_pool.h"
#include "task.h"
#include <mutex>       
#include <thread> 

ThreadPool::ThreadPool(int numThreads) 
    : numThreads(numThreads), shouldStop(false) {
    
    taskQueue = new TaskQueue();
    
    threads = new std::thread[numThreads];
    
    for(int i = 0; i < numThreads; i++) {
        threads[i] = std::thread(&ThreadPool::workerLoop, this);
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
    delete taskQueue;
    delete[] threads;  
}

void ThreadPool::submit(Task* task) {
    if(!shouldStop) {
        taskQueue->push(task);
    }
}

void ThreadPool::shutdown() {
    shouldStop = true;
    
    for(int i = 0; i < numThreads; i++) {
        taskQueue->cv.notify_all();
    }
    
    for(int i = 0; i < numThreads; i++) {
        if(threads[i].joinable()) {
            threads[i].join();
        }
    }
}

void ThreadPool::workerLoop() {
    while(!shouldStop) {
        std::unique_lock<std::mutex> lock(taskQueue->mtx);
        
        taskQueue->cv.wait(lock, [this]() {
            return taskQueue->head != nullptr || shouldStop;
        });
        
        if(shouldStop && taskQueue->head == nullptr) {
            break;
        }
        
        Task* task = nullptr;
        if(taskQueue->head != nullptr) {
            TaskNode* node = taskQueue->head;
            task = node->task;
            taskQueue->head = node->sledeci;
            if(taskQueue->head == nullptr) {
                taskQueue->tail = nullptr;
            }
            delete node;
        }
        
        lock.unlock();
        
        if(task != nullptr) {
            task->execute();
            delete task;
        }
    }
}