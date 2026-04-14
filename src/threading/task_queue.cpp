#include "task_queue.h"
#include "task.h"

TaskQueue::TaskQueue() : head(nullptr), tail(nullptr) {}

void TaskQueue::push(Task* task) {
    std::unique_lock<std::mutex> lock(mtx);
    
    TaskNode* newNode = new TaskNode();
    newNode->task = task;
    newNode->sledeci = nullptr;

    if(tail == nullptr) {
        head = tail = newNode;
    } else {
        tail->sledeci = newNode;
        tail = newNode;
    }

    cv.notify_one();
}

Task* TaskQueue::pop() {
    std::unique_lock<std::mutex> lock(mtx);
    
    cv.wait(lock, [this] { return head != nullptr; });

    if(head == nullptr) {
        return nullptr;
    }

    TaskNode* node = head;
    Task* task = node->task;
    head = head->sledeci;

    if(head == nullptr) {
        tail = nullptr;
    }

    delete node;
    return task;
}