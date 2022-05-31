//
// Created by Sebastian Knell on 14/05/22.
//

#ifndef RTREE_RTREE_H
#define RTREE_RTREE_H

#include <vector>
#include <stack>
//#include <cstdlib>
#include <algorithm>
//#include <utility>
#include <cmath>
#include <opencv2/opencv.hpp>

using namespace std;

using Point = cv::Point;
using Rect = struct {int x_low; int y_low; int x_high; int y_high;};
using Data = vector<cv::Point>;

extern int radius;
extern bool isInCircle(Point p, Point circleP, int radius);

struct Node {
    // TODO ver si combiene usar List
    Rect rect;
    vector<Rect> regions; // regiones o bounding boxes si es hoja
    vector<Node*> childs;
    bool isLeaf;
    vector<Data*> data; // valido si es hoja
    explicit Node(bool isLeaf): isLeaf(isLeaf) {};
};

using pos = struct {Node* node; int index;};

class RTree {
    Node* root;
    int order;
    Node* splitNode(Node*) const;
public:
    RTree(int order = 3);
    void insert(const Data);
    void remove(Point);
    void show(cv::InputOutputArray&);
};


#endif //RTREE_RTREE_H
