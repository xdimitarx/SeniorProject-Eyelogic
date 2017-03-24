#ifndef Win_h
#define Win_h
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


class Win : public System {
public:
    static void setCurPos(int x, int y) override {

        // Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos(x, y);
        cout << x << "    " << y << endl;

    }

    void setCurPos(cv::Point cv::Point) {
        //Windows way to set the cursor position to some x,y coordinate on the screen
        SetCursorPos(cv::Point.x, cv::Point.y);
        cout << cv::Point.x << "    " << cv::Point.y << endl;
    }

    cv::Point getCurPos() {
        //Windows way to get the cursor position
        cv::Point windowscv::Point;
        GetCursorPos(&windowscv::Point);
        cv::Point curr((int)windowscv::Point.x, (int)windowscv::Point.y);

        return curr;
    }

}
#endif /* WinInclude_h */
