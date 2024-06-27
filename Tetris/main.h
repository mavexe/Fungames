
#include <iostream>
#include <vector>
#include "stdio.h"
#include<iostream>
#include "string"
#include <vector>
#include <ncurses.h>
#include <stdlib.h>
#include <ctime>

const int WIDTH = 20;
const int WEIGHT =20;

const int DROP_INTERVAL = 500000*2;
#pragma once

#ifndef WRAPPED_NCURSES.H
#define WRAPPED_NCURSES_H
#include <ncurses.h>
namespace ncurses{
    using::move;
}
#endif

bool canMove(int dx, int dy);
void draw();
void rotate();
void clearLines();
bool isBlockAbottomorAnotherBlock();

