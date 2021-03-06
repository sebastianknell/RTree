//
// Created by Sebastian Knell on 14/05/22.
//

#ifndef RTREE_RTREE_H
#define RTREE_RTREE_H

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <cmath>
#include <random>
#include <optional>
#include <opencv2/opencv.hpp>
#include "lib/utils.h"

using namespace std;

using Circle = struct {Point center; int radius;};

struct Node{
    Rect rect;
    Circle circle;
    int minRadius;
    int level;
    vector<Rect> regions; // regiones o bounding boxes si es hoja
    vector<Node*> childs;
    bool isLeaf;
    vector<Data*> data; // valido si es hoja
    explicit Node(bool, int);
    ~Node();
};

using pos = struct {Node* node; int index;};
using pos2 = struct {Node* node; Rect* region;};
using lineTo = struct {Point p; double distance;};
using knnResult = struct {Node* node; int index; Point p;};

class RTree : public Tree {
    Node* root;
    int order;
    Node* splitNode(Node*) const;
    void adjustTree(Node*, stack<pos2>&);
    void reinsert(queue<Node*>&);
public:
    RTree(int order = 3);
    ~RTree();
    bool isEmpty() { return root == nullptr; };
    void search(const Data &) override;
    void insert(const Data&) override;
    void remove(const Data&) override;
    vector<knnResult> knn(Point, int);
    vector<knnResult> depthFirst(Point, int);
    void callKnn(Point, int) override;
    void useCircles();
    void show(cv::InputOutputArray &);
    void clear() override;
    vector<double> getLeafsOverlap() override;
    vector<double> getInternalOverlap() override;
};


#endif //RTREE_RTREE_H
