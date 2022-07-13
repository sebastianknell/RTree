//
// Created by Sebastian Knell on 14/06/22.
//

#include "HilbertRtree.h"

/*static void addRegion(Node* node, Rect region) {
    if (node->regions.empty()) node->rect = region;
    else {
        node->rect.x_low = min(node->rect.x_low, region.x_low);
        node->rect.x_high = max(node->rect.x_high, region.x_high);
        node->rect.y_low = min(node->rect.y_low, region.y_low);
        node->rect.y_high = max(node->rect.y_high, region.y_high);
    }
    node->regions.push_back(region);
}*/

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

HilbertNode* HilbertRtree::chooseLeaf(HilbertNode* node, int h) {
    if (node->isLeaf) return node;

    HilbertNode* t = nullptr;
    for (int i = 0; i < node->children.size(); i++) {
        if (node->children[i]->lhv >= h || i == node->children.size()-1) {
            t = node->children[i];
            break;
        }
    }
    return chooseLeaf(t, h);
}

static bool operator>(Entry a, Entry b) {
    if (a.type && b.type) {     // child
        return a.child->lhv > b.child->lhv;
    } else {                    // data
        return a.data.hIndex > b.data.hIndex;
    }
}

static bool operator<(Entry a, Entry b) {
    if (a.type && b.type) {     // child
        return a.child->lhv < b.child->lhv;
    } else {                    // data
        return a.data.hIndex < b.data.hIndex;
    }
}

void HilbertRtree::handleOverflow(HilbertNode* v) {
    if (v == root) {
        HilbertNode* newRoot = new HilbertNode(0);
        newRoot->rect = v->rect;
        newRoot->lhv = v->lhv;
        // hacer que v se vuelva hijo de root
        v->parent = newRoot;
        newRoot->children.push_back(v);
        root = newRoot;
    }
    HilbertNode* p = v->parent;
    p->lvl = v->lvl+1;
    vector<HilbertNode*> S;

    // agrego los que no son v (izq -> der)
    for (auto x : p->children) {
        if (x != v)
            S.push_back(x);
        else
            break;
    }
    S.push_back(v);

    // pasar las entradas de todos los nodos en S a C ordenadas por LHV
    priority_queue<Entry, vector<Entry>, greater<vector<Entry>::value_type>> C;
    for (auto node : S) {
        Entry e;
        if (node->isLeaf) {
            for (auto entry : node->data) {
                e.type = 0;
                e.data = entry;
                e.rect = getBoundingBox(entry.data);
                // las regiones en los nodos hoja son los bounding boxes
                C.push(e);
            }
        } else {
            for (auto entry : node->children) {
                e.type = 1;
                e.child = entry;
                e.rect = entry->rect;
                //getBoundingRect(entry->regions);   // capaz es innecesario
                C.push(e);
            }
        }
    }

    // si todos los nodos en S tienen overflow...
    if (S.size()*M < C.size()) {
        HilbertNode* w = new HilbertNode(v->isLeaf);
        w->parent = p;
        p->children.insert(p->children.begin(), w);
        S.insert(S.begin(), w);
    }

    int q = C.size() / S.size();
    for (int i = 0; i < S.size()-1; i++) {
        HilbertNode* currNode = S[i];
        if (currNode->isLeaf) currNode->data.clear();
        else currNode->children.clear();
        
        /* empiezan cambios */
        currNode->regions.clear();
        /* terminan cambios */

        for (int j = 0; j < q; j++) {
            if (currNode->isLeaf) {
                currNode->data.push_back(C.top().data);
            } else {
                currNode->children.push_back(C.top().child);
            }
            /* empiezan cambios */
            currNode->regions.push_back(C.top().rect);
            /* terminan cambios */
            C.pop();
        }
        // actualizar currNode
        //currNode->updateBoundingBox();
        
        currNode->updateLHV();
        // actualizando los bounding rects de los nodos hoja
        currNode->rect = getBoundingRect(currNode->regions);
    }

    // agregar elementos excedentes al ultimo nodo
    HilbertNode* lastNode = S[S.size()-1];
    if (lastNode->isLeaf) lastNode->data.clear();
    else lastNode->children.clear();
    lastNode->regions.clear();
    
    int it = C.size();
    for (int i = 0; i < it; i++) {
        if (lastNode->isLeaf) {
            lastNode->data.push_back(C.top().data);
        } else {
            lastNode->children.push_back(C.top().child);
        }
        /* empiezan cambios */
        lastNode->regions.push_back(C.top().rect);
        /* terminan cambios */
        C.pop();
    }
    //lastNode->updateBoundingBox();
    lastNode->updateLHV();
    // actualizando el bounding rect del lastNode
    lastNode->rect = getBoundingRect(lastNode->regions);

    // actualizando el vector de regions de el nodo padre
    p->regions.clear();
    for (auto child : p->children)
        p->regions.push_back(child->rect);
    
    // actualizando el bounding rect del padre
    p->rect = getBoundingRect(p->regions);
    adjustTree(p);

    if (p->children.size() > M) handleOverflow(p);
}

