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

int getOverlap(Rect r1, Rect r2) {
    auto lx = max(0, min(r1.x_high, r2.x_high) - max(r1.x_low, r2.x_low));
    auto ly = max(0, min(r1.y_high, r2.y_high) - max(r1.y_low, r2.y_low));
    return lx * ly;
}

double getTotalOverlap(vector<Rect> &rects) {
    int intersection = 0;
    int total = 0;
    for (int i = 0; i < rects.size(); i++) {
        for (int j = i+1; j < rects.size(); j++) {
            int overlap = getOverlap(rects[i], rects[j]);
            int S = getArea(rects[i]) + getArea(rects[j]) - overlap;
            intersection += overlap;
            total += S;
        }
    }
    return (double) intersection / total;
}

Data generatePolygon(int gridWidth, int gridHeight) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> randomPercent(42,84);
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
    const int gridSize = 1000;
    auto polygon = generatePolygon(gridSize, gridSize);
    rects.push_back(getBoundingBox(polygon));
    vector<double> overlaps(testSize, 0);
    for (int k = 0; k < iterations; k++) {
        for (int i = 1; i < overlaps.size(); i++) {
            polygon = generatePolygon(gridSize, gridSize);
            rects.push_back(getBoundingBox(polygon));
            auto currentOverlap = getTotalOverlap(rects);
            overlaps[i] += currentOverlap;
        }
        rects.clear();
        polygon = generatePolygon(gridSize, gridSize);
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
    vector<long> times(500, 0);
    const int iterations = 1;
    vector<Data> polygons;
    for (int i = 0; i < 5000; i++){
        polygons.push_back(generatePolygon(512, 512));
    }
    int a = 0;
    for (int k = 0; k < iterations; k++) {
        for (int i = 0; i < 500; i++) {
            auto t1 = chrono::high_resolution_clock::now();
            for (int j =0; j < 10; j++) {
                tree.insert(polygons[a]); a++;
            }
            auto t2 = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(t2-t1).count();
            times[i] += duration;
        }
        tree.clear();
        a = 0;
    }
    for (int i = 0; i < times.size(); i++) {
        doc.InsertRow<double>(i, {(double)(i+1)*10, (double)times[i] / iterations});
    }
    doc.Save("../output/insert.csv");
}

void testSearch(Tree &tree) {
    rapidcsv::Document doc;
    doc.InsertColumn<int>(0, {1000, 2000, 3000, 4000, 5000}, "n");
    const int iterations = 100;
    for (int k = 0; k < iterations; k++) {
        vector<long> times;
        for (int i = 0; i < 5; i++) {
            vector<Data> polygons;
            for (int j =0; j < 1000; j++) {
                auto polygon = generatePolygon(720, 720);
                polygons.push_back(polygon);
                tree.insert(polygon);
            }
            auto t1 = chrono::high_resolution_clock::now();
            for (auto &p : polygons) {
                tree.search(p);
            }
            auto t2 = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(t2-t1).count();
            times.push_back(duration);
        }
        doc.InsertColumn(k+1, times, "times" + to_string(k+1));
        tree.clear();
    }
    doc.Save("../output/search.csv");
}

void testRemove(Tree &tree) {
    rapidcsv::Document doc;
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> random(0,5000);
    doc.InsertColumn<int>(0, {1000, 2000, 3000, 4000, 5000}, "n");
    const int iterations = 2;
    for (int k = 0; k < iterations; k++) {
        vector<long> times;
        vector<Data> polygons;
        for (int j = 0; j < 5000; j++) {
            auto polygon = generatePolygon(512, 512);
            polygons.push_back(polygon);
            tree.insert(polygon);
        }
        for (int i = 0; i < 5; i++) {
            auto t1 = chrono::high_resolution_clock::now();
            for (int j = 0; j < 1000; j++) {
                auto index = random(rng) % polygons.size();
                // TODO algo esta fallando en el remove
                tree.remove(polygons[index]);
                // Problema: se cuenta para el tiempo
                polygons.erase(polygons.begin() + index);
            }
            auto t2 = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(t2-t1).count();
            times.push_back(duration);
        }
        doc.InsertColumn(k+1, times, "times" + to_string(k+1));
        tree.clear();
        times.clear();
    }
    doc.Save("../output/remove.csv");
}
