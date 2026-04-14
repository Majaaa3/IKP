#include "task.h"
#include "../model/subscriber.h" 

Task::Task(Subscriber* subskrajber, const std::string& poruka):subskrajber(subskrajber), poruka(poruka){}

void Task::execute(){
    if(subskrajber!=nullptr){
        subskrajber->primiPoruku(poruka);
    }
}