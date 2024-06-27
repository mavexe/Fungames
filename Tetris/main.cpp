#include <iostream>
#include <vector>
#include <ctime>
#include <ncurses.h>

using namespace std;

const int width = 10;
const int height = 20;
const char emptyy = ' ';
const char blockLeft = '[';
const char blockRight = ']';

struct Point {
    int x, y;
};

class Tetris {
private:
    vector<vector<char>> field;
    vector<Point> currentBlock;
    int score;
    bool gameOver;

    const vector<vector<Point>> shapes = {
        {{0, 1}, {1, 1}, {2, 1}, {3, 1}},  // I
        {{1, 0}, {1, 1}, {1, 2}, {2, 2}},  // L
        {{1, 0}, {1, 1}, {1, 2}, {0, 2}},  // J
        {{0, 1}, {1, 1}, {2, 1}, {1, 2}},  // T
        {{1, 0}, {2, 0}, {1, 1}, {2, 1}},  // O
        {{0, 1}, {1, 1}, {1, 2}, {2, 2}},  // Z
        {{2, 1}, {1, 1}, {1, 2}, {0, 2}}   // S
    };

public:
    Tetris() {
        field = vector<vector<char>>(height, vector<char>(width, emptyy));
        score = 0;
        gameOver = false;
        spawnBlock();
    }

    void spawnBlock() {
        int shapeIndex = rand() % shapes.size();
        currentBlock = shapes[shapeIndex];

        for (auto &p : currentBlock) {
            p.x += width / 2 - 1;
            p.y += 0;
            if (field[p.y][p.x] != emptyy) {
                gameOver = true;
            }
        }
    }

    void draw() {
        clear();
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                bool isBlock = false;
                for (auto &p : currentBlock) {
                    if (p.x == j && p.y == i) {
                        isBlock = true;
                        break;
                    }
                }
                if (isBlock) {
                    mvaddch(i, j * 2, blockLeft);
                    mvaddch(i, j * 2 + 1, blockRight);
                } else {
                    mvaddch(i, j * 2, field[i][j]);
                    mvaddch(i, j * 2 + 1, field[i][j]);
                }
            }
        }
        mvprintw(height + 1, 0, "Score: %d", score);
        if (gameOver) {
            mvprintw(height + 2, 0, "Game Over");
        }
        refresh();
    }

    bool isCollision(vector<Point> newBlock) {
        for (auto &p : newBlock) {
            if (p.x < 0 || p.x >= width || p.y >= height || field[p.y][p.x] != emptyy) {
                return true;
            }
        }
        return false;
    }

    void placeBlock() {
        for (auto &p : currentBlock) {
            field[p.y][p.x] = blockLeft;  // Only need to set one of the pair since they are identical
        }
        clearLines();
        spawnBlock();
    }

    void clearLines() {
        for (int i = 0; i < height; ++i) {
            bool fullLine = true;
            for (int j = 0; j < width; ++j) {
                if (field[i][j] == emptyy) {
                    fullLine = false;
                    break;
                }
            }
            if (fullLine) {
                for (int k = i; k > 0; --k) {
                    for (int j = 0; j < width; ++j) {
                        field[k][j] = field[k - 1][j];
                    }
                }
                for (int j = 0; j < width; ++j) {
                    field[0][j] = emptyy;
                }
                score += 100;
            }
        }
    }

    void moveBlock(int dx, int dy) {
        vector<Point> newBlock = currentBlock;
        for (auto &p : newBlock) {
            p.x += dx;
            p.y += dy;
        }
        if (!isCollision(newBlock)) {
            currentBlock = newBlock;
        } else if (dy != 0 && dx == 0) {
            placeBlock();
        }
    }

    void rotateBlock() {
        vector<Point> newBlock = currentBlock;
        Point pivot = currentBlock[1];  // Pivot around the second block
        for (auto &p : newBlock) {
            int x = p.y - pivot.y;
            int y = p.x - pivot.x;
            p.x = pivot.x - x;
            p.y = pivot.y + y;
        }
        if (!isCollision(newBlock)) {
            currentBlock = newBlock;
        }
    }

    void run() {
        int ch;
        timeout(500);  // Set a delay for automatic downward movement
        while ((ch = getch()) != 'q' && !gameOver) {
            switch (ch) {
                case KEY_LEFT:
                    moveBlock(-1, 0);
                    break;
                case KEY_RIGHT:
                    moveBlock(1, 0);
                    break;
                case KEY_DOWN:
                    moveBlock(0, 1);
                    break;
                case KEY_UP:
                    rotateBlock();
                    break;
                default:
                    moveBlock(0, 1);  // Automatic downward movement
                    break;
            }
            draw();
        }
    }
};

int main() {
    srand(time(0));
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    Tetris game;
    game.run();

    endwin();
    return 0;
}
