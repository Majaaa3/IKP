#ifndef TASK_H
#define TASK_H

#include <string>

struct Subscriber;

struct Task{
    Subscriber* subskrajber;
    std::string poruka;

    Task(Subscriber* subskrajber, const std::string& poruka);

    void execute();
};

#endif