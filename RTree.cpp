//
// Created by Sebastian Knell on 14/05/22.
//

#include "RTree.h"

int radius = 4;

bool isInCircle(Point p, Point circleP, int r) {
    return (pow(p.x - circleP.x, 2) + pow(p.y - circleP.y, 2)) <= pow(r, 2);
}

bool isInRect(const Point &p, const Rect &rect) {
    return p.x >= rect.x_low - radius && p.x <= rect.x_high + radius && p.y >= rect.y_low - radius && p.y <= rect.y_high + radius;
}

static bool operator==(const Rect &a, const Rect &b) {
    return a.x_low == b.x_low && a.x_high == b.x_high && a.y_low == b.y_low && a.y_high == b.y_high;
}

static int getPerimeterEnlargement(Rect region, Rect r) {
    auto x_low = min(region.x_low, r.x_low);
    auto y_low = min(region.y_low, r.y_low);
    auto x_high = max(region.x_high, r.x_high);
    auto y_high = max(region.y_high, r.y_high);
    return ((x_high - x_low) + (y_high - y_low)) - ((region.x_high - region.x_low) + (region.y_high - region.y_low));
}

static int getBestRegion(Node* node, Rect r) {
    auto best = getPerimeterEnlargement(node->regions.front(), r);
    auto bestIndex = 0;
    for (int i = 1; i < node->regions.size(); i++) {
        auto newPerimeter = getPerimeterEnlargement(node->regions[i], r);
        if (newPerimeter < best) {
            best = newPerimeter;
            bestIndex = i;
        }
    }
    return bestIndex;
}

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

static Rect getBoundingBox(const Data &data) {
    if (data.size() == 1) { // Es un punto
        return {data.front().x, data.front().y, data.front().x, data.front().y};
    }
    else {
        Rect bBox;
        bBox.x_high = bBox.x_low = data.front().x;
        bBox.y_low = bBox.y_high = data.front().y;
        for(const auto& vertex : data){
            if(vertex.x < bBox.x_low) bBox.x_low = vertex.x;
            else if(vertex.x > bBox.x_high) bBox.x_high = vertex.x;
            if(vertex.y < bBox.y_low) bBox.y_low = vertex.y;
            else if(vertex.y > bBox.y_high) bBox.y_high = vertex.y;
        }
        return bBox;
    }
}

static Rect getBoundingRect(const vector<Rect> &regions) {
    int x_min = regions.front().x_low, x_max = regions.front().x_high;
    int y_min = regions.front().y_low, y_max = regions.front().y_high;
    for (int i = 1; i < regions.size(); i++) {
        if (regions[i].x_low < x_min) x_min = regions[i].x_low;
        if (regions[i].x_high > x_max) x_max = regions[i].x_high;
        if (regions[i].y_low < y_min) y_min = regions[i].y_low;
        if (regions[i].y_high > y_max) y_max = regions[i].y_high;
    }
    return {x_min, y_min, x_max, y_max};
}


RTree::RTree(int order): order(order), root(nullptr) {}

Node::Node(bool isLeaf): isLeaf(isLeaf) {}

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

