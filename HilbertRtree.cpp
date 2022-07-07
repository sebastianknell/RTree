//
// Created by Sebastian Knell on 14/06/22.
//

#include "HilbertRtree.h"

static void addRegion(Node* node, Rect region) {
    if (node->regions.empty()) node->rect = region;
    else {
        node->rect.x_low = min(node->rect.x_low, region.x_low);
        node->rect.x_high = max(node->rect.x_high, region.x_high);
        node->rect.y_low = min(node->rect.y_low, region.y_low);
        node->rect.y_high = max(node->rect.y_high, region.y_high);
    }
    node->regions.push_back(region);
}

static int getHilbertIndexRec(Point p, int x, int y, int xi, int xj, int yi, int yj, int n, int index) {
    // Ver en que cuadrante esta
    // Cuadrante 1
    if (isInRect(p, {x, y, x+xi/2+yi/2, y+xj/2+yj/2})) {
        if (n <= 0) return index;
        return getHilbertIndexRec(p, x, y, yi/2, yj/2, xi/2, xj/2, n-1, index);
    }
    // Cuadrante 2
    else if (isInRect(p, {x+xi/2, y+xj/2, x+xi+yi/2, y+xj+yj/2})) {
        if (n <= 0) return index + 1;
        return getHilbertIndexRec(p, x+xi/2, y+xj/2, xi/2, xj/2, yi/2, yj/2, n-1, index + 4);
    }
    // Cuadrante 3
    else if (isInRect(p, {x+xi/2+yi/2, y+xj/2+yj/2, x+xi+yi, y+xj+yj})) {
        if (n <= 0) return index + 2;
        return getHilbertIndexRec(p, x+xi/2+yi/2, y+xj/2+yj/2, xi/2, xj/2, yi/2, yj/2, n-1, index + 8);
    }
    // Cuadrante 4
    else {
        if (n <= 0) return index + 3;
        return getHilbertIndexRec(p, x+xi/2+yi, y+xj/2+yj, -yi/2, -yj/2, -xi/2, -xj/2, n-1, index + 12);
    }
}

int HilbertRtree::getHilbertIndex(Point p) {
    return getHilbertIndexRec(p, 0, 0, gridWidth, 0, 0, gridHeight, levels, 0);
}

void HilbertRtree::insert(Data& data) {
    auto bb = getBoundingBox(data);
    int hIndex;
    if (data.size() == 1)
        hIndex = getHilbertIndex(data.front());
    else
        hIndex = getHilbertIndex(getCenter(bb));

    if (!root) {
        root = new Node(true);
        addRegion(root, bb);
        root->data.push_back({new Data(data), hIndex});
    }
    auto curr = root;
    while (!curr->isLeaf) {
        int i = 0;
        while (i < curr->childs.size() && curr->childs[i]->lhv <= hIndex) i++;
        curr = curr->childs[i];
    }
    addRegion(curr, bb);
    curr->data.push_back({new Data(data), hIndex});
    if (root->regions.size() > order) {

    }
}
