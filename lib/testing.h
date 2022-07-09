//
// Created by Sebastian Knell on 9/07/22.
//

#ifndef RTREE_TESTING_H
#define RTREE_TESTING_H

#include <chrono>
#include "utils.h"
#include "rapidcsv.h"

Data generatePolygon();
void testInsert(Tree&);
void testSearch(Tree&);
void testRemove(Tree&);

#endif //RTREE_TESTING_H
