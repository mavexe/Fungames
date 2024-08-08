#ifndef MAIN_H
#define MAIN_H

class Point {
    public:
        Point(double x, double y);
        ~Point();
    int getX(int x){
        return x;
    }
        void print() const;
    private:
        double x;
        double y;        
};

#endif
