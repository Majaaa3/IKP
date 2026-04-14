#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <mutex>
#include <condition_variable>
#include <thread> 
#include <condition_variable>  

struct Task;

struct TaskNode {
    Task* task;
    TaskNode* sledeci;
};

struct TaskQueue {
    TaskNode* head;
    TaskNode* tail;

    std::mutex mtx;
    std::condition_variable cv;

    TaskQueue();

    void push(Task* task);
    Task* pop();
};

#endif
