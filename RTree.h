//
// Created by Sebastian Knell on 14/05/22.
//

#ifndef RTREE_RTREE_H
#define RTREE_RTREE_H

#include <vector>
using namespace std;

using Point = struct {int x; int y;};
using Region = struct {int x; int y; int w; int h;};
using Polygon = vector<Point>;

struct Node {
    bool isLeaf;
    vector<Region> regions;
    vector<Node*> childs;
};

class RTree {
    Node* root;
public:
    void insert(Point);
    void insert(Polygon);
    void remove(Point);
    void remove(Polygon);
};


#endif //RTREE_RTREE_H
