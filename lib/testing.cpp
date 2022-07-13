//
// Created by Sebastian Knell on 9/07/22.
//

#include "testing.h"

bool isOverlapping(Rect r1, Rect r2){
    if((r1.x_high == r2.x_high) || (r1.x_low==r2.x_low) || (r1.y_high=r2.y_high) || (r1.y_low == r2.y_low))
        return false;
    if (r1.x_high > r2.x_high || r2.x_low > r1.x_low)
        return false;
    if (r1.y_high > r2.y_high || r2.y_low > r1.y_low)
        return false;

    return true;
}

double getOverlap(Rect r1, Rect r2) {
    double Ix, Iy, R1x, R1y, R2x, R2y;
    R1x = r1.x_high - r1.x_low;
    R1y = r1.y_high - r1.y_low;

    R2x = r2.x_high - r2.x_low;
    R2y = r2.y_high - r2.y_low;

    if (min(r1.x_high,r2.x_high)==r1.x_high)  // min(x1a,x1b) = x1a
        Ix = max(0, r1.x_low - r2.x_high);   // max(0, x2a-x1b)
    else
        Ix = max(0, r2.x_low - r1.x_high);

    if (min(r1.y_high,r2.y_high)==r1.y_high)  // min(y1a,y1b) = y1a
        Iy = max(0, r1.y_low - r2.y_high);   // max(0, y2a-y1b)
    else
        Iy = max(0, r2.y_low - r1.y_high);


    double coef = (Ix * Iy) / (R1x * R1y);

    return coef;
}

double getTotalOverlap(vector<Rect> &rects) {
    double intersection = 0.0;
    double total = 0.0;
    for (int i = 0; i < rects.size(); i++) {
        for (int j = i+1; j < rects.size(); j++) {
            auto overlap = getOverlap(rects[i], rects[j]);
            auto S = getArea(rects[i]) + getArea(rects[j]) - overlap;
            intersection += overlap;
            total += S;
        }
    }
    return intersection / total;
}

Data generatePolygon(int gridWidth, int gridHeight) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> randomPercent(100,300);
    int width = gridWidth * (randomPercent(rng) / 100.0) / 100.0;
    int height = gridHeight * (randomPercent(rng) / 100.0) / 100.0;
    std::uniform_int_distribution<std::mt19937::result_type> random(width,720);
    int x = random(rng);
    int y = random(rng);
    Rect bb = {x-width/2, y-height/2, x+width/2, y+height/2};
    Data polygon;
    // coger 1 o 2 puntos de cada lado
    int n;
    Point p1;
    Point p2;
    // arriba
    n = random(rng) % 2 + 1;
    std::uniform_int_distribution<std::mt19937::result_type> random1(bb.x_low, bb.x_low+width);
    p1 = {(int)random1(rng), bb.y_low};
    if (n == 2) {
        p2 = {(int)random1(rng), bb.y_low};
        polygon.push_back(p1.x < p2.x ? p1 : p2);
        polygon.push_back(p1.x < p2.x ? p2 : p1);
    }
    else polygon.push_back(p1);
    // derecha
    n = random(rng) % 2 + 1;
    std::uniform_int_distribution<std::mt19937::result_type> random2(bb.y_low,bb.y_low+height);
    p1 = {bb.x_high, (int)random2(rng)};
    if (n == 2) {
        p2 = {bb.x_high, (int)random2(rng)};
        polygon.push_back(p1.y < p2.y ? p1 : p2);
        polygon.push_back(p1.y < p2.y ? p2 : p1);
    }
    else polygon.push_back(p1);
    // abajo
    p1 = {(int)random1(rng), bb.y_high};
    if (n == 2) {
        p2 = {(int)random1(rng), bb.y_high};
        polygon.push_back(p1.x < p2.x ? p2 : p1);
        polygon.push_back(p1.x < p2.x ? p1 : p2);
    }
    else polygon.push_back(p1);
    // izquierda
    p1 = {bb.x_low, (int)random2(rng)};
    if (n == 2) {
        p2 = {bb.x_low, (int)random2(rng)};
        polygon.push_back(p1.y < p2.y ? p2 : p1);
        polygon.push_back(p1.y < p2.y ? p1 : p2);
    }
    else polygon.push_back(p1);
    return polygon;
}

void testOverlap() {
    rapidcsv::Document doc;
    doc.SetColumnName(0, "n");
    doc.SetColumnName(1, "overlap");
    vector<Rect> rects;
    const int testSize = 1111;
    const int iterations = 10;
    auto polygon = generatePolygon(1000, 1000);
    rects.push_back(getBoundingBox(polygon));
    vector<double> overlaps(testSize, 0);
    for (int k = 0; k < iterations; k++) {
        for (int i = 1; i < overlaps.size(); i++) {
            polygon = generatePolygon(1000, 1000);
            rects.push_back(getBoundingBox(polygon));
            auto currentOverlap = getTotalOverlap(rects);
            overlaps[i] += currentOverlap;
        }
        rects.clear();
        polygon = generatePolygon(1000, 1000);
        rects.push_back(getBoundingBox(polygon));
    }
    for (int i = 0; i < overlaps.size(); i++) {
        doc.InsertRow<double>(i, {(double)i+1, overlaps[i] / iterations});
    }
    doc.Save("../output/overlap.csv");
}

void testInsert(Tree &tree) {
    rapidcsv::Document doc;
    doc.SetColumnName(0, "n");
    doc.SetColumnName(1, "time");
    for (int i = 0; i < 500; i++) {
        auto t1 = chrono::high_resolution_clock::now();
        for (int j =0; j < 10; j++) {
            auto polygon = generatePolygon(720, 720);
            tree.insert(polygon);
        }
        auto t2 = chrono::high_resolution_clock::now();
        long duration = chrono::duration_cast<chrono::microseconds>(t2-t1).count();
        doc.InsertRow<long>(i, {(i+1)*10, duration});
    }
    doc.Save("../output/insert.csv");
}
