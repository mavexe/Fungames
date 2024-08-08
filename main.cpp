#include "main.h"
#include <stdio.h>
#include <iostream>
Point::Point(double x, double y) : x(x), y(y) {}
Point::~Point() : std::cout<<"answer is"<<y-x;

int main(){
    Point p(3,5);
    std::cout<<p.getX;
    return 0;
}


