//
// Created by Sebastian Knell on 14/06/22.
//

#include "HilbertRtree.h"

bool isInRect(const Point &p, Rect rect) {
    if (rect.xlow > rect.xhigh) swap(rect.xlow, rect.xhigh);
    if (rect.ylow > rect.yhigh) swap(rect.ylow, rect.yhigh);
    return p.x >= rect.xlow && p.x <= rect.xhigh && p.y >= rect.ylow && p.y <= rect.yhigh;
}

int HilbertRtree::getHilbertIndex(Point p, int x, int y, int xi, int xj, int yi, int yj, int n, int index) {
    // Ver en que cuadrante esta
    // Cuadrante 1
    if (isInRect(p, {x, y, x+xi/2+yi/2, y+xj/2+yj/2})) {
        if (n <= 0) return index;
        return getHilbertIndex(p, x, y, yi/2, yj/2, xi/2, xj/2, n-1, index);
    }
    // Cuadrante 2
    else if (isInRect(p, {x+xi/2, y+xj/2, x+xi+yi/2, y+xj+yj/2})) {
        if (n <= 0) return index + 1;
        return getHilbertIndex(p, x+xi/2, y+xj/2, xi/2, xj/2, yi/2, yj/2, n-1, index + 4);
    }
    // Cuadrante 3
    else if (isInRect(p, {x+xi/2+yi/2, y+xj/2+yj/2, x+xi+yi, y+xj+yj})) {
        if (n <= 0) return index + 2;
        return getHilbertIndex(p, x+xi/2+yi/2, y+xj/2+yj/2, xi/2, xj/2, yi/2, yj/2, n-1, index + 8);
    }
    // Cuadrante 4
    else {
        if (n <= 0) return index + 3;
        return getHilbertIndex(p, x+xi/2+yi, y+xj/2+yj, -yi/2, -yj/2, -xi/2, -xj/2, n-1, index + 12);
    }
}
