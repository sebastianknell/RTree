//
// Created by Sebastian Knell on 14/06/22.
//

#ifndef HILBERT_RTREE_HILBERTRTREE_H
#define HILBERT_RTREE_HILBERTRTREE_H

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "utils.h"

using namespace std;

struct HData {
    Data data;
    int hIndex;
    HData() {}
    HData(Data data, int hIndex): data(data), hIndex(hIndex) {}
    ~HData() {}
};

struct HilbertNode {
    bool isLeaf;
    int lhv, lvl;
    HilbertNode* parent;
    Rect rect;

    vector<Rect> regions;
    vector<HilbertNode*> children;
    vector<HData> data;

    explicit HilbertNode(bool isLeaf): isLeaf(isLeaf), lhv(0), parent(nullptr), lvl(0) {}
    HilbertNode() {}
    void insertOrdered(HData hdata, Rect region);
    void updateBoundingBox();
    void updateLHV();
};

struct Entry {
    bool type;  // 0 = data, 1 = child
    HData data;
    Rect rect;  // rectangulo asociado a la data
    HilbertNode* child;

    Entry() {}
    ~Entry() {}
};

class HilbertRtree {
    int gridWidth, gridHeight, levels, order;
    HilbertNode* root;
    int getHilbertIndex(Point);
public:
    HilbertRtree(int gridW, int gridH, int order = 3) : order(order), gridWidth(gridW), gridHeight(gridH) {
        this->levels = 9;
        this->root = new HilbertNode(true);
    }
    void insert(const Data);
    void remove(const Data);
    void adjustTree(HilbertNode*);
    HilbertNode* chooseLeaf(HilbertNode*, int);
    void handleOverflow(HilbertNode*);
    void showHilbert(cv::InputOutputArray&);
};


#endif //HILBERT_RTREE_HILBERTRTREE_H
