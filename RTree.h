//
// Created by Sebastian Knell on 14/05/22.
//

#ifndef RTREE_RTREE_H
#define RTREE_RTREE_H

#include <vector>
#include <stack>
#include <cstdlib>
#include <algorithm>
#include <utility>
#include <cmath>

using namespace std;

using Point = struct {int x; int y;};
// creo que combiene esta definicion
using Rect = struct {int x_low; int y_low; int x_high; int y_high;};
//using Rect = struct {int x; int y; int w; int h;};
using Data = vector<Point>;

extern int radius;
extern bool isInCircle(Point p, Point circleP, int radius);

struct Node {
    Rect rect;
    vector<Rect> regions; // regiones o bounding boxes si es hoja
    vector<Node*> childs;
    bool isLeaf;
    vector<Data*> data; // valido si es hoja
};

class RTree {
    Node* root;
    int order;
    pair<Node*, Node*> splitNode(Node*);
public:
    RTree(int order = 3);
    void insert(const Data&);
    void remove(Point);
};


#endif //RTREE_RTREE_H