Node* RTree::splitNode(Node* node) const {
    // TODO refactor y optimizar
    auto seeds = pickSeeds(node->regions);
    assert(seeds.first != seeds.second);
    auto group1 = new Node(node->isLeaf), group2 = new Node(node->isLeaf);
    addRegion(group1, node->regions[seeds.first]);
    addRegion(group2, node->regions[seeds.second]);

    if (node->isLeaf) {
        group1->data.push_back(node->data[seeds.first]);
        group2->data.push_back(node->data[seeds.second]);
    }
    else {
        group1->childs.push_back(node->childs[seeds.first]);
        group2->childs.push_back(node->childs[seeds.second]);
    }

    node->regions.erase(node->regions.begin() + seeds.first);
    if (seeds.first < seeds.second) seeds.second -= 1; // los indices pueden cambiar luego de borrar
    node->regions.erase(node->regions.begin() + seeds.second);

    if (node->isLeaf) {
        node->data.erase(node->data.begin() + seeds.first);
        node->data.erase(node->data.begin() + seeds.second);
    }
    else {
        node->childs.erase(node->childs.begin() + seeds.first);
        node->childs.erase(node->childs.begin() + seeds.second);
    }

    auto min = (int)order/2 + order%2;
    do {
        if (group1->regions.size() + node->regions.size() == min) {
            // Agregar todas las regiones restantes al grupo 1
            for (int i = 0; i < node->regions.size(); i++) {
                addRegion(group1, node->regions[i]);
                if (node->isLeaf) group1->data.push_back(node->data[i]);
                else group1->childs.push_back(node->childs[i]);
            }
            break;
        }
        else if (group2->regions.size() + node->regions.size() == min) {
            // Agregar todas las regiones restantes al grupo 2
            for (int i = 0; i < node->regions.size(); i++) {
                addRegion(group2, node->regions[i]);
                if (node->isLeaf) group2->data.push_back(node->data[i]);
                else group2->childs.push_back(node->childs[i]);
            }
            break;
        }
        auto next = pickNext(node->regions, group1->rect, group2->rect);
        auto group1Enlargement = getPerimeterEnlargement(group1->rect, node->regions[next]);
        auto group2Enlargement = getPerimeterEnlargement(group2->rect, node->regions[next]);
        if (group1Enlargement < group2Enlargement) {
            // Mover la region al grupo 1
            addRegion(group1, node->regions[next]);
            if (node->isLeaf) group1->data.push_back(node->data[next]);
            else group1->childs.push_back(node->childs[next]);
        }
        else if (group1Enlargement > group2Enlargement) {
            // Mover la region al grupo 2
            addRegion(group2, node->regions[next]);
            if (node->isLeaf) group2->data.push_back(node->data[next]);
            else group2->childs.push_back(node->childs[next]);
        }
        else {
            // Mover al que tiene menos regiones
            if (group1->regions.size() < group2->regions.size()) {
                addRegion(group1, node->regions[next]);
                if (node->isLeaf) group1->data.push_back(node->data[next]);
                else group1->childs.push_back(node->childs[next]);
            }
            else {
                addRegion(group2, node->regions[next]);
                if (node->isLeaf) group2->data.push_back(node->data[next]);
                else group2->childs.push_back(node->childs[next]);
            }
        }
        node->regions.erase(node->regions.begin() + next);
        if (node->isLeaf) node->data.erase(node->data.begin() + next);
        else node->childs.erase(node->childs.begin() + next);
    } while (!node->regions.empty());
    node->regions = group1->regions;
    node->rect = group1->rect;
    if (node->isLeaf) node->data = group1->data;
    else node->childs = group1->childs;
    return group2;
}

void RTree::insert(const Data data) {
    // Calcular bounding box
    Rect bb = getBoundingBox(data);
    if (!root) {
        root = new Node(true);
        addRegion(root, bb);
        root->data.push_back(new Data(data));
        return;
    }
    // Buscar region
    auto curr = root;
    using pos = struct {Node* node; Rect* region;};
    stack<pos> parents;
    while (!curr->isLeaf) {
        auto regionIndex = getBestRegion(curr, bb);
        parents.push({curr, &curr->regions[regionIndex]});
        curr = curr->childs[regionIndex];
    }
    // Insertar
    addRegion(curr, bb);
    curr->data.push_back(new Data(data));
    Node *curr2 = nullptr;
    bool rootSplit = false;
    // Si el nodo es invalido, hacer split
    if (curr->regions.size() > order) {
        curr2 = splitNode(curr);
        if (curr == root) rootSplit = true;
    }
    // Ajustar el arbol
    do {
        if (curr == root) break;
        auto parent = parents.top().node;
        auto regionInParent = parents.top().region;
        parents.pop();
        // Ajustar la region en el padre
        *regionInParent = curr->rect;
        parent->rect = getBoundingRect(parent->regions);
        // Propagar split hacia arriba
        if (curr2) {
            addRegion(parent, curr2->rect);
            parent->childs.push_back(curr2);
            if (parent->regions.size() > order) {
                curr2 = splitNode(parent);
                if (parent == root) rootSplit = true;
            }
        }
        curr = parent;
    } while (!parents.empty());
    // Cambiar al root si se hizo un split
    if (rootSplit) {
        assert(curr2 != nullptr);
        root = new Node(false);
        addRegion(root, curr->rect);
        addRegion(root, curr2->rect);
        root->childs.push_back(curr);
        root->childs.push_back(curr2);
    }
}

// Esta rect dentro de region?
static bool isOverlapping(const Rect &rect, const Rect &region) {
    return isInRect({rect.x_low, rect.y_low}, region) &&
            isInRect({rect.x_low, rect.y_high}, region) &&
            isInRect({rect.x_high, rect.y_low}, region) &&
            isInRect({rect.x_high, rect.y_high}, region);
}

