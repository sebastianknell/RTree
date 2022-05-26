//
// Created by Sebastian Knell on 14/05/22.
//

#ifndef RTREE_RTREE_H
#define RTREE_RTREE_H

#include <vector>
#include <cstdlib>
#include <algorithm>
#include <utility>
#include <cmath>

using namespace std;

using Point = struct {int x; int y;};
// creo que combiene esta definicion
using Rect = struct {int x_low; int y_low; int x_high; int y_high;};
//using Rect = struct {int x; int y; int w; int h;};
using Polygon = vector<Point>;

extern int radius;
extern bool isInCircle(Point p, Point circleP, int radius);

struct Node {
    bool isLeaf;
    Rect rect;
    vector<Rect> regions;
    vector<Point> points;
    vector<Node*> childs;
};

class RTree {
    Node* root;
    int order;
    void splitNode(Node*);
public:
    RTree(int order = 3);
    void insert(Point);
    void insert(Polygon);
    void remove(Point);
    void remove(Polygon);
};


#endif //RTREE_RTREE_H
