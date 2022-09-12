#include <drag_rtmp.h>
#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/Image.h>

int main(int argc, char *argv[])
{
    string node_arg = argv[1];
    string drag_addr;
    string node_name;
    string publish_topic;
    int fps;

    if (node_arg == "zedm")
    {
        node_name = "/drag_zedm";
        publish_topic = "/drag_zedm_image";
        drag_addr = "rtmp://1.116.137.21:7788/zedm";
    }
    else if (node_arg == "realsense_dis")
    {
        node_name = "/drag_realsense_dis";
        publish_topic = "/drag_realsense_distort_image";
        drag_addr = "rtmp://1.116.137.21:7789/realsense_distort";
    }
    else if (node_arg == "realsense_undis")
    {
        node_name = "/drag_realsense_undis";
        publish_topic = "/drag_realsense_undistort_image";
        drag_addr = "rtmp://1.116.137.21:7790/realsense_undistort";
    }
    else if (node_arg == "realsense_dis_rect")
    {
        node_name = "/drag_realsense_dis_rect";
        publish_topic = "/drag_realsense_distort_image_rect";
        drag_addr = "rtmp://1.116.137.21:7789/realsense_distort";
    }
    else
    {
        ROS_ERROR("Wrong node argument!");
    }

    ros::init(argc, argv, node_name);
    ros::NodeHandle nh;

    ros::param::get(node_name + "/fps", fps);
    // cout << "fps:" << fps << endl;

    DragRTMP::Ptr drag_rtmp = make_shared<DragRTMP>(fps);
    if (!drag_rtmp->initContext(drag_addr))
    {
        ROS_ERROR("Failed to initialize RTMP!");
        return -1;
    }

    Mat map1, map2;
    if (node_arg == "realsense_dis_rect")
    {
        drag_rtmp->initRectify(map1, map2);
    }

    image_transport::ImageTransport it(nh);
    image_transport::Publisher pub = it.advertise(publish_topic, 1);

    Mat img;
    sensor_msgs::ImagePtr img_msg;
    double dt = 0.01;
    ros::Rate loop(1 / dt);
    while (ros::ok())
    {
        drag_rtmp->startDrag();
        img = drag_rtmp->getImage();
        if (node_arg == "realsense_dis_rect")
        {
            remap(img, img, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
        }
        imshow(node_name, img);
        waitKey(1);
        img_msg = cv_bridge::CvImage(std_msgs::Header(), sensor_msgs::image_encodings::BGR8, img).toImageMsg();
        pub.publish(*img_msg);
        ros::spinOnce();
        loop.sleep();
    }

    return 0;
}