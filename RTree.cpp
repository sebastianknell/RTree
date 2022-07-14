//
// Created by Sebastian Knell on 14/05/22.
//

#include "RTree.h"

static Point operator-(const Point &a, const Point &b) {
    return {a.x - b.x, a.y - b.y};
}

static lineTo getDistanceToSegment(Point p, Point a, Point b) {
    auto dot = [](Point a, Point b) -> double {
        return a.x * b.x + a.y * b.y;
    };
    auto norm = [](Point p) -> double {
        return sqrt(pow(p.x, 2) + pow(p.y, 2));
    };
    auto ab = b - a;
    auto bp = p - b;
    auto ap = p - a;

    if (dot(ab, ap) < 0) {
        return {a, norm(ap)};
    }
    if (dot(ab, bp) > 0) {
        return {b, norm(bp)};
    }
    else {
        auto d = abs(ab.x * ap.y - ab.y * ap.x) / norm(ab);
        auto t = dot(ap, ab) / dot(ab, ab);
        Point point = {int(a.x + ab.x*t), int(a.y + ab.y*t)};
        return {point, d};
    }
}

static lineTo getDistance(Point p, Data *data) {
    if (data->size() == 1) return {data->front(), getDistance(p, data->front())};
    data->push_back(data->front());
    lineTo min = {{}, INT_MAX};
    for (int i = 0; i < data->size() - 1; i++) {
        auto distanceToSide = getDistanceToSegment(p, data->at(i), data->at(i+1));
        if (distanceToSide.distance < min.distance) min = distanceToSide;
    }
    return min;
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

RTree::RTree(int order): order(order), root(nullptr) {}

RTree::~RTree() {
    delete root;
}

Node::Node(bool isLeaf, int level): isLeaf(isLeaf), level(level), rect({0, 0, 0, 0}) {}

Node::~Node() {
    for (auto &c : childs) {
        delete c;
    }
}

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
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<std::mt19937::result_type> dist(0,regions.size()-1);
    if (x_dif >= y_dif) {
        while (lowRegion_x == highRegion_x) {
            highRegion_x = dist(rng);
        }
        return {lowRegion_x, highRegion_x};

    } else {
        while (lowRegion_y == highRegion_y) {
            highRegion_y = dist(rng);
        }
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
    auto group1 = new Node(node->isLeaf, node->level), group2 = new Node(node->isLeaf, node->level);
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

void RTree::adjustTree(Node* curr, stack<pos2> &parents) {
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
            else curr2 = nullptr;
        }
        curr = parent;
    } while (!parents.empty());
    // Cambiar al root si se hizo un split
    if (rootSplit) {
        assert(curr2 != nullptr);
        root = new Node(false, curr->level + 1);
        addRegion(root, curr->rect);
        addRegion(root, curr2->rect);
        root->childs.push_back(curr);
        root->childs.push_back(curr2);
    }
}

void RTree::insert(const Data &data) {
    // Calcular bounding box
    Rect bb = getBoundingBox(data);
    if (!root) {
        root = new Node(true, 0);
        addRegion(root, bb);
        root->data.push_back(new Data(data));
        return;
    }
    // Buscar region
    auto curr = root;
    stack<pos2> parents;
    while (!curr->isLeaf) {
        auto regionIndex = getBestRegion(curr, bb);
        parents.push({curr, &curr->regions[regionIndex]});
        curr = curr->childs[regionIndex];
    }
    // Insertar
    addRegion(curr, bb);
    curr->data.push_back(new Data(data));
    adjustTree(curr, parents);
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
    delete root;
    root = nullptr;
//    this->root = new Node(true);

    for(auto dat : toReinsert){
        insert(*dat);
    }
}

static bool checkSubtree(Node* node) {
    if (!(node->rect == getBoundingRect(node->regions))) return false;
    if (node->isLeaf) {
        if (node->level != 0) return false;
        if (!node->childs.empty()) return false;
        if (node->regions.size() < 2 || node->data.size() < 2) return false;
        if (node->regions.size() > 3 || node->data.size() > 3) return false;
        return true;
    }
    if (!node->data.empty()) return false;
    if (node->regions.size() < 2 || node->childs.size() < 2) return false;
    if (node->regions.size() > 3 || node->childs.size() > 3) return false;
    for (auto &c : node->childs) {
        if (c->level != node->level - 1) return false;
    }
    bool isValid = true;
    for (auto &c : node->childs) {
        isValid = isValid && checkSubtree(c);
    }
    return isValid;
}

void RTree::reinsert2(queue<Node*> &nodes) {
    while(!nodes.empty()) {
        auto nodeToInsert = nodes.front();
        nodes.pop();
        if (nodeToInsert->isLeaf) {
            for (auto d : nodeToInsert->data) {
                insert(*d);
            }
        }
        else {
            auto curr = root;
            stack<pos2> parents;
            while (curr->level > nodeToInsert->level) {
                auto regionIndex = getBestRegion(curr, nodeToInsert->rect);
                parents.push({curr, &curr->regions[regionIndex]});
                curr = curr->childs[regionIndex];
            }
            // ya estamos en el nivel correcto
            for (auto region : nodeToInsert->regions) addRegion(curr, region);
            for (auto child : nodeToInsert->childs) curr->childs.push_back(child);
            adjustTree(curr, parents);
        }
    }
}

