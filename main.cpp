#include "RTree.h"

using namespace std;

const int height = 720;
const int width = 1080;
const string windowName = "RTree Visualization";
cv::Mat img(height, width, CV_8UC3, {255, 255, 255});
bool drawing = false;
vector<Point> currentDrawing;
RTree rtree;

static void clickHandler(int event, int x, int y, int flags, void*) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        if (!drawing) {
            if (flags == (cv::EVENT_FLAG_ALTKEY + cv::EVENT_FLAG_LBUTTON)) {
                cout << x << ", " << y << endl;
                rtree.insert({{x, y}});
                img.setTo(cv::Scalar(255, 255, 255));
                rtree.show(img);
            }
            else {
                drawing = true;
                currentDrawing.emplace_back(x, y);
                cv::circle(img, {x, y}, radius, {0, 0, 0}, -1);
            }
        }
        else {
            cv::line(img, currentDrawing.back(), {x, y}, colors[3], 2);
            cv::circle(img, {x, y}, radius, {0, 0, 0}, -1);
            if (isInCircle({x, y}, currentDrawing.front(), radius)) {
                for (auto &p : currentDrawing) cout << p.x << ", " << p.y << " | ";
                cout << endl;
                rtree.insert(currentDrawing);
                img.setTo(cv::Scalar(255,255,255));
                rtree.show(img);
                currentDrawing.clear();
                drawing = false;
            }
            else currentDrawing.emplace_back(x, y);
        }
        cv::imshow(windowName, img);
    }
    else if (event == cv::EVENT_RBUTTONDOWN) {
        cout << x << ", " << y << endl;
        rtree.remove({{x, y}});
        img.setTo(cv::Scalar(255, 255, 255));
        rtree.show(img);
        cv::imshow(windowName, img);
    }
    else if (event == cv::EVENT_MOUSEMOVE) {
        if (!drawing && !rtree.isEmpty()) {
            img.setTo(cv::Scalar(255, 255, 255));
            rtree.show(img);
//            auto nns = rtree.depthFirst({x, y}, 2);
            auto nns = rtree.knn({x, y}, 2);
            for (auto nn : nns) {
                cv::line(img, {x, y}, nn.p, {0, 0, 0}, 1);
                if (nn.node->data[nn.index]->size() == 1)
                    cv::circle(img, nn.node->data[nn.index]->back(), radius, colors[0], -1);
                else cv::polylines(img, *nn.node->data[nn.index], true, colors[0], 2);
            }
            cv::imshow(windowName, img);
        }
    }
}

int main() {
//    rtree.insert({{184, 76}});
//    rtree.insert({{776, 138}});
//    rtree.insert({{582, 354}});
//    rtree.insert({{228, 229}});
//    rtree.insert({{281, 316}});
//    rtree.insert({{177, 370}});
//    rtree.insert({{100, 269}});
//    rtree.insert({{324, 164}});
//    rtree.insert({{671, 254}});
//    rtree.insert({{200, 435}});
//    rtree.useCircles();
//    rtree.show(img);
    cv::imshow(windowName, img);
    cv::waitKey(1);

    cv::setMouseCallback(windowName, clickHandler);

    char c;
    do {
        c = (char)cv::waitKey(0);
    } while (c != 'q');

    return 0;
}
