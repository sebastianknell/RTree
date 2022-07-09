//
// Created by Sebastian Knell on 14/06/22.
//

#ifndef HILBERT_RTREE_HILBERTRTREE_H
#define HILBERT_RTREE_HILBERTRTREE_H

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "lib/utils.h"

using namespace std;

using HData = struct {Data* data; int hIndex;};

struct Node {
    bool isLeaf;
    Node* prev; // se necesita para pedir del hermano?
    int lhv;
    Rect rect;
    vector<Rect> regions;
    vector<Node*> childs;
    vector<HData> data;
    explicit Node(bool isLeaf): isLeaf(isLeaf), lhv(0) {}
};

class HilbertRtree {
    int gridWidth, gridHeight, levels, order;
    Node* root;
    int getHilbertIndex(Point);
public:
    HilbertRtree(int order = 3): order(order), root(nullptr) {}
    void insert(Data&);
    void remove(Data&);
};


#endif //HILBERT_RTREE_HILBERTRTREE_H
