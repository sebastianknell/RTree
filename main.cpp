#include "RTree.h"
#include "lib/testing.h"

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
                drawing = true;
                currentDrawing.emplace_back(x, y);
                cv::circle(img, {x, y}, radius, {0, 0, 0}, -1);
            }
            else {
                cout << x << ", " << y << endl;
                rtree.insert({{x, y}});
                img.setTo(cv::Scalar(255, 255, 255));
                rtree.show(img);
            }
        }
        else {
            cv::line(img, currentDrawing.back(), {x, y}, colors[0], 2);
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
            //auto nns = rtree.depthFirst({x, y}, 3);
            auto nns = rtree.knn({x, y}, 3);
            for (auto nn : nns) {
                cv::line(img, {x, y}, nn.p, {0, 0, 0}, 1);
                if (nn.node->data[nn.index]->size() == 1)
                    cv::circle(img, nn.node->data[nn.index]->back(), radius, colors[5], -1);
                else cv::polylines(img, *nn.node->data[nn.index], true, colors[5], 2);
            }
            cv::imshow(windowName, img);
        }
    }
}

int main() {
    testInsert(rtree);
    /*rtree.show(img);
    cv::imshow(windowName, img);
    cv::waitKey(1);
    cv::setMouseCallback(windowName, clickHandler);

    char c;
    do {
        c = (char)cv::waitKey(0);
    } while (c != 'q');*/

    return 0;
}