void HilbertNode::updateBoundingBox() {
    int x_min = INT_MAX;
    int y_min = INT_MAX;
    int x_max = 0;
    int y_max = 0;

    for (auto rect : this->regions) {
        if (rect.x_low < x_min) x_min = rect.x_low;
        if (rect.y_low < y_min) y_min = rect.y_low;
        if (rect.x_high > x_max) x_max = rect.x_high;
        if (rect.y_high > y_max) y_max = rect.y_high;
    }
    /*
    this->rect.x_low = x_min;
    this->rect.y_low = y_min;
    this->rect.x_high = x_max;
    this->rect.y_high = x_max;
    */
}

void HilbertNode::updateLHV() {
    if (isLeaf) {
        this->lhv = data[data.size()-1].hIndex;
    } else {
        this->lhv = children[children.size()-1]->lhv;
    }
}
    
// creo que si se podria pasar un puntero NN si es que se hizo split aquí
void HilbertRtree::adjustTree(HilbertNode* node) { //, Node* NN
    // node->updateBoundingBox();
    if (node->isLeaf) {
        for (int i=0; i<node->data.size(); i++) {
            node->regions[i] = getBoundingBox(node->data[i].data);
        }
    }
    node->rect = getBoundingRect(node->regions);
    node->updateLHV();

    if ( /*node != root*/ node->parent != nullptr) {
        node->parent->regions.clear();
        for (int i=0; i<node->parent->children.size(); i++) {
            // node->parent->regions[i] = node->parent->children[i]->rect;
            node->parent->regions.push_back(node->parent->children[i]->rect);
            // node->parent->regions[i] = getBoundingRect(node->parent->children[i]->regions);
        }
        adjustTree(node->parent);
    }
}

void HilbertNode::insertOrdered(HData hdata, Rect region) {
    if (this->isLeaf) {
        int insertPos = 0;
        for (int i = 0; i < data.size(); i++) {
            int currIdx = data[i].hIndex;
            if (hdata.hIndex > currIdx)
                insertPos = i+1;
            else
                break;
        }
        data.insert(data.begin() + insertPos, hdata);
        regions.insert(regions.begin() + insertPos, region);
    }
}

void HilbertRtree::insert(const Data obj) {
    auto R = getBoundingBox(obj);
    auto h = getHilbertIndex(getCenter(R));
    HData hd(obj, h);

    HilbertNode* node = chooseLeaf(root, h);
    node->insertOrdered(hd, R);

    if (node->data.size() > M) {
        cout << "---------- ENTRA: ----------" << endl;
        handleOverflow(node);
    }

    adjustTree(node);

    // actualizar el rect del nodo en el que se inserto la data
    // node->rect = getBoundingRect(node->regions);
    ;
}

/////////////////////////////////////////////////////////////////////////

bool operator==(Data data1, Data data2) {
    if (data1.size() != data2.size()) return false;

    for (int i=0; i<data1.size(); i++)
        if (data1[i] != data2[i])
            return false;
    
    return true;
}

bool HilbertRtree::search(const Data obj) {
    // como la data es la misma se puede buscar con el indice de hilbert

    auto R = getBoundingBox(obj);
    auto h = getHilbertIndex(getCenter(R));

    // TODO
    HilbertNode* node = chooseLeaf(root, h);
    for (int i=0; i<node->data.size(); i++)
        if (node->data[i].data == obj)
            return true;    

    return false;
}

