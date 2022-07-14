//
// Created by Sebastian Knell on 6/07/22.
//

#ifndef RTREE_UTILS_H
#define RTREE_UTILS_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>

using namespace std;

using Point = cv::Point;
struct Rect {
    int x_low; int y_low; int x_high; int y_high;
    void operator=(Rect r) {
        x_low = r.x_low;
        y_low = r.y_low;
        x_high = r.x_high;
        y_high = r.y_high;
    }
};
// using Rect = struct {int x_low; int y_low; int x_high; int y_high;};
using Data = vector<Point>;

class Tree {
public:
    virtual void search(const Data&) = 0;
    virtual void insert(const Data&) = 0;
    virtual void remove(const Data&) = 0;
    virtual void clear() = 0;
    virtual void callKnn(Point, int) = 0;
    virtual double getLeafsOverlap() = 0;
    virtual double getInternalOverlap() = 0;
};

extern int radius;
extern cv::Scalar colors[];
bool isInCircle(Point p, Point circleP, int r);
bool isInRect(const Point &p, Rect rect);
bool operator==(const Rect &a, const Rect &b);
int getPerimeterEnlargement(Rect region, Rect r);
double getDistance(Point p, Rect rect);
double getDistance(Point a, Point b);
Rect getBoundingBox(const Data &data);
Rect getBoundingRect(const vector<Rect> &regions);
Point getCenter(const Rect&);
int getArea(const Rect&);
bool rectsOverlap(Rect r1, Rect r2);
int getOverlap(Rect, Rect);
double getTotalOverlap(vector<Rect>&);

#endif //RTREE_UTILS_H
