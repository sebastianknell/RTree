//
// Created by Sebastian Knell on 9/07/22.
//

#ifndef RTREE_TESTING_H
#define RTREE_TESTING_H

#include <chrono>
#include <random>
#include "utils.h"
#include "rapidcsv.h"

bool isOverlapping(Rect, Rect);
double getOverlap(Rect, Rect);
double getTotalOverlap(vector<Rect>&);
Data generatePolygon(int, int);
void testOverlap();
void testSearch(Tree&);
void testInsert(Tree&);
void testRemove(Tree&);

#endif //RTREE_TESTING_H
