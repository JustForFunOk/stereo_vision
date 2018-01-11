//回调函数
//http://wiki.ros.org/roscpp/Overview/Callbacks%20and%20Spinning
//这个函数中的执行频率和消息池需重新考虑
//http://www.cnblogs.com/liu-fa/p/5925381.html
#include <ros/ros.h>
#include <geometry_msgs/Point.h> //点 消息类型
//#include <message_filters/subscriber.h>
//#include <message_filters/time_synchronizer.h> //时间调整
#include <iostream>
#include <stdio.h>

using namespace std;

ros::Publisher target3DPositionPub;
geometry_msgs::Point left2DPoint, right2DPoint, target3DPosition;
/*
void Cal3DPosCallback(geometry_msgs::Point& leftPoint, geometry_msgs::Point& rightPoint)
{
  geometry_msgs::Point target3DPosition;
  
  target3DPosition.x = leftPoint.x + rightPoint.x;
  target3DPosition.y = leftPoint.y + rightPoint.y;
  target3DPosition.z = leftPoint.z + rightPoint.z;
  
  target3DPositionPub.publish(target3DPosition);
}
*/
void leftPointCallback(const geometry_msgs::Point& leftPoint) //形参到底怎么写？::::
{
  left2DPoint = leftPoint;	
}

void rightPointCallback(const geometry_msgs::Point& rightPoint)
{
  right2DPoint = rightPoint;	
}

void CalTargetPos(geometry_msgs::Point& leftPoint, geometry_msgs::Point& rightPoint)
{
  target3DPosition.x = leftPoint.x + rightPoint.x;
  target3DPosition.y = leftPoint.y + rightPoint.y;
  target3DPosition.z = leftPoint.z + rightPoint.z;
//  cout<< target3DPosition.x << "," << target3DPosition.y <<endl;	
  target3DPositionPub.publish(target3DPosition);
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "cal_target_3Dposition"); //节点名称：cal_target_3Dposition
  ros::NodeHandle nh; //创建与master通信的节点
  ros::Rate r(5); //执行频率10Hz
  target3DPositionPub = nh.advertise<geometry_msgs::Point>("target_3Dposition", 1); //发布话题：target_3Dposition
  ros::Subscriber left2DPositionSub = nh.subscribe("/left_2Dposition", 1, leftPointCallback); //订阅左图像话题
  ros::Subscriber right2DPositionSub = nh.subscribe("/right_2Dposition", 1, rightPointCallback); //订阅右图像话题
//  message_filters::Subscriber<geometry_msgs::Point> left2DPositionSub(nh, "/left_2Dposition", 1);
//  message_filters::Subscriber<geometry_msgs::Point> right2DPositionSub(nh, "/right_2Dposition", 1);
//  TimeSynchronizer<geometry_msgs::Point, geometry_msgs::Point> sync(left2DPositionSub, right2DPositionSub, 1);
//  sync.registerCallback(boost::bind(&Cal3DPosCallback, _1, _2)); //放到一个回调函数中	
  while(ros::ok())
  {
    CalTargetPos(left2DPoint, right2DPoint);
    ros::spinOnce(); //不能忘！！缺少则无法调用回调函数
    r.sleep();
  }

  return 0;
}
