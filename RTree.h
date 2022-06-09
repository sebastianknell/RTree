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
#include <opencv2/opencv.hpp>

using namespace std;

using Point = cv::Point;
using Rect = struct {int x_low; int y_low; int x_high; int y_high;};
using Data = vector<cv::Point>;

extern int radius;
extern bool isInCircle(Point p, Point circleP, int radius);
extern Rect getSearchRectangle(const Point&, int);

static cv::Scalar colors[] = {
        {106, 100, 208},
        {233, 145, 198},
        {255, 221, 137},
        {255, 170, 130},
        {141, 232, 195},
        {107, 203, 255}
};

struct Node {
    // TODO ver si combiene usar List
    Rect rect;
    vector<Rect> regions; // regiones o bounding boxes si es hoja
    vector<Node*> childs;
    bool isLeaf;
    vector<Data*> data; // valido si es hoja
    explicit Node(bool);
};

class RTree {
    Node* root;
    int order;
    Node* splitNode(Node*) const;
    void reinsert();
public:
    RTree(int order = 3);
    void insert(Data);
    void remove(const Data&);
    void show(cv::InputOutputArray&);
    Point search(Rect);
};


#endif //RTREE_RTREE_H
