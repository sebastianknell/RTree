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
int getOverlap(Rect, Rect);
double getTotalOverlap(vector<Rect>&);
Data generatePolygon(int, int);
void testOverlap();
void testSearch(Tree&, const string&);
void testInsert(Tree&, const string&);
void testRemove(Tree&, const string&);

#endif //RTREE_TESTING_H
