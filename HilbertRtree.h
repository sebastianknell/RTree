//
// Created by Sebastian Knell on 14/06/22.
//

#ifndef HILBERT_RTREE_HILBERTRTREE_H
#define HILBERT_RTREE_HILBERTRTREE_H

#include <iostream>
#include <vector>

using namespace std;

using Point = struct {int x, y;};
using Rect = struct {int xlow, ylow, xhigh, yhigh;};
using Data = vector<Point>;

struct Node {
    bool isLeaf;
    Node* prev;
    int lhv;
    vector<Rect> regions;
    vector<Node*> childs;
    vector<Data*> data;
};

class HilbertRtree {
    Node* root;
public:
    static int getHilbertIndex(Point, int x, int y, int xi, int xj, int yi, int yj, int n, int index);
    void insert(Data);
    void remove(Data);
};


#endif //HILBERT_RTREE_HILBERTRTREE_H
