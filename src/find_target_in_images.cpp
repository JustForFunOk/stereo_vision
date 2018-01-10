//ROS和CV之间图像转换
//http://wiki.ros.org/cv_bridge/Tutorials/UsingCvBridgeToConvertBetweenROSImagesAndOpenCVImages
//message_filters
//http://wiki.ros.org/message_filters
//image_transport
//http://wiki.ros.org/image_transport
//Publisher and Subscriber 
//http://wiki.ros.org/ROS/Tutorials/WritingPublisherSubscriber%28c%2B%2B%29
#include <ros/ros.h>
#include <geometry_msgs/Point.h> //点 消息类型
#include <image_transport/image_transport.h> //图像传输
#include <sensor_msgs/Image.h> //图像 消息类型
#include <message_filters/subscriber.h>
#include <message_filters/time_synchronizer.h> //时间调整
#include <cv_bridge/cv_bridge.h> //ROS中的图像与Opencv中的图像之间的转换

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace message_filters;
using namespace sensor_msgs;
using namespace cv;
using namespace std;

//全局变量
cv::CascadeClassifier faceCascade;
const std::string face_cascade_name = "haarcascade_frontalface_alt.xml";
//检测
cv::Point ObjectDetect(Mat& img)
{
	std::vector<Rect> faces;
	Mat grayImg;
	cv::Point center(0, 0);
	cvtColor( img, grayImg, COLOR_BGR2GRAY ); //转换成灰度图
	equalizeHist( grayImg, grayImg ); //直方图均衡化，拉伸像素使其分布到0-255,增加对比度
	faceCascade.detectMultiScale( grayImg, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) ); //进行人脸检测
	if(faces.size() == 1) //检测到一个人脸，则返回人脸坐标
	{
		center.x = faces[0].x + faces[0].width/2;
		center.y = faces[0].y + faces[0].height/2;
		return center;
	}
	else
	{
		return center;
	}
		
	
}

void leftImageCallback(const ImageConstPtr& left)
{
	cv_bridge::CvImagePtr cv_left;
	try
    {
      cv_left = cv_bridge::toCvCopy(left);
    }
    catch (cv_bridge::Exception& e) //如果抛出这种异常，则打印如下错误信息，并退出
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }
    
    //使用opencv中的算法检测人脸
	cv::Point leftPoint = ObjectDetect(cv_left->image);
	cout<< "L:" << leftPoint.x << "," << leftPoint.y <<endl;
}

void rightImageCallback(const ImageConstPtr& right)
{
	cv_bridge::CvImagePtr cv_right;
	try
    {
      cv_right = cv_bridge::toCvCopy(right);
    }
    catch (cv_bridge::Exception& e) //如果抛出这种异常，则打印如下错误信息，并退出
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }
    
    //使用opencv中的算法检测人脸
    cv::Point rightPoint = ObjectDetect(cv_right->image);
	cout<< "R:" << rightPoint.x << "," << rightPoint.y <<endl;
}

/*
void ImgProcCallback(const ImageConstPtr& left, const ImageConstPtr& right)
{
	cv_bridge::CvImagePtr cv_left;
  try
    {
      cv_left = cv_bridge::toCvCopy(left);
    }
    catch (cv_bridge::Exception& e) //如果抛出这种异常，则打印如下错误信息，并退出
    {
      ROS_ERROR("cv_bridge exception: %s", e.what());
      return;
    }

  cv_bridge::CvImagePtr cv_right;
  try
      {
        cv_right = cv_bridge::toCvCopy(right);
      }
      catch (cv_bridge::Exception& e)
      {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
      }

	//使用opencv中的算法检测人脸
	cv::Point leftPoint = ObjectDetect(cv_left->image);
	cout<< "L:" << leftPoint.x << "," << leftPoint.y <<endl;
	cv::Point rightPoint = ObjectDetect(cv_right->image);
	cout<< "R:" << rightPoint.x << "," << rightPoint.y <<endl;


	//发布得到的目标坐标点
//	leftPointPub.publish();
//	rightPointPub.publish();
	//发布处理后的图像
//	leftProcImgPub.publish();
//	rightProcImgPub.publish();
}
*/

int main(int argc, char **argv)
{
	//加载训练好的用来人脸检测的文件
	if( !faceCascade.load(face_cascade_name) ){ ROS_ERROR("--(!)Error loading face cascade\n"); return -1; };
//  if( !eyes_cascade.load("./haarcascade_eye_tree_eyeglasses.xml") ){ printf("--(!)Error loading eyes cascade\n"); return -1; };

	ros::init(argc, argv, "find_target_in_images"); //节点名称：show_target_position
	ros::NodeHandle nh; //创建与master通信的节点
	image_transport::ImageTransport it(nh); //使用image_transport传输图像
	ros::Publisher leftPointPub = nh.advertise<geometry_msgs::Point>("left_2Dposition", 1); //发布话题：left_2Dposition
	ros::Publisher rightPointPub = nh.advertise<geometry_msgs::Point>("right_2Dposition", 1); //发布话题：right_2Dposition
	image_transport::Publisher leftProcImgPub = it.advertise("left_processed_image", 1); //发布话题，用来传输处理过的图像
	image_transport::Publisher rightProcImgPub = it.advertise("right_processed_image", 1); //发布话题
//	ros::Subscriber leftImage = n.subscribe<Image>("/left_cam/image_raw", Image, leftImgProcCallback); //订阅话题
//	ros::Subscriber rightImage = n.subscribe<Image>("/right_cam/image_raw", Image, rightImgProcCallback); //订阅话题
	image_transport::Subscriber leftRawImg = it.subscribe("/left_cam/image_raw", 1, leftImageCallback); //订阅左图像话题
	image_transport::Subscriber rightRawImg = it.subscribe("/right_cam/image_raw", 1, rightImageCallback); //订阅左图像话题
	//这部分参考https://github.com/rachillesf/stereoMagic/blob/master/src/stereo_node.cpp但是调试下来发现只处理了一个摄像头的图像
//	message_filters::Subscriber<Image> leftRawImgSub(nh, "/left_cam/image_raw", 1);
//	message_filters::Subscriber<Image> rightRawImgSub(nh, "/right_cam/image_raw", 1);
//	TimeSynchronizer<Image, Image> sync(leftRawImgSub, leftRawImgSub, 1);
//	sync.registerCallback(boost::bind(&ImgProcCallback, _1, _2)); //放到一个回调函数中
	while(ros::ok())
	{
		ros::spin();
	}

	return 0;
}
