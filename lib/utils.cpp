//
// Created by Sebastian Knell on 6/07/22.
//

#include "utils.h"

int radius = 4;

cv::Scalar colors[] = {
        {255, 170, 130},
        {233, 145, 198},
        {107, 203, 255},
        {255, 221, 137},
        {141, 232, 195},
        {106, 100, 208}
};

bool isInCircle(Point p, Point circleP, int r) {
    return (pow(p.x - circleP.x, 2) + pow(p.y - circleP.y, 2)) <= pow(r, 2);
}

bool isInRect(const Point &p, Rect rect) {
    if (rect.x_low > rect.x_high) swap(rect.x_low, rect.x_high);
    if (rect.y_low > rect.y_high) swap(rect.y_low, rect.y_high);
    return p.x >= rect.x_low - radius && p.x <= rect.x_high + radius && p.y >= rect.y_low - radius && p.y <= rect.y_high + radius;
}

bool operator==(const Rect &a, const Rect &b) {
    return a.x_low == b.x_low && a.x_high == b.x_high && a.y_low == b.y_low && a.y_high == b.y_high;
}

int getPerimeterEnlargement(Rect region, Rect r) {
    auto x_low = min(region.x_low, r.x_low);
    auto y_low = min(region.y_low, r.y_low);
    auto x_high = max(region.x_high, r.x_high);
    auto y_high = max(region.y_high, r.y_high);
    return ((x_high - x_low) + (y_high - y_low)) - ((region.x_high - region.x_low) + (region.y_high - region.y_low));
}

double getDistance(Point p, Rect rect) {
    auto x_dif = min(abs(p.x - rect.x_low), abs(p.x - rect.x_high));
    auto y_dif = min(abs(p.y - rect.y_low), abs(p.y - rect.y_high));
    return sqrt(pow(x_dif, 2) + pow(y_dif, 2));
}

double getDistance(Point a, Point b) {
    return getDistance(a, {b.x, b.y, b.x, b.y});
}

Rect getBoundingBox(const Data &data) {
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

Rect getBoundingRect(const vector<Rect> &regions) {
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

Point getCenter(const Rect &rect) {
    return {(rect.x_low + rect.x_high) / 2, (rect.y_low + rect.y_high) / 2};
}

int getArea(const Rect &rect) {
    return (rect.x_high - rect.x_low) * (rect.y_high - rect.y_low);
}

bool rectsOverlap(Rect r1, Rect r2) {
    if (isInRect({r1.x_high, r1.y_high}, r2) || isInRect({r1.x_high, r1.y_low}, r2) || 
            isInRect({r1.x_low, r1.y_high}, r2) || isInRect({r1.x_low, r1.y_high}, r2)) return true;
    if (isInRect({r2.x_high, r2.y_high}, r1) || isInRect({r2.x_high, r2.y_low}, r1) || 
            isInRect({r2.x_low, r2.y_high}, r1) || isInRect({r2.x_low, r2.y_high}, r1)) return true;
    return false;
}
