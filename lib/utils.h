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
using Rect = struct {int x_low; int y_low; int x_high; int y_high;};
using Data = vector<Point>;

class Tree {
public:
    virtual void search(const Data&) = 0;
    virtual void insert(const Data&) = 0;
    virtual void remove(const Data&) = 0;
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

#endif //RTREE_UTILS_H