#include <iostream>
#include <stdio.h>
#include <vector>

class Point {

    Point(){
    printf("Its conctructor");
    }

    void print(){
        printf("its method");
    }
};

int main(){
    Point point1;
    point1.print();

}
