#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void findSquares(std::vector<std::vector<Point>> &contours, std::vector<std::vector<Point>> &squares) {
    bool found = false;
    for (const auto& c : contours) {
        if (c.size() == 4) continue;
        else if (c.size() < 4) continue;
        else {
            // Loop through all points in a contour
            // Find lower x bound and upper x bound -> creates width
            // Find lower y bound and upper y bound -> creates height
            int x = NULL, y = NULL;
            int w = NULL, h = NULL;
            for (const auto& p : c) {
                // Points p.x p.y
                if (x == NULL && y == NULL) {
                    x = p.x;
                    y = p.y;
                    continue;
                }
                if (p.x < x) {
                    x = p.x;
                }
                if (p.y < y) {
                    y = p.y;
                }
                if (abs(p.x) - x > 15 && p.x - x > w) {
                    w = p.x - x;
                }
                if (abs(p.y) - y > 15 && p.y - y > h) {
                    h = p.y - y;
                }
            }
            if (x != NULL && y != NULL && w != NULL && h != NULL) {
                std::vector<Point> square = { {x,y},{x,y+h},{x+h,y+h},{x+h,y} };
                squares.push_back(square);
            }
        }
    }
}

bool inColorRange(Scalar color, char c) {
    switch (c) {
        case 'b':
            if (color.val[0] >= 105 && color.val[0] <= 130) {
                if (color.val[1] >= 150 && color.val[1] <= 255) {
                    if (color.val[2] >= 80 && color.val[2] <= 255) {
                        return true;
                    }
                }
            }
            break;
        case 'r':
            if (color.val[0] >= 105 && color.val[0] <= 130) {
                if (color.val[1] >= 150 && color.val[1] <= 255) {
                    if (color.val[2] >= 80 && color.val[2] <= 255) {
                        return true;
                    }
                }
            }
            break;
        case 'g':
            if (color.val[0] >= 105 && color.val[0] <= 130) {
                if (color.val[1] >= 150 && color.val[1] <= 255) {
                    if (color.val[2] >= 80 && color.val[2] <= 255) {
                        return true;
                    }
                }
            }
            break;
        case 'y':
            if (color.val[0] >= 105 && color.val[0] <= 130) {
                if (color.val[1] >= 150 && color.val[1] <= 255) {
                    if (color.val[2] >= 80 && color.val[2] <= 255) {
                        return true;
                    }
                }
            }
            break;
    }
    return false;
}

int main(int argc, char** argv) {
    // Read the image into memory
    //Mat image = imread("3squares.png");

    VideoCapture capture(0);

    if (!capture.isOpened()) {
        cerr << "ERROR: Could not open the video camera" << endl;
        return 1;
    }

    int frame_width = capture.get(CAP_PROP_FRAME_WIDTH);
    int frame_height = capture.get(CAP_PROP_FRAME_HEIGHT);

    namedWindow("Video", WINDOW_AUTOSIZE);
    namedWindow("Blue Only", WINDOW_AUTOSIZE);

    Mat frame;

    while (true) {
        capture >> frame;

        // Check if the frame was empty
        if (frame.empty())
        {
            cerr << "ERROR: Could not read a frame from the video camera" << endl;
            break;
        }

        // Convert the image to the HSV color space
        Mat hsv;
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        // Define the range of blue colors in the HSV color space
        Scalar lower_blue(105, 150, 80);
        Scalar upper_blue(130, 255, 255);

        // Threshold the image to get only blue colors
        Mat mask;
        inRange(hsv, lower_blue, upper_blue, mask);

        // Bitwise-AND the mask with the original image
        Mat blue_image;
        bitwise_and(frame, frame, blue_image, mask);

        // Convert the image to grayscale
        Mat gray;
        cvtColor(blue_image, gray, COLOR_BGR2GRAY);

        // Blur the image to reduce noise
        Mat blurred;
        GaussianBlur(gray, blurred, Size(3, 3), 0);

        // Use the Canny edge detection algorithm to find edges in the image
        Mat edges;
        Canny(blurred, edges, 100, 200);

        try {
            // Find contours in the image
            std::vector<std::vector<Point>> contours;
            findContours(edges, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
            std::vector<std::vector<Point>> squares;
            findSquares(contours, squares);

            // Loop through the contours and find the square
            for (const auto& c : squares) {
                // Check if the contour is a square
                if (c.size() == 4) {
                    // Draw the contour on the image
                    drawContours(frame, squares, -1, Scalar(0, 255, 0), 2);

                    // Calculate the average color of the square
                    // Calculate the average color of the square
                    Vec3b color = frame.at<Vec3b>(c[0].x + (c[2].x - c[0].x) / 2, c[0].y + (c[1].y - c[0].y) / 2);

                    // Print the color of the square
                    if (inColorRange(color, 'b')) cout << "BLUE : " << color << endl;
                }
            }
        } catch (exception e) {
            cerr << e.what() << endl;
        }

        // Show the resulting image
        imshow("Video", frame);
        imshow("Blue Only", blue_image);
        // Check if the user pressed the "Esc" key
        if (waitKey(1) == 27)
        {
            break;
        }
    }

    // Release the video capture object
    capture.release();

    // Close the window
    destroyAllWindows();

    return 0;
}