//
// Created by Sebastian Knell on 9/07/22.
//

#include "testing.h"

Data generatePolygon() {
    int width = 10;
    int height = 10;
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> random(width,1000);
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
    std::uniform_int_distribution<std::mt19937::result_type> random1(bb.x_low,width);
    p1 = {(int)random1(rng), bb.y_low};
    if (n == 2) {
        p2 = {(int)random1(rng), bb.y_low};
        polygon.push_back(p1.x < p2.x ? p1 : p2);
        polygon.push_back(p1.x < p2.x ? p2 : p1);
    }
    else polygon.push_back(p1);
    // derecha
    n = random(rng) % 2 + 1;
    std::uniform_int_distribution<std::mt19937::result_type> random2(bb.y_low,height);
    p1 = {bb.x_high, (int)random2(rng)};
    if (n == 2) {
        p2 = {bb.x_high, (int)random2(rng)};
        polygon.push_back(p1.x < p2.x ? p1 : p2);
        polygon.push_back(p1.x < p2.x ? p2 : p1);
    }
    else polygon.push_back(p1);
    // abajo
    p1 = {(int)random1(rng), bb.y_high};
    if (n == 2) {
        p2 = {(int)random1(rng), bb.y_high};
        polygon.push_back(p1.x < p2.x ? p1 : p2);
        polygon.push_back(p1.x < p2.x ? p2 : p1);
    }
    else polygon.push_back(p1);
    // izquierda
    p1 = {bb.x_low, (int)random2(rng)};
    if (n == 2) {
        p2 = {bb.x_low, (int)random2(rng)};
        polygon.push_back(p1.x < p2.x ? p1 : p2);
        polygon.push_back(p1.x < p2.x ? p2 : p1);
    }
    else polygon.push_back(p1);
    return polygon;
}

void testInsert(Tree &tree) {
    rapidcsv::Document doc;
    doc.SetColumnName(0, "n");
    doc.SetColumnName(1, "time");
    vector<double> times;
    for (int i = 0; i < 500; i++) {
        auto t1 = chrono::high_resolution_clock::now();
        for (int j =0; j < 10; j++) {
            auto polygon = generatePolygon();
            tree.insert(polygon);
        }
        auto t2 = chrono::high_resolution_clock::now();
        long duration = chrono::duration_cast<chrono::microseconds>(t2-t1).count();
        vector<long> row = {(i+1)*10, duration};
        doc.InsertRow(i, row);
    }
    doc.Save("../output/insert.csv");
}
