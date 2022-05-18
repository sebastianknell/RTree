//
// Created by Sebastian Knell on 14/05/22.
//

#include "RTree.h"

// TODO generalizar para rectangulos
static int getPerimeterEnlargement(Rect region, Rect r) {

}

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

pair<int,int> pickSeeds(const vector<Rect> &regions) {
    // Escoger primeras 2 regiones
    int x_min = regions.front().x, x_max = regions.front().x + regions.front().w;
    int y_min = regions.front().y - regions.front().h, y_max = regions.front().y;
    int x_low = 0, x_high = 0, y_low = 0, y_high = 0; // Representan regiones
    for (int i = 1; i < regions.size(); i++) {
        // En x
        // Mayor lado inferior
        if (regions[i].x - regions[i].w > regions[x_low].x - regions[x_low].w) x_low = i;
        // Menor lado superior
        if (regions[i].x < regions[x_high].x) x_high = i;
        // En y
        // Mayor lado inferior
        if (regions[i].y - regions[i].h > regions[y_low].y - regions[y_low].h) y_low = i;
        // Menor lado superior
        if (regions[i].y < regions[y_high].y) y_high = i;

        // Actualizar minimos y maximos entre todas las regiones
        if (regions[i].x < x_min) x_min = regions[i].x;
        if (regions[i].x + regions[i].w > x_max) x_max = regions[i].x + regions[i].w;
        if (regions[i].y - regions[i].h < y_min) y_min = regions[i].y - regions[i].h;
        if (regions[i].y > y_max) y_max = regions[i].y;

    }
    auto x_dif = regions[x_high].x - regions[x_low].x + regions[x_low].w;
    auto y_dif = regions[y_high].y - regions[y_low].y + regions[y_low].h;
    // Normalizar las distancias
    x_dif /= (x_max - x_min);
    y_dif /= (y_max - y_min);
    // Escoger la mayor
    if (x_dif >= y_dif) {
        return {x_low, x_high};
    } else {
        return {y_low, y_high};
    }
}

void splitNode(Node* node) {
    auto seeds = pickSeeds(node->regions);
    vector<Rect> group1, group2;
    group1.push_back(node->regions[seeds.first]);
    group2.push_back(node->regions[seeds.second]);

}

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
