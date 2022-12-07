#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>

using namespace std;

#define FINDSHAPES		0
#define CORNERSUBPIX	1
uint8_t image_filter = FINDSHAPES;

#define CAMERA			2
#define THREESQUARES	3
#define FOURSQUARES		4
#define THREESQUARESCENTER	5
uint8_t image_source = CAMERA;

bool image_print = false;

string img_loc_3 = "C:/Users/ajroh/repos/opencv_new/module15/3squares.png";
string img_loc_4 = "C:/Users/ajroh/repos/opencv_new/module15/4squares.png";
string img_loc_3_centered = "C:/Users/ajroh/repos/opencv_new/module15/3squarescentered.png";

string win_cam = "Camera Detection";
string win_thresh = "Threshold View";
string win_draw = "Object Drawing";

int thresh = 50, N = 11;

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
}

// returns sequence of squares detected on the image.
static void findSquares(const cv::Mat& image, vector<vector<cv::Point> >& squares)
{
    squares.clear();

    cv::Mat pyr, timg, gray0(image.size(), CV_8U), gray;

    // down-scale and upscale the image to filter out the noise
    pyrDown(image, pyr, cv::Size(image.cols / 2, image.rows / 2));
    pyrUp(pyr, timg, image.size());
    vector<vector<cv::Point> > contours;

    // find squares in every color plane of the image
   /* for (int c = 0; c < 3; c++)
    {*/
    int c = 0;
        int ch[] = { c, 0 };
        mixChannels(&timg, 1, &gray0, 1, ch, 1);

        // try several threshold levels
        for (int l = 0; l < N; l++)
        {
            // hack: use Canny instead of zero threshold level.
            // Canny helps to catch squares with gradient shading
            if (l == 0)
            {
                // apply Canny. Take the upper threshold from slider
                // and set the lower to 0 (which forces edges merging)
                Canny(gray0, gray, 0, thresh, 5);
                // dilate canny output to remove potential
                // holes between edge segments
                dilate(gray, gray, cv::Mat(), cv::Point(-1, -1));
            }
            else
            {
                // apply threshold if l!=0:
                //     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
                gray = gray0 >= (l + 1) * 255 / N;
            }

            // find contours and store them all as a list
            findContours(gray, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

            vector<cv::Point> approx;

            // test each contour
            for (size_t i = 0; i < contours.size(); i++)
            {
                // approximate contour with accuracy proportional
                // to the contour perimeter
                approxPolyDP(contours[i], approx, arcLength(contours[i], true) * 0.02, true);

                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation
                if (approx.size() == 4 &&
                    fabs(contourArea(approx)) > 1000 &&
                    isContourConvex(approx))
                {
                    double maxCosine = 0;

                    for (int j = 2; j < 5; j++)
                    {
                        // find the maximum cosine of the angle between joint edges
                        double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
                        maxCosine = MAX(maxCosine, cosine);
                    }

                    // if cosines of all angles are small
                    // (all angles are ~90 degree) then write quandrange
                    // vertices to resultant sequence
                    if (maxCosine < 0.3)
                        squares.push_back(approx);
                }
            }
        }
    //}
}


int main() {
	cout << "Hello World!\n";
	cout << "OpenCV Version is " << CV_VERSION << endl;

	cv::namedWindow(win_cam, cv::WINDOW_NORMAL);
	cv::namedWindow(win_thresh, cv::WINDOW_NORMAL);
	cv::namedWindow(win_draw, cv::WINDOW_NORMAL);

	cv::VideoCapture source = cv::VideoCapture(0);

	uint shape_cnt = 0;
	uint *shape_cnt_history = (uint*)calloc(100, sizeof(uint));
	uint *shape_cnt_history_2nd_lvl = (uint*)calloc(100, sizeof(uint));

	cv::Mat frame, frame_draw, frame_blur, frame_hsv, frame_thresh;

	while (1) {
		switch (image_source) {
			case CAMERA:
				bool ok = source.read(frame);
				if (!ok) {
					continue;
				}
				cv::flip(frame, frame, 1);
				break;
			/*case THREESQUARES:

				break;
			case FOURSQUARES:

				break;
			case THREESQUARESCENTER:

				break;*/
		}

		try {
			switch (image_filter) {
				case FINDSHAPES:
                    vector<vector<cv::Point>> squares;
                    cv::blur(frame, frame_blur, cv::Size(13, 13));
                    cv::cvtColor(frame_blur, frame_hsv, cv::COLOR_BGR2HSV);
                    cv::inRange(frame_hsv, (105, 150, 80), (130, 255, 255), frame_thresh);
                    findSquares(frame_thresh, squares);
					break;
			}
		}
		catch (exception e) {
			cout << "Exception!" << endl;
		}

        cv::imshow(win_cam, frame);
        cv::imshow(win_draw, frame_blur);
        cv::imshow(win_thresh, frame_thresh);

        if ((cv::waitKey(1) & 0xFF) == (int)'q') break;
	} 

    cv::destroyAllWindows();
    source.release();

	return 0;
}