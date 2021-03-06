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
    vector<HilbertNode*> children;  // si es interno
    vector<HData> data;             // si es hoja

    explicit HilbertNode(bool isLeaf): isLeaf(isLeaf), lhv(0), parent(nullptr), lvl(0) {}
    HilbertNode() {}
    ~HilbertNode() {
        if (!isLeaf) {
            for (int i = 0; i < children.size(); i++) {
                delete children[i];
            }
        }
    }

    void insertOrdered(HData hdata, Rect region);
    void updateBoundingBox();
    void updateLHV();
};

using lineToH = struct {Point p; double distance;};
struct knnResultH {HilbertNode* node; int index; Point p;};

struct Entry {
    bool type;  // 0 = data, 1 = child
    HData data;
    Rect rect;  // rectangulo asociado a la data

    HilbertNode* child;

    Entry() {}
    ~Entry() {}
};

class HilbertRtree : public Tree {
    int gridWidth, gridHeight, levels, M, m;
    HilbertNode* root;
    int getHilbertIndex(Point);
public:
    HilbertRtree(int gridW, int gridH, int M = 3, int m = 2) : gridWidth(gridW), gridHeight(gridH), M(M), m(m) {
        assert(m <= M/2 + 1);
        this->levels = 3;
        this->root = new HilbertNode(true);
    }
    void insert(const Data&) override;
    void remove(const Data&) override;
    void search(const Data&) override;
    void clear() override;
    pair<int, HilbertNode*> searchUtil(const Data);
    vector<knnResultH> knn(Point, int);
    void callKnn(Point, int) override;
    void adjustTree(HilbertNode*);
    HilbertNode* chooseLeaf(HilbertNode*, int);
    void handleOverflow(HilbertNode*);
    void handleUnderflow(HilbertNode*);
    void showHilbert(cv::InputOutputArray&);
    bool isEmpty() {
        if (root->isLeaf)
            return root->data.size() == 0;
        return root->children.size() == 0;
    }
    vector<double> getLeafsOverlap() override;
    vector<double> getInternalOverlap() override;
    void setLevels(int l) {levels = l;};
};


#endif //HILBERT_RTREE_HILBERTRTREE_H
