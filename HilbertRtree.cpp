//
// Created by Sebastian Knell on 14/06/22.
//

#include "HilbertRtree.h"

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

    // agrego hermanos cooperantes (izq -> der)
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
                currNode->children[j]->parent = currNode;
            }
            /* empiezan cambios */
            currNode->regions.push_back(C.top().rect);
            /* terminan cambios */
            C.pop();
        }
        // actualizar currNode
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
            lastNode->children[i]->parent = lastNode;
        }
        /* empiezan cambios */
        lastNode->regions.push_back(C.top().rect);
        /* terminan cambios */
        C.pop();
    }
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

void HilbertNode::updateLHV() {
    if (isLeaf) {
        this->lhv = data[data.size()-1].hIndex;
    } else {
        this->lhv = children[children.size()-1]->lhv;
    }
}
    
// creo que si se podria pasar un puntero NN si es que se hizo split aquí
void HilbertRtree::adjustTree(HilbertNode* node) { //, Node* NN
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

void HilbertRtree::insert(const Data& obj) {
    auto R = getBoundingBox(obj);
    auto h = getHilbertIndex(getCenter(R));
    HData hd(obj, h);

    HilbertNode* node = chooseLeaf(root, h);
    node->insertOrdered(hd, R);

    if (node->data.size() > M) {
        handleOverflow(node);
    }

    adjustTree(node);
}

/////////////////////////////////////////////////////////////////////////

bool operator==(Data data1, Data data2) {
    if (data1.size() != data2.size()) return false;

    for (int i=0; i<data1.size(); i++)
        if (data1[i] != data2[i])
            return false;
    
    return true;
}

pair<int, HilbertNode*> HilbertRtree::searchUtil(const Data obj) {
    auto R = getBoundingBox(obj);
    auto h = getHilbertIndex(getCenter(R));

    HilbertNode* node = chooseLeaf(root, h);
    for (int i=0; i<node->data.size(); i++) {
        if (obj.size() == 1 &&
            (node->data[i].data.size() == 1 && isInCircle(node->data[i].data[0], obj[0], 20) || isInRect(obj.front(), node->regions[i]))
            )
        {
            return make_pair(i, node);
        } else if ((node->data[i].data.size() > 1 && obj.size() == 1 && isInRect(obj.front(), node->regions[i])) ||
                    node->data[i].data == obj) {
            return make_pair(i, node);
        }
    }

    return make_pair(-1, nullptr);
}

void HilbertRtree::search(const Data& obj) {
    searchUtil(obj);
}

void HilbertRtree::clear() {
    delete this->root;                  // elimina los nodos hijo
    this->root = new HilbertNode(true); // crea un nodo vacio
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

void HilbertRtree::handleUnderflow(HilbertNode* v) {
    HilbertNode* p = v->parent;
    vector<HilbertNode*> S;

    // agrego hermanos cooperantes (izq -> der), si 
    for (auto x : p->children) {
        if (x != v)
            S.push_back(x);
        else {
            S.push_back(v);
            if (S.size() == 1) continue;
            break;
        }
    }

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

    // si todos los nodos en S tienen underflow...
    if (S.size()*m > C.size()) {
        // actualizar lista children del padre
        // auto it = S.begin();
        for (int i=0; i< S[0]->parent->children.size(); i++) {
            if (S[0]->parent->children[i] == S[0]) {
                auto it = S[0]->parent->children.begin() + i;
                auto it2 = S[0]->parent->regions.begin() + i;
                S[0]->parent->children.erase(it);
                S[0]->parent->regions.erase(it2);
                break;
            }
        }
        // eliminar el nodo de S
        S.erase(S.begin());
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
                currNode->children[j]->parent = currNode;
            }
            /* empiezan cambios */
            currNode->regions.push_back(C.top().rect);
            /* terminan cambios */
            C.pop();
        }
        // actualizar currNode
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
            lastNode->children[i]->parent = lastNode;
        }
        /* empiezan cambios */
        lastNode->regions.push_back(C.top().rect);
        /* terminan cambios */
        C.pop();
    }
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

    /* if (p->children.size() < m) {
        if (p == root) {    // root && leaf, root notLeaf
            //root = root->children[0];
            
            // hijos del padre son hojas, el padre se vuelve hoja

            
            * si es root y hoja y se eliminan nodos, sí o sí va a quedar el caso de 1 solo hijo pq el root puede tener menos del minimo
            * si es hoja y no es root, va a juntarse con su hermano e hipoteticamente llegará al caso en el que haya menos de "m"
            en ese caso, (suponiendo que sean 5-10), el sgt nodo arriba se queda con 4 hijos, por lo que como no puede pasar esto, si el sgt nodo es el root
            y el root es el único que puede tener menos de m, este nodo se vuelve el root, si no es el root, necesariamente debe tener un hermano a la izquierda o
            a la derecha, asi que se junta con ese hermano.
            
            
            if (p->children[0]->isLeaf) {
                p->isLeaf = true;
                p->data.clear();
                p->regions.clear();
                for (auto x : p->children) {
                    p->data.push_back(x->data);
                    p->regions.push_back(getBoundingBox(x.data));
                }
            } else {    // hijos del padre no son hoja
                p->isLeaf = true;
                p->children.clear();
                p->regions.clear();
                for (int i=0; i<p->children.size(); i++) {
                    p->children.push_back(p->children[i]);
                    p->regions.push_back(p->children[i]->rect);
                }
            }
            
            root->parent = nullptr;
        } else {
            p = p->children[0];
        }
    } */

    if (p->children.size() < m && p != root) {
        handleUnderflow(p);
    }

    if (p == root && p->children.size() == 1) {
        root = p->children[0];
    }
}

void HilbertRtree::remove(const Data& obj) {
    auto n = searchUtil(obj);
    if (n.first == -1) {
        return;
    }

    HilbertNode* node = n.second;
    auto it = node->data.begin() + n.first;
    auto it2 = node->regions.begin() + n.first;

    node->data.erase(it);
    node->regions.erase(it2);

    if (node->data.size() < m && node != root) handleUnderflow(node);

    if (node->data.size() > 0)
        adjustTree(node);
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
