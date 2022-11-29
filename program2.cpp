// Yen Duyen Amy Le
// CSE 4310

#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

#define NUM_COMMAND_LINE_ARGUMENTS 1
#define DISPLAY_WINDOW_NAME_INPUT "Program #2 Input"
#define DISPLAY_WINDOW_NAME_OUTPUT "Program #2 Output"
#define QUARTER_SIZE 95
#define NICKEL_SIZE 83
#define PENNY_SIZE 75
#define QUARTER_VALUE 0.25
#define NICKEL_VALUE 0.05
#define PENNY_VALUE 0.01
#define DIME_VALUE 0.10

// validate and parse the command line arguments
int main(int argc, char ** argv)
{
    cv::Mat imageIn;
    cv::Mat imageColor;

    // validate and parse the command line arguments
    if(argc != NUM_COMMAND_LINE_ARGUMENTS + 1)
    {
        std::printf("USAGE: %s <image_path> \n", argv[0]);
        return 0;
    }
    else
    {
        imageIn = cv::imread(argv[1], cv::IMREAD_COLOR);
        imageColor = cv::imread(argv[1], cv::IMREAD_COLOR);

        // check for file error
        if(!imageIn.data)
        {
            std::cout << "Error while opening file " << argv[1] << std::endl;
            return 0;
        }
    }

    // convert the image to grayscale
    cv::Mat imageGray;
    cv::cvtColor(imageIn, imageGray, cv::COLOR_BGR2GRAY);

    // normalize the image
    cv::Mat imageNormalized;
    cv::normalize(imageGray, imageNormalized, 0, 255, cv::NORM_MINMAX, CV_8UC1);

    // find the image edges
    cv::Mat imageEdges;
    const double cannyThreshold1 = 100;
    const double cannyThreshold2 = 200;
    const int cannyAperture = 3;
    cv::Canny(imageGray, imageEdges, cannyThreshold1, cannyThreshold2, cannyAperture);

    // erode and dilate the edges to remove noise
    int morphologySize = 1;
    cv::Mat edgesDilated;
    cv::dilate(imageEdges, edgesDilated, cv::Mat(), cv::Point(-1, -1), morphologySize);
    cv::Mat edgesEroded;
    cv::erode(edgesDilated, edgesEroded, cv::Mat(), cv::Point(-1, -1), morphologySize);

    // locate the image contours (after applying a threshold or canny)
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(edgesEroded, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    // fit ellipses to contours containing sufficient inliers
    std::vector<cv::RotatedRect> fittedEllipses(contours.size());
    for(int i = 0; i < contours.size(); i++)
    {
        // compute an ellipse only if the contour has more than 5 points (the minimum for ellipse fitting)
        if(contours.at(i).size() > 5)
        {
            fittedEllipses[i] = cv::fitEllipse(contours[i]);
        }
    }

    // draw the ellipses
    cv::Mat imageEllipse = cv::Mat::zeros(imageEdges.size(), CV_8UC3);
    const int minEllipseInliers = 50;
    
    // maintain count for each coin and total monetary value
    int numQuarters = 0;
    int numNickels = 0;
    int numPennies = 0;
    int numDimes = 0;
    float total = 0;

    for(int i = 0; i < contours.size(); i++)
    {
        // draw any ellipse with sufficient inliers
        if(contours.at(i).size() > minEllipseInliers)
        {
            int R = 255;
            int G = 255;
            int B = 255;

            // determine type of coin based on size of sufficient ellipse
            // coin is a quarter
            if(fittedEllipses[i].size.width > QUARTER_SIZE)
            {
                // quarter ellipses are green
                R = 0;
                G = 255;
                B = 0;
                total += QUARTER_VALUE;
                numQuarters += 1;
            }
            // coin is a nickel
            else if(fittedEllipses[i].size.width > NICKEL_SIZE)
            {
                // nickel ellipses are yellow
                R = 255;
                G = 255;
                B = 0;
                total += NICKEL_VALUE;
                numNickels += 1;
            }
            // coin is a penny
            else if(fittedEllipses[i].size.width > PENNY_SIZE)
            {
                // penny ellipses are red
                R = 255;
                G = 0;
                B = 0;
                total += PENNY_VALUE;
                numPennies += 1;
            }
            // coin is a dime
            else
            {
                // dime ellipses are blue
                R = 0;
                G = 0;
                B = 255;
                total += DIME_VALUE;
                numDimes += 1;
            }
            cv::Scalar color = cv::Scalar(B, G, R);
            cv::ellipse(imageColor, fittedEllipses[i], color, 2);
        }
    }

    // display the count for each coin and the total monetary value
    std::printf("Penny - %d\n", numPennies);
    std::printf("Nickel - %d\n", numNickels);
    std::printf("Dime - %d\n", numDimes);
    std::printf("Quarter - %d\n", numQuarters);
    std::printf("Total - $%0.2f\n", total);

    // display the images
    cv::imshow(DISPLAY_WINDOW_NAME_INPUT, imageIn);
    cv::imshow(DISPLAY_WINDOW_NAME_OUTPUT, imageColor);
    cv::waitKey();

    return 0;
}