pair<bool, list<pos>*> findLeaf(Node* curr, const Data &data, const Rect &bb) {
    if (!curr->isLeaf) {
        for (int i = 0; i < curr->regions.size(); i++) {
            if (isOverlapping(bb, curr->regions[i])) {
                auto res = findLeaf(curr->childs[i], data, bb);
                if (res.first) {
                    res.second->push_front({curr, i});
                    return {true, res.second};
                }
            }
        }
        return {false, nullptr};
    }
    else {
        bool found = false;
        for (int i = 0; i < curr->regions.size(); i++) {
            if (data.size() == 1) {
                if (curr->data[i]->size() == 1) {
                    if (isInCircle(data.front(), curr->data[i]->front(), radius)) {
                        found = true;
                    }
                }
                else {
                    if (isInRect(data.front(), curr->regions[i])) found = true;
                }
            }
            else if (*curr->data[i] == data) {
                found = true;
            }
            if (found) {
                auto parents = new list<pos>;
                parents->push_front({curr, i});
                return {true, parents};
            }
        }
        return {false, nullptr};
    }
}

void RTree::remove(const Data &data) {
    if (!root) return;
    auto bb = getBoundingBox(data);
    auto res = findLeaf(root, data, bb);
    // Si no se encontro, terminar
    if (!res.first) return;
    // Eliminar data
    auto parents = res.second;
    auto curr = parents->back().node;
    assert(curr->isLeaf);
    auto currIndex = parents->back().index;
    parents->pop_back();
    curr->regions.erase(curr->regions.begin() + currIndex);
    curr->data.erase(curr->data.begin() + currIndex);
    curr->rect = getBoundingRect(curr->regions);

    // Ajustar bounding boxes de los padres
    auto temp = curr;
    queue<Node*> toReinsert;
    while (!parents->empty() && temp != root) {
        auto p = parents->back();
        parents->pop_back();
        auto m = (int)order/2 + order%2;

        if (temp->regions.size() < m) {
            p.node->regions.erase(p.node->regions.begin() + p.index);
            p.node->childs.erase(p.node->childs.begin() + p.index);
            toReinsert.push(temp);
        }
        else p.node->regions[p.index] = getBoundingRect(temp->regions);
        p.node->rect = getBoundingRect(p.node->regions);

        temp = p.node;
    }
    if (root->childs.size() == 1) {
        auto oldRoot = root;
        root = root->childs.front();
        oldRoot->childs.clear();
        delete oldRoot;
    }
    // Reinsertar data huerfana
    reinsert2(toReinsert);
}

vector<knnResult> RTree::knn(Point p, int k) {
    using dist = struct {Node* node; int index; double distance; Point p;};
    auto cmp = [](dist a, dist b) {
        return a.distance > b.distance;
    };
    priority_queue<dist, vector<dist>, decltype(cmp)> nodes(cmp);
    vector<knnResult> knn;
    for (int i = 0; i < root->regions.size(); i++)
        if (!root->isLeaf)
            nodes.push({root, i, getDistance(p, root->regions[i])});
        else {
            auto line = getDistance(p, root->data[i]);
            nodes.push({root, i, line.distance, line.p});
        }

    while (!nodes.empty() && knn.size() < k) {
        auto curr = nodes.top();
        nodes.pop();
        if (!curr.node->isLeaf) {
            auto child = curr.node->childs[curr.index];
            for (int i = 0; i < child->regions.size(); i++) {
                if (!child->isLeaf)
                    nodes.push({child, i, getDistance(p, child->regions[i])});
                else {
                    auto line = getDistance(p, child->data[i]);
                    nodes.push({child, i, line.distance, line.p});
                }
            }
        }
        else {
            knn.push_back({curr.node, curr.index, curr.p});
        }
    }
    return knn;
}

vector<knnResult> RTree::depthFirst(Point p, int k) {
    using dist = struct {Node* node; int index; double distance; Point p;};
    auto cmp = [](dist a, dist b) {
        return a.distance < b.distance;
    };
    priority_queue<dist, vector<dist>, decltype(cmp)> nodes(cmp);
    double dmax = INT_MAX;
    stack<Node*> stack;
    stack.push(root);
    while (!stack.empty()) {
        auto curr = stack.top();
        stack.pop();
        if (!curr->isLeaf) {
            for (auto &c : curr->childs) {
                if (dmax + c->circle.radius < getDistance(p, c->circle.center)) continue;
                if (dmax + getDistance(p, c->circle.center) < c->minRadius) continue;
                stack.push(c);
            }
        }
        else {
            for (int i = 0; i < curr->regions.size(); i ++) {
                auto distancePtoMp = getDistance(p, curr->circle.center);
                if (dmax + getDistance(curr->circle.center, curr->data[i]).distance < distancePtoMp) continue;
                if (dmax + distancePtoMp < getDistance(curr->circle.center, curr->data[i]).distance) continue;
                if (k == 1 && distancePtoMp + curr->minRadius < dmax) dmax = distancePtoMp + curr->minRadius;

                auto line = getDistance(p, curr->data[i]);
                if (line.distance < dmax || nodes.size() < k) {
                    if (nodes.size() == k) nodes.pop();
                    nodes.push({curr, i, line.distance, line.p});
                    dmax = nodes.top().distance;
                }
            }
        }
    }
    vector<knnResult> result;
    while (!nodes.empty()) {
        result.push_back({nodes.top().node, nodes.top().index, nodes.top().p});
        nodes.pop();
    }
    return result;
}

