#include <opencv2/aruco/charuco.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <string>
#include "aruco_samples_utility.hpp"

#include <ros/ros.h>

namespace {
const char* about = "A tutorial code on charuco board creation and detection of charuco board with and without camera caliberation";
const char* keys = "{c        |       | Put value of c=1 to create charuco board;\nc=2 to detect charuco board without camera calibration;\nc=3 to detect charuco board with camera calibration and Pose Estimation}";
}

static inline void createBoard()
{
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
    cv::Ptr<cv::aruco::CharucoBoard> board = cv::aruco::CharucoBoard::create(5, 7, 0.04f, 0.02f, dictionary);
    cv::Mat boardImage;
    board->draw(cv::Size(600, 500), boardImage, 10, 1);
    cv::imwrite("BoardImage.jpg", boardImage);
}

int main(int argc, char** argv)
{
    ros::init (argc, argv, "charuco");
    ros::NodeHandle nh;

    createBoard();
    std::cout << "An image named BoardImg.jpg is generated in folder containing this file" << std::endl;
    return 0;
}