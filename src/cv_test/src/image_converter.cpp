#include "ros/ros.h"
#include "cv_test/MsgControl.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <cmath>
using namespace cv;
Mat myHough(Mat houghImage, Mat origin);
Mat myCanny(Mat &frame);
Mat getROI(Mat& origImag, Point2f top, Point2f botLeft, Point2f botRight);
Mat drawLines(Mat drawImage,vector<Vec4i> &lines);
void myransac(vector<int> x,vector<int> y,Point &pt1, Point &pt2,int imageRows);
void getControl(Point ptl1, Point ptl2, Point ptr1, Point ptr2);

double controlInfo;
double centerPoint;

int main(int argc,char **argv)
{
    //initialize ros && opencv
    ros::init(argc,argv,"image_converter");
    ros::NodeHandle nh;
    ros::Publisher talker = nh.advertise<cv_test::MsgControl>("ros_msg",100);
    cv_test::MsgControl msg;
    ros::Rate loop_rate(10);
    VideoCapture capture(0);
    if(!capture.isOpened())
    {
        std::cout<<"error!";
        return 0;
    }
    Size size= Size((int)capture.get(CV_CAP_PROP_FRAME_WIDTH),(int)capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    std::cout<<"size : "<<size<<std::endl;

    //make out frame using window
    namedWindow("roiImage", WINDOW_AUTOSIZE);
    namedWindow("houghImage", WINDOW_AUTOSIZE);

    //make variable
    int delay=30;
    Mat frame,roiImage , edgeImage, houghImage;

    //go main loop
    while(true)
    {
        capture >>frame;
        if(!frame.empty())
        {
            centerPoint = frame.cols/2;
            edgeImage = myCanny(frame);
            roiImage = getROI(edgeImage,Point2f(frame.cols/2,frame.rows/2),Point2f(0,frame.rows),Point2f(frame.cols,frame.rows));
            imshow("roiImage",roiImage);
            houghImage = myHough(roiImage,frame);
            imshow("houghImage",houghImage);
            msg.data = controlInfo;
            ROS_INFO("send msg = %d",msg.data);
            talker.publish(msg);
        }
        int ckey = waitKey(delay);
        if(ckey == 27)
            break;
    }
}
//ROI
Mat getROI(Mat& origImag, Point2f top, Point2f botLeft, Point2f botRight){
    Mat black(origImag.rows, origImag.cols, origImag.type(), cv::Scalar::all(0));
    Mat mask(origImag.rows, origImag.cols, CV_8UC1, cv::Scalar(0));
    vector< vector<Point> >  co_ordinates;
    co_ordinates.push_back(vector<Point>());
    co_ordinates[0].push_back(top);
    co_ordinates[0].push_back(botLeft);
    co_ordinates[0].push_back(botRight);
    drawContours( mask,co_ordinates,0, Scalar(255),CV_FILLED, 8 );

    origImag.copyTo(black,mask);
    return black;
}

//get outline using canny
Mat myCanny(Mat &frame)
{
    Mat grayImage, edgeImage;
    cvtColor(frame,grayImage,COLOR_BGR2GRAY); // convert color image
    Canny(grayImage,edgeImage,125,250,3); // using canny get outline
    return edgeImage;
}
//hough transform
Mat myHough(Mat edgeImage, Mat houghImage)
{
    vector<Vec4i> lines; // get line information in vector
    Vec4i params;
    int x1,y1,x2,y2;
    HoughLinesP(edgeImage,lines, 1, CV_PI/180.0, 30, 40, 25);// get lines using hough transform
    houghImage = drawLines(houghImage, lines);
    return houghImage;
}
Mat drawLines(Mat drawImage,vector<Vec4i> &lines)
{
    if(lines.empty())
        return drawImage;
    Mat line_img(drawImage.rows,drawImage.cols,CV_8UC3, cv::Scalar(0));
    Vec4i params;
    vector<int> left_line_x;
    vector<int> left_line_y;
    vector<int> right_line_x;
    vector<int> right_line_y;
    int x1,y1,x2,y2;
    double slope;

    int lineLen = lines.size();
    for(int k=0;k<lineLen;k++)
    {
        //grouping!!
        params = lines[k]; // straght line information insert param
        x1 = params[0];
        y1 = params[1];
        x2 = params[2];
        y2 = params[3];
        slope = ((double)y2 - y1)/(x2-x1); //get slope
        if(fabs(slope)<0.5)
            continue;
        else if(slope<0)    // leftLine
        {
            left_line_x.push_back(x1);
            left_line_x.push_back(x2);
            left_line_y.push_back(y1);
            left_line_y.push_back(y2);
        }
        else    //rightLine
        {
            right_line_x.push_back(x1);
            right_line_x.push_back(x2);
            right_line_y.push_back(y1);
            right_line_y.push_back(y2);
        }
    }
    //test code
    /*
    std::cout<<"left_line_x :\n";
    for(int i=0;i<left_line_x.size();i++)
        std::cout<<left_line_x[i]<<' ';
    std::cout<<'\n';
    std::cout<<"left_line_y :\n";
    for(int i=0;i<left_line_y.size();i++)
        std::cout<<left_line_y[i]<<' ';
    std::cout<<'\n';
    std::cout<<"right_line_x :\n";
    for(int i=0;i<right_line_x.size();i++)
        std::cout<<right_line_x[i]<<' ';
    std::cout<<'\n';
    std::cout<<"right_line_y :\n";
    for(int i=0;i<right_line_y.size();i++)
        std::cout<<right_line_y[i]<<' ';
    std::cout<<'\n';
    */
    //until here
     //RANSAC
    Point ptl1, ptl2, ptr1, ptr2;
    myransac(left_line_x,left_line_y,ptl1, ptl2, drawImage.rows);
    myransac(right_line_x,right_line_y,ptr1, ptr2, drawImage.rows);
    //draw line
    line(line_img, ptl1, ptl2, Scalar(0, 0, 255),3); // draw line in raw image
    line(line_img, ptr1, ptr2, Scalar(0, 0, 255),3); // draw line in raw image
    addWeighted(drawImage, 0.8, line_img, 1.0, 0.0, drawImage);
    getControl(ptl1, ptl2, ptr1, ptr2);
    return drawImage;
}
void getControl(Point ptl1, Point ptl2, Point ptr1, Point ptr2)
{
    int mid1 = (ptl1.x + ptr1.x)/2;
    int mid2 = (ptl2.x + ptr2.x)/2;
    controlInfo = fabs(centerPoint-(mid1 + mid2)/2);
}
void myransac(vector<int> x,vector<int> y,Point &pt1, Point &pt2,int imageRows)
{
    double gradient, distance, yIC;
    double resultn = log(1-0.99)/log(1-(0.5*0.5));//store n
    int r1,r2;
    int dataNum = x.size();
    int threshold = 20;
    int inlier = 0, inMax = 0;
    int x1,y1,x2,y2;
    for(int i=0;i<resultn;i++)
    {
        r1 = rand()%(dataNum);
        r2 = ((rand()%(dataNum))+r1)%(dataNum);
        if(r1 == r2 || x[r1] == x[r2])
        {
            i--;
            continue;
        }

        gradient = ((double)(y[r2]-y[r1]))/(x[r2]-x[r1]);
        yIC = y[r1] - (gradient * x[r1]);
        for(int k=0;k<dataNum;k++)
        {
            if(k!=r1 && k!=r2)
            {
                distance = abs(-(x[k]*gradient) - yIC + y[k])/sqrt((gradient*gradient)+1);
                if(distance < threshold)
                {
                    inlier++;
                }
            }
        }
        if(inlier>=inMax)
        {
            inMax = inlier;
            int y1 = imageRows;
            int x1 = (int)((y1-yIC)/gradient);
            int y2 = (imageRows*3)/4;
            int x2 = (int)((y2-yIC)/gradient);
            pt1 = Point(x1, y1);
            pt2 = Point(x2, y2);
            //test code
            std::cout << "x1,y1 : \n" <<pt1<<std::endl;
            std::cout << "x2, y2 : \n" <<pt2<<std::endl;
            //until here
        }
        inlier = 0;
    }
}
