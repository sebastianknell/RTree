//
// Created by Sebastian Knell on 14/05/22.
//

#include "RTree.h"

static int getNewPerimeter(Rect region, Point point) {
    // TODO comprobar
    auto newWidth = min(abs(region.x - point.x), abs(region.x + region.w - point.x));
    auto newHeight = min(abs(region.y - point.y), abs(region.y + region.h - point.y));
    return newWidth*2 + newHeight*2;
}

static int getBestRegion(Node* node, Point point) {
    auto best = getNewPerimeter(node->regions.front(), point);
    auto bestIndex = 0;
    for (int i = 1; i < node->regions.size(); i++) {
        auto newPerimeter = getNewPerimeter(node->regions[i], point);
        if (newPerimeter < best) {
            best = newPerimeter;
            bestIndex = i;
        }
    }
    return bestIndex;
}

RTree::RTree(int order): order(order), root(nullptr) {}

void RTree::insert(Point point) {
    if (!root) {
        root = new Node();
        root->points.push_back(point);
        return;
    }
    // Buscar region
    auto curr = root;
    while (!curr->isLeaf) {
        auto regionIndex = getBestRegion(curr, point);
        curr = curr->childs[regionIndex];
    }
    // Insertar
    curr->points.push_back(point);
    // Si el nodo es invalido, hacer split
    if (curr->points.size() > order) {

    }
}
