//
// Created by Sebastian Knell on 14/05/22.
//

#ifndef RTREE_RTREE_H
#define RTREE_RTREE_H

#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <cmath>
#include <random>
#include "utils.h"

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

class RTree {
    Node* root;
    int order;
    Node* splitNode(Node*) const;
    void adjustTree(Node*, stack<pos2>&);
    void reinsert();
    void reinsert2(queue<Node*>&);
public:
    RTree(int order = 3);
    ~RTree();
    bool isEmpty() { return root == nullptr; };
    void insert(const Data&);
    void remove(const Data&);
    vector<knnResult> knn(Point, int);
    vector<knnResult> depthFirst(Point, int);
    void useCircles();
    void show(cv::InputOutputArray&);
};


#endif //RTREE_RTREE_H