static lineToH getDistanceToSegment(Point p, Point a, Point b) {
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

static lineToH getDistance(Point p, Data *data) {
    if (data->size() == 1) return {data->front(), getDistance(p, data->front())};
    data->push_back(data->front());
    lineToH min = {{}, INT_MAX};
    for (int i = 0; i < data->size() - 1; i++) {
        auto distanceToSide = getDistanceToSegment(p, data->at(i), data->at(i+1));
        if (distanceToSide.distance < min.distance) min = distanceToSide;
    }
    return min;
}

vector<knnResultH> HilbertRtree::knn(Point p, int k) {
    using distance = struct {HilbertNode* node; int index; double distance; Point p;};
    auto cmp = [](distance a, distance b) {
        return a.distance > b.distance;
    };
    priority_queue<distance, vector<distance>, decltype(cmp)> nodes(cmp);
    vector<knnResultH> knn;
    for (int i = 0; i < root->regions.size(); i++)
        if (!root->isLeaf)
            nodes.push({root, i, getDistance(p, root->regions[i])});
        else {
            auto line = getDistance(p, &root->data[i].data);
            nodes.push({root, i, line.distance, line.p});
        }

    while (!nodes.empty() && knn.size() < k) {
        auto curr = nodes.top();
        nodes.pop();
        if (!curr.node->isLeaf) {
            auto child = curr.node->children[curr.index];
            for (int i = 0; i < child->regions.size(); i++) {
                if (!child->isLeaf)
                    nodes.push({child, i, getDistance(p, child->regions[i])});
                else {
                    auto line = getDistance(p, &child->data[i].data);
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

/////////////////////////////////////////////////////////////////////////

void HilbertRtree::handleUnderflow(HilbertNode* node) {
    HilbertNode* p = node->parent;
    vector<HilbertNode*> S;

    ;

    ;
}

void HilbertRtree::remove(const Data obj, int type) {
    /* Rect window_q = {q.x, q.y, q.x, q.y};

    // encontrar la hoja v que puede contener la data
    HilbertNode* v = findLeaf(root, window_q);
    if (v == nullptr) {
        cout << "Object was not found" << endl;
        return;
    }; 

    // recorrer v y borrar la data si se encuentra
    bool found = false;
    for (auto it = v->data.begin(); it != v->data.end();) {
        if () {
            it = v->data.erase(it);
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "Object was not found" << endl;
        return;
    } 

    if (v->data.size() < m) handleUnderflow(v);
    adjustTree(v); */
}

/////////////////////////////////////////////////////////////////////////


int colorIdx2 = 0;

static void showHilbertNode(HilbertNode* node, cv::InputOutputArray &img) {
    if (node == nullptr) return;
    colorIdx2++;
    if (node->isLeaf) {
        for (int i = 0; i < node->regions.size(); i++) {
            if (node->data[i].data.size() == 1) {
                cv::circle(img, node->data[i].data.back(), radius, colors[0], -1);
            }
            else {
                cv::rectangle(img, {node->regions[i].x_low, node->regions[i].y_low}, {node->regions[i].x_high-1, node->regions[i].y_high-1}, {0,0,0}, 1);
                cv::polylines(img, node->data[i].data, true, colors[0], 2);
            }
        }
    }
    else {
        for (const auto &r : node->regions) {
            cv::rectangle(img, {r.x_low, r.y_low}, {r.x_high-1, r.y_high-1}, colors[node->lvl%6], 2); // level
        }
        for (const auto &c : node->children) {
//            cv::circle(img, c->circle.center, c->circle.radius, colors[colorIdx%6], 2);
            showHilbertNode(c, img);
        }
    }
}

void HilbertRtree::showHilbert(cv::InputOutputArray& img) {
    colorIdx2 = 0;
    cv::rectangle(img, {root->rect.x_low, root->rect.y_low}, {root->rect.x_high-1, root->rect.y_high-1}, colors[root->lvl%6], 2);
    showHilbertNode(root, img);
}