void RTree::callKnn(Point p, int k) {
    knn(p, k);
}

static void useCirclesRec(Node* node) {
    // Solo funciona con puntos
    if (node->isLeaf) {
        // Calcular el centroide a partir de los puntos
        int cx = 0, cy = 0;
        int A = 0;
        for (auto &d : node->data) {
            cx += d->front().x * radius;
            cy += d->front().y * radius;
            A += radius;
        }
        Point centroid = {cx/A, cy/A};
        double maxD = 0;
        double minD = INT_MAX;
        for (auto &d : node->data) {
            auto distance = getDistance(centroid, d->front());
            if (distance > maxD) maxD = distance;
            if (distance < minD) minD = distance;
        }
        node->circle = {centroid, (int)maxD};
        node->minRadius = minD;
    }
    else {
        for (auto &c : node->childs) {
            useCirclesRec(c);
        }
        // Calcular el centroide a partir de los circulos hijos
        int cx = 0, cy = 0;
        int A = 0;
        for (auto &c : node->childs) {
            cx += c->circle.center.x * pow(c->circle.radius, 2);
            cy += c->circle.center.y * pow(c->circle.radius, 2);
            A += pow(c->circle.radius, 2);
        }
        Point centroid = {cx/A, cy/A};
        double maxD = 0;
        double minD = INT_MAX;
        for (auto &c : node->childs) {
            auto distance = getDistance(centroid, c->circle.center) + c->circle.radius;
            if (distance > maxD) maxD = distance;
            if (distance < minD) minD = distance;
        }
        node->circle = {centroid, (int)maxD};
        node->minRadius = minD;
    }
}

void RTree::useCircles() {
    useCirclesRec(root);
}

void RTree::search(const Data &data) {
    auto bb = getBoundingBox(data);
    stack<Node*> dfs;
    dfs.push(root);
    while (!dfs.empty()) {
        auto curr = dfs.top();
        dfs.pop();
        if (!curr->isLeaf) {
            for (auto &c : curr->childs) {
                if (isOverlapping(bb, c->rect)) dfs.push(c);
            }
        }
        else {
            for (auto &d : curr->data) {
                if (*d == data) break;
            }
        }
    }
}

static void showNode(Node* node, cv::InputOutputArray &img) {
    if (node == nullptr) return;
    if (node->isLeaf) {
        for (int i = 0; i < node->regions.size(); i++) {
            if (node->data[i]->size() == 1) {
                cv::circle(img, node->data[i]->back(), radius, colors[0], -1);
            }
            else {
                cv::rectangle(img, {node->regions[i].x_low, node->regions[i].y_low}, {node->regions[i].x_high-1, node->regions[i].y_high-1}, {0,0,0}, 1);
                cv::polylines(img, *node->data[i], true, colors[0], 2);
            }
        }
    }
    else {
        for (const auto &r : node->regions) {
            cv::rectangle(img, {r.x_low, r.y_low}, {r.x_high-1, r.y_high-1}, colors[node->level%6], 2);
        }
        for (const auto &c : node->childs) {
//            cv::circle(img, c->circle.center, c->circle.radius, colors[colorIdx%6], 2);
            showNode(c, img);
        }
    }
}

void RTree::show(cv::InputOutputArray &img) {
    showNode(root, img);
}

void RTree::clear() {
    delete root;
    root = nullptr;
}

double RTree::getLeafsOverlap() {
    vector<Rect> rects;
    stack<Node*> dfs;
    dfs.push(root);
    while (!dfs.empty()) {
        auto curr = dfs.top();
        dfs.pop();
        if (curr->isLeaf) {
            for (auto r : curr->regions) rects.push_back(r);
        }
        else {
            for (auto &c : curr->childs) dfs.push(c);
        }
    }
    return getTotalOverlap2(rects);
}

double RTree::getInternalOverlap() {
    stack<Node*> dfs;
    double overlap = 0.0;
    int area = 0;
    dfs.push(root);
    while (!dfs.empty()) {
        auto curr = dfs.top();
        dfs.pop();
        if (!curr->isLeaf) {
            for (auto r : curr->regions) area += getArea(r);
            overlap += getTotalOverlap2(curr->regions);
        }
    }
    return overlap / (area - overlap);
}