void dfs(vector<Data*> &toReinsert, Node* rt){

    if(rt->isLeaf){
        auto data = rt->data;
        toReinsert.insert(toReinsert.begin(), data.begin(), data.end());
        return;
    }

    for(auto node : rt->childs){
        dfs(toReinsert, node);
    }
}

void RTree::reinsert(){

    vector<Data*> toReinsert;

    dfs(toReinsert, root);
    this->root = new Node(true);

    for(auto dat : toReinsert){
        insert(*dat);
    }
}

void RTree::remove(const Data& data) {
    auto bb = getBoundingBox(data);
    using pos = struct {Node* node; int index;};
    stack<pos> parents;
    auto curr = root;
    bool found = false;
    // Encontrar hoja en recorrido dfs y quedarme con el stack de padres
    if (!curr->isLeaf) {
        for (int i = 0; i < curr->regions.size(); i++) {
            if (isOverlapping(bb, curr->regions[i])) {
                parents.push({curr, i});
            }
        }
    }
    else {
        for (int i = 0; i < curr->regions.size(); i++) {
            if (curr->data[i]->size() == 1 && data.size() == 1) {
                if (isInCircle(data.front(), curr->data[i]->front(), radius)) {
                    found = true;
                }
            }
            else if (*curr->data[i] == data) {
                found = true;
            }
            if (found) {
                parents.push({curr, i});
                break;
            }
        }
    }
    while (!parents.empty() && !found) {
        curr = parents.top().node->childs[parents.top().index];
        if (!curr->isLeaf) {
            bool isCandidate = false;
            for (int i = 0; i < curr->regions.size(); i++) {
                if (isOverlapping(bb, curr->regions[i])) {
                    parents.push({curr, i});
                    isCandidate = true;
                }
            }
            if (!isCandidate) parents.pop();
        }
        else {
            for (int i = 0; i < curr->regions.size(); i++) {
                if (curr->data[i]->size() == 1 && data.size() == 1) {
                    if (isInCircle(data.front(), curr->data[i]->front(), radius)) {
                        found = true;
                    }
                }
                else if (*curr->data[i] == data) {
                    found = true;
                }
                if (found) {
                    parents.push({curr, i});
                    break;
                }
            }
            if (!found) parents.pop();
        }
    }
    // Si no se encontro, terminar
    if (!found) return;

    // Eliminar data
    assert(curr->isLeaf);
    auto parent = parents.top();
    parents.pop();
    auto currIndex = parent.index;
    curr->regions.erase(curr->regions.begin() + currIndex);
    curr->data.erase(curr->data.begin() + currIndex);
    curr->rect = getBoundingRect(curr->regions);

    // Adjust Parent Bounding Boxes
    parent.node->rect = getBoundingRect(parent.node->regions);
    while(!parents.empty()){
        parent = parents.top();
        parents.pop();
        parent.node->rect = getBoundingRect(parent.node->regions);
    }

    // Si no hay huerfanos, return.
    auto minEntries = (int)order/2 + order%2;
    if(curr->data.size() >= minEntries || curr == root) return;

    // Reinsertar data huerfana
    reinsert();
}

int colorIdx = 0;

static void showNode(Node* node, cv::InputOutputArray &img) {
    // TODO pintar el arbol de arriba hacia abajo para que las hojas no pinten por encima a las regiones superiores
    if (node == nullptr) return;
    colorIdx++;
    if (node->isLeaf) {
        for (int i = 0; i < node->regions.size(); i++) {
            if (node->data[i]->size() == 1) {
                cv::circle(img, node->data[i]->back(), radius, colors[3], -1);
            }
            else {
                cv::rectangle(img, {node->regions[i].x_low, node->regions[i].y_low}, {node->regions[i].x_high-1, node->regions[i].y_high-1}, colors[3], 2);
                cv::polylines(img, *node->data[i], true, colors[3], 2);
            }
        }
    }
    else {
        for (const auto &r : node->regions) {
            cv::rectangle(img, {r.x_low, r.y_low}, {r.x_high-1, r.y_high-1}, colors[colorIdx%6], 2);
        }
        for (const auto &c : node->childs) {
            showNode(c, img);
        }
    }
}

void RTree::show(cv::InputOutputArray &img) {
    colorIdx = 0;
    showNode(root, img);
}
