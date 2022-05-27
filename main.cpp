#include <iostream>
#include <opencv2/opencv.hpp>
#include "RTree.h"

using namespace std;

const int width = 720;
const string windowName = "RTree Visualization";
cv::Mat img(width, width, CV_8UC3, {255, 255, 255});
bool drawing = false;
vector<Point> currentDrawing;

static void clickHandler(int event, int x, int y, int flags, void*) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        if (!drawing) {
            if (flags == (cv::EVENT_FLAG_ALTKEY + cv::EVENT_FLAG_LBUTTON)) {
                cv::circle(img, {x, y}, radius, {0,0,0}, -1);
            }
            else {
                drawing = true;
                currentDrawing.push_back({x, y});
            }
        }
        else {
            auto last = currentDrawing.back();
            cv::line(img, {last.x, last.y}, {x, y}, {0 , 0, 0}, 2);
            if (isInCircle({x, y}, currentDrawing.front(), radius)) {
                for (auto &p : currentDrawing) cout << p.x << ", " << p.y << " | ";
                cout << endl;
                currentDrawing.clear();
                drawing = false;
            }
            else currentDrawing.push_back({x, y});
        }
        cv::imshow(windowName, img);
    }
}

int main() {
    cv::imshow(windowName, img);
    cv::setWindowProperty(windowName, cv::WND_PROP_TOPMOST, 1);
    cv::setMouseCallback(windowName, clickHandler);

    char c;
    do {
        c = (char)cv::waitKey(0);
    } while (c != 'q');

    return 0;
}
