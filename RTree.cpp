//
// Created by Sebastian Knell on 14/05/22.
//

#include "RTree.h"

static int getPerimeterEnlargement(Rect region, Point point) {
    int widthEnlargement = 0, heightEnlargement = 0;
    if(point.x < region.x || point.x > (region.x + region.w)){
        widthEnlargement = min(abs(region.x - point.x), abs(region.x + region.w - point.x));
    }
    if(point.y < region.y || point.y > (region.y + region.h)) {
        heightEnlargement = min(abs(region.y - point.y), abs(region.y + region.h - point.y));
    }
    return widthEnlargement*2 + heightEnlargement*2;
}

static int getBestRegion(Node* node, Point point) {
    auto best = getPerimeterEnlargement(node->regions.front(), point);
    auto bestIndex = 0;
    for (int i = 1; i < node->regions.size(); i++) {
        auto newPerimeter = getPerimeterEnlargement(node->regions[i], point);
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
