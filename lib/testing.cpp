//
// Created by Sebastian Knell on 9/07/22.
//

#include "testing.h"

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

void testInsert(Tree &tree, const string& path) {
    rapidcsv::Document doc;
    doc.SetColumnName(0, "n");
    doc.SetColumnName(1, "time");
    vector<long> times(500, 0);
    const int iterations = 100;
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
        polygons.clear();
        for (int i = 0; i < 5000; i++){
            polygons.push_back(generatePolygon(512, 512));
        }
    }
    for (int i = 0; i < times.size(); i++) {
        doc.InsertRow<double>(i, {(double)(i+1)*10, (double)times[i] / iterations});
    }
    doc.Save(path);
}

void testSearch(Tree &tree, const string& path) {
    rapidcsv::Document doc;
    doc.InsertColumn<int>(0, {1000, 2000, 3000, 4000, 5000}, "n");
    const int iterations = 100;
    for (int k = 0; k < iterations; k++) {
        vector<long> times;
        for (int i = 0; i < 5; i++) {
            vector<Data> polygons;
            for (int j =0; j < 1000; j++) {
                auto polygon = generatePolygon(512, 512);
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
    doc.Save(path);
}

void testRemove(Tree &tree, const string& path) {
    rapidcsv::Document doc;
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> random(0,5000);
    doc.InsertColumn<int>(0, {1000, 2000, 3000, 4000, 5000}, "n");
    const int iterations = 100;
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
    doc.Save(path);
}

void testKnn(Tree &tree, const string &path) {
    rapidcsv::Document doc;
    doc.InsertColumn<int>(0, {1000, 2000, 3000, 4000, 5000}, "n");
    const int iterations = 100;
    int k = 1;
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> randomPoint(0,512);
    for (int iter = 0; iter < iterations; iter++) {
        vector<long> times;
        for (int i = 0; i < 5; i++) {
            vector<Data> polygons;
            for (int j =0; j < 1000; j++) {
                auto polygon = generatePolygon(512, 512);
                polygons.push_back(polygon);
                tree.insert(polygon);
            }
            Point p = {(int)randomPoint(rng), (int)randomPoint(rng)};
            auto t1 = chrono::high_resolution_clock::now();
            tree.callKnn(p, k);
            auto t2 = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::microseconds>(t2-t1).count();
            times.push_back(duration);
        }
        doc.InsertColumn(iter+1, times, "times" + to_string(iter+1));
        tree.clear();
    }
    doc.Save(path);
}

void compareOverlap(const string &path) {
    rapidcsv::Document overlaps;
    overlaps.SetColumnName(0, "divisiones");
    overlaps.SetColumnName(1, "rtree_leaf");
    overlaps.SetColumnName(2, "htree_leaf");
    overlaps.SetColumnName(3, "rtree_internal");
    overlaps.SetColumnName(4, "htree_internal");
    overlaps.SetColumnName(5, "rtree_all");
    overlaps.SetColumnName(6, "htree_all");
    RTree rtree;
    HilbertRtree htree(512, 512);
    int levels = 3;
    const int maxLevels = 12;
    vector<double> htreeSampleLeaf;
    vector<double> htreeSampleInternal;
    vector<double> htreeSampleAll;
    while (levels < maxLevels) {
        double rtreeOverlapLeaf = 0.0;
        double htreeOverlapLeaf = 0.0;
        double rtreeOverlapInternal = 0.0;
        double htreeOverlapInternal = 0.0;
        double rtreeOverlapAll = 0.0;
        double htreeOverlapAll = 0.0;
        for (int iter = 0; iter < 1; iter++) {
            for (int j =0; j < 5000; j++) {
                auto polygon = generatePolygon(512, 512);
                rtree.insert(polygon);
                htree.insert(polygon);
            }
            auto rtreeOverlapsLeaf = rtree.getLeafsOverlap();
            auto htreeOverlapsLeaf = htree.getLeafsOverlap();
            auto rtreeOverlapsInternal = rtree.getLeafsOverlap();
            auto htreeOverlapsInternal = htree.getLeafsOverlap();

            auto rtreeOverlapsAll = rtreeOverlapsLeaf;
            rtreeOverlapsAll.insert(rtreeOverlapsAll.end(), rtreeOverlapsInternal.begin(), rtreeOverlapsInternal.end());
            auto htreeOverlapsAll = htreeOverlapsLeaf;
            htreeOverlapsAll.insert(htreeOverlapsAll.end(), htreeOverlapsInternal.begin(), htreeOverlapsInternal.end());

            sort(rtreeOverlapsLeaf.begin(), rtreeOverlapsLeaf.end());
            sort(htreeOverlapsLeaf.begin(), htreeOverlapsLeaf.end());
            sort(rtreeOverlapsInternal.begin(), rtreeOverlapsInternal.end());
            sort(htreeOverlapsInternal.begin(), htreeOverlapsInternal.end());
            sort(rtreeOverlapsAll.begin(), rtreeOverlapsAll.end());
            sort(htreeOverlapsAll.begin(), htreeOverlapsAll.end());

            rtreeOverlapLeaf += rtreeOverlapsLeaf[(rtreeOverlapsLeaf.size()-1)/2];
            htreeOverlapLeaf += htreeOverlapsLeaf[(htreeOverlapsLeaf.size()-1)/2];
            rtreeOverlapInternal += rtreeOverlapsInternal[(rtreeOverlapsInternal.size()-1)/2];
            htreeOverlapInternal += htreeOverlapsInternal[(htreeOverlapsInternal.size()-1)/2];
            rtreeOverlapAll += rtreeOverlapsAll[(rtreeOverlapsAll.size()-1)/2];
            htreeOverlapAll += htreeOverlapsAll[(htreeOverlapsAll.size()-1)/2];

            if (levels == 9) {
                htreeSampleLeaf.insert(htreeSampleLeaf.end(), htreeOverlapsLeaf.begin(), htreeOverlapsLeaf.end());
                htreeSampleInternal.insert(htreeSampleInternal.end(), htreeOverlapsInternal.begin(), htreeOverlapsInternal.end());
                htreeSampleAll.insert(htreeSampleAll.end(), htreeOverlapsAll.begin(), htreeOverlapsAll.end());
            }
            rtree.clear();
            htree.clear();
        }
        overlaps.InsertRow<double>(levels-3, {(double)levels, rtreeOverlapLeaf/100, htreeOverlapLeaf/100, rtreeOverlapInternal/100, htreeOverlapInternal/100, rtreeOverlapAll/100, htreeOverlapAll/100});
        levels++;
        htree.setLevels(levels);
    }
    overlaps.Save(path);
    rapidcsv::Document overlapSample;
    overlapSample.InsertColumn(0, htreeSampleLeaf, "overlap_leaf");
    overlapSample.InsertColumn(1, htreeSampleInternal, "overlap_internal");
    overlapSample.InsertColumn(2, htreeSampleAll, "overlap_all");
    overlapSample.Save("../overlap_sample_htree");
}
