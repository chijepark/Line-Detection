#include "ros/ros.h"
#include "cv_test/MsgControl.h"
void msgCallback(const cv_test::MsgControl::ConstPtr& msg)
{
    ROS_INFO("recieve msg = %d",msg->data);
}
int main(int argc,char** argv)
{
    ros::init(argc,argv,"listener");
    ros::NodeHandle nh;
    ros::Subscriber ros_tutorial_sub = nh.subscribe("ros_msg",100,msgCallback);
    ros::spin();
    return 0;
}
