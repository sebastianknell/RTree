//
// Created by Sebastian Knell on 14/05/22.
//

#include "RTree.h"

int radius = 4;

bool isInCircle(Point p, Point circleP, int r) {
    return (pow(p.x - circleP.x, 2) + pow(p.y - circleP.y, 2)) <= pow(r, 2);
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
    auto group1 = new Node(node->isLeaf), group2 = new Node(node->isLeaf);
    addRegion(group1, node->regions[seeds.first]);
    addRegion(group2, node->regions[seeds.second]);
    node->regions.erase(node->regions.begin() + seeds.first);
    if (seeds.first < seeds.second) seeds.second -= 1; // los indices pueden cambiar luego de borrar
    node->regions.erase(node->regions.begin() + seeds.second);
    if (node->isLeaf) {
        group1->data.push_back(node->data[seeds.first]);
        group2->data.push_back(node->data[seeds.second]);
        node->data.erase(node->data.begin() + seeds.first);
        node->data.erase(node->data.begin() + seeds.second);
    }
    else {
        group1->childs.push_back(node->childs[seeds.first]);
        group2->childs.push_back(node->childs[seeds.second]);
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

int colorIdx = 0;

static void showNode(Node* node, cv::InputOutputArray &img) {
    // TODO pintar el arbol de arriba hacia abajo para que las hojas no pinten por encima a las regiones superiores
    if (node == nullptr) return;
    for (const auto &r : node->regions) {
        cv::rectangle(img, {r.x_low, r.y_low}, {r.x_high-1, r.y_high-1}, colors[colorIdx%6], 2);
    }
    colorIdx++;
    if (node->isLeaf) {
        for (const auto &d : node->data) {
            if (d->size() == 1) {
                cv::circle(img, d->front(), radius, colors[3], -1);
            }
            else {
//                cv::polylines(img, *d, true, colors[3], 2);
                cv::fillPoly(img, *d, colors[3]);
            }
        }
    }
    else {
        for (const auto &c : node->childs) {
            showNode(c, img);
        }
    }
}

void RTree::show(cv::InputOutputArray &img) {
    colorIdx = 0;
    showNode(root, img);
}

Node* RTree::findLeaf(Node* current, const Data& record) {

    // [Search Subtrees] if T is not a leaf, check each entry F in T to determine
    // if Fi overlaps Ei for each such entry invoke findLeaf on the tree whose
    // root is pointed to by Fp until E is found or all entries have been checked
    if(!current->isLeaf) {

        for(auto reg : current->regions){
            // Check if overlap? -> Hacer funcion externa?
            if(reg.x_high >= record.front().x && reg.x_low <= record.front().x){
                if(reg.y_high >= record.front().y & reg.y_low <= record.front().y){
                    findLeaf() // Que es lo que le tengo que comparar? region?
                    // Tengo pasarle un child a la funcion?
                }
            }
        }
    }

    // [Search leaf node for record] If T is a leaf, check each entry to see if it
    // matches E if E is found return T.
    else{

    }
}

void RTree::condenseTree(Node* node) {

    // Dado un nodo hoja L donde se ha eliminado un entry, eliminar el nodo
    // si es que tiene muy pocas entries y relocalizarlas. Propagar la eliminacion
    // hacia arriba como sea necesario.
    // Ajustar los rectangulos en el camino al root, achicandolos si es posible.

    // step1: Set N = L, Set Q as empty -> the set of eliminated nodes

    // step2: [Find parent entry] If N is the root, go to step6. Otherwise, let P be the parent
    // of N and let En be N's entry in P.

    // step3: [Eliminate under-full node]
    // if N has fewer than m entires, delete En from P and add N to set Q.

    // step4: [Adjust covering rectangle]
    // if N has not been eliminated, adjust En I to tightly contain all entries in N.

    // step5: [Move up one level in tree]
    // Set N = P and repeat from step2.

    // step6: [Re-insert orphaned entires]
    // Reinsert all entries of nodes in set Q.
    // Entires from eliminated leaf nodes are reinserted in tree leaves as described
    // in Algorithm Insert, but entries from higher-level nodes must be placed higeher
    // in the tree, so that leaves of their dependent subtrees will be on the same level as leaves
    // of the main tree.

}

void RTree::remove(Point) {

    // FIND node containing containing record E.

    // Invoke FindLeaf to locate the Leaf node L containing E. Stop if E was not found.

    // Delete record. -> Remove E from L.

    // Propagate changes. Invoke CondenseTree, passing L.

    // Shorten tree. If the root node has only one child after the tree has been adjusted,
    // make the child the new root.

}
