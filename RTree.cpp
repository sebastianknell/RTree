//
// Created by Sebastian Knell on 14/05/22.
//

#include "RTree.h"

int radius = 4;

bool isInCircle(Point p, Point circleP, int radius) {
    return (pow(p.x - circleP.x, 2) + pow(p.y - circleP.y, 2)) <= pow(radius, 2);
}

static int getPerimeterEnlargement(Rect region, Rect r) {
    auto x_low = min(region.x_low, r.x_low);
    auto y_low = min(region.y_low, r.y_low);
    auto x_high = max(region.x_high, r.x_high);
    auto y_high = max(region.y_high, r.y_high);
    return ((x_high - x_low) + (y_high - y_low)) - ((region.x_high - region.x_low) + (region.y_high - region.y_low));
}

static int getPerimeterEnlargement(Rect region, Point point) {
    int widthEnlargement = 0, heightEnlargement = 0;
    if(point.x < region.x_low || point.x > region.x_high){
        widthEnlargement = min(abs(region.x_low - point.x), abs(region.x_high - point.x));
    }
    if(point.y < region.y_low || point.y > region.y_high) {
        heightEnlargement = min(abs(region.y_low - point.y), abs(region.y_high - point.y));
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

static void addRegion(Node* node, Rect region) {
    node->rect.x_low = min(node->rect.x_low, region.x_low);
    node->rect.x_high = min(node->rect.x_high, region.x_high);
    node->rect.y_low = min(node->rect.y_low, region.y_low);
    node->rect.y_high = min(node->rect.y_high, region.y_high);
    node->regions.push_back(region);
}

RTree::RTree(int order): order(order), root(nullptr) {}

static pair<int,int> pickSeeds(const vector<Rect> &regions) {
    // Escoger primeras 2 regiones
    int x_min = regions.front().x_low, x_max = regions.front().x_high;
    int y_min = regions.front().y_low, y_max = regions.front().y_high;
    int lowRegion_x = 0, highRegion_x = 0, lowRegion_y = 0, highRegion_y = 0;
    for (int i = 1; i < regions.size(); i++) {
        // En x
        // Mayor lado inferior
        if (regions[i].x_low > regions[lowRegion_x].x_low) lowRegion_x = i;
        // Menor lado superior
        if (regions[i].x_high < regions[highRegion_x].x_high) highRegion_x = i;
        // En y
        // Mayor lado inferior
        if (regions[i].y_low > regions[lowRegion_y].y_low) lowRegion_y = i;
        // Menor lado superior
        if (regions[i].y_high < regions[highRegion_y].y_high) highRegion_y = i;

        // Actualizar minimos y maximos entre todas las regiones
        if (regions[i].x_low < x_min) x_min = regions[i].x_low;
        if (regions[i].x_high > x_max) x_max = regions[i].x_high;
        if (regions[i].y_low < y_min) y_min = regions[i].y_low;
        if (regions[i].y_high > y_max) y_max = regions[i].y_high;
    }
    auto x_dif = regions[highRegion_x].x_low - regions[lowRegion_x].x_high;
    auto y_dif = regions[highRegion_y].y_low - regions[lowRegion_y].y_high;
    // Normalizar las distancias
    x_dif /= (x_max - x_min);
    y_dif /= (y_max - y_min);
    // Escoger la mayor
    if (x_dif >= y_dif) {
        return {lowRegion_x, highRegion_x};
    } else {
        return {lowRegion_y, highRegion_y};
    }
}

static int pickNext(const vector<Rect> &regions, Rect group1, Rect group2) {
    int i = 0;
    int maxDif = -1;
    do {
        auto dif = abs(getPerimeterEnlargement(group1, regions[i]) - getPerimeterEnlargement(group2, regions[i]));
        if (dif > maxDif) maxDif = i;
        i++;
    } while (i < regions.size());
    return maxDif;
}

void RTree::splitNode(Node* node) {
    auto seeds = pickSeeds(node->regions);
    auto group1 = new Node(), group2 = new Node();
    addRegion(group1, node->regions[seeds.first]);
    addRegion(group2, node->regions[seeds.second]);
    node->regions.erase(node->regions.begin() + seeds.first);
    node->regions.erase(node->regions.begin() + seeds.second);

    auto min = (int)order/2;
    do {
        auto next = pickNext(node->regions, group1->rect, group2->rect);
        if (group1->regions.size() + node->regions.size() == min) {
            // Agregar todas las regiones restantes al grupo 1
            for (auto &region : node->regions) {
                addRegion(group1, region);
            }
            break;
        }
        else if (group2->regions.size() + node->regions.size() == min) {
            // Agregar todas las regiones restantes al grupo 2
            for (auto &region : node->regions) {
                addRegion(group2, region);
            }
            break;
        }
        auto group1Enlargement = getPerimeterEnlargement(group1->rect, node->regions.back());
        auto group2Enlargement = getPerimeterEnlargement(group2->rect, node->regions.back());
        if (group1Enlargement < group2Enlargement) {
            // Mover la region al grupo 1
            addRegion(group1, node->regions[next]);
        }
        else if (group1Enlargement > group2Enlargement) {
            // Mover la region al grupo 2
            addRegion(group2, node->regions[next]);
        }
        else {
            // Mover al que tiene menos regiones
            if (group1->regions.size() < group2->regions.size()) {
                addRegion(group1, node->regions[next]);
            }
            else {
                addRegion(group2, node->regions[next]);
            }
        }
        node->regions.erase(node->regions.begin() + next);
    } while (!node->regions.empty());
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
