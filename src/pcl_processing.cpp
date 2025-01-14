#include <iostream>
#include <string>
#include <sstream>

#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
#include <std_msgs/Int64.h>
#include <pcl_conversions/pcl_conversions.h>

#include <pcl/io/pcd_io.h>
#include <pcl/point_cloud.h>
#include <pcl/filters/passthrough.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/extract_indices.h>

#include <Eigen/Eigen>

#include <iterator>
#include <vector>

#include <pcl/visualization/cloud_viewer.h>

typedef pcl::PointXYZRGBA PointT;
static int pcd_index = 0;
static char gotDataFlag = 0;// could use a 'class' to reduce this global variable
static int pcl_registered_flag = 1;

pcl::PointCloud<PointT>::Ptr
cloud_filter(pcl::PointCloud<PointT>::Ptr &cloud);

void
reg_callback(std_msgs::Int64 msg)
{
  pcl_registered_flag = msg.data;
}

void
callback(sensor_msgs::PointCloud2 cloud_raw)
{
    // auto data = cloud_raw.data;
    // std::vector<int> v(std::begin(data), std::end(data));
    // for (int i = 0; i < v.size(); i++) {
    //   std::cout << v[i] << "\n";
    // }
    // int width = cloud_raw.width;
    // ROS_INFO("raw cloud height: %d", width);
    // cloud_raw is PC data from Kinect V2;
    // static int pcd_index = 0;
    pcl::PointCloud<PointT>::Ptr cloud_ptr (new pcl::PointCloud<PointT>);
    std::string filename = "/home/lachlan/catkin_ws/src/scanner/data/" + std::to_string(pcd_index) + ".pcd";

    // ROS_INFO("Processing #%i PointCloud...", pcd_index);

    // change PC format from PointCloud2 to pcl::PointCloud<PointT>
    pcl::fromROSMsg(cloud_raw, *cloud_ptr);

    // TEST SAVE (failed)
    // std::string name = "/home/lachlan/catkin_ws/src/scanner/data/TEST.pcd";
    // pcl::io::savePCDFileBinary(name, *cloud_ptr);

    // crop, segment, filter
    cloud_ptr = cloud_filter(cloud_ptr);

    // Check PCL has size (it does)    
    // int size = cloud_ptr->size();
    // ROS_INFO("size: %d", size);

    // save PCD file to local folder
    pcl::io::savePCDFileBinary(filename, *cloud_ptr);  // ERROR
    // ROS_INFO("Post-save");

    gotDataFlag = 1;

    // cout << "\nPress Enter to Continue";
    // cin.ignore();

    // ++pcd_index;
}


int
main (int argc, char **argv)
{
    
    ros::init (argc, argv, "pcl_processing");

    ros::NodeHandle nh; // can sub and pub use the same NodeHandle?
    ros::Subscriber sub = nh.subscribe("/kinect2/qhd/points", 1 , callback);
    // ros::Subscriber sub = nh.subscribe("/realsense/depth/points", 1 , callback);
    ros::Publisher pub = nh.advertise<std_msgs::Int64> ("pcd_save_done", 1);

    ros::Subscriber reg_sub = nh.subscribe("pcl_registered", 1 , reg_callback);

    ros::Rate loop_rate(1);
    std_msgs::Int64 number_PCDdone;
    // std::stringstream ss;
    while (ros::ok())
    {
        /* Do something? */
        // ss.str("");
        // ss << "have saved pcd #" << pcd_index ;
        // msg.data = ss.str();

        // cout << "\nPress Enter to Continue";
        //

        number_PCDdone.data = pcd_index;
        

        // ros::spin()
        //*** only when this is run, it will get to callback
        // cout << "\nPress Enter To Capture Point Cloud\n";
        // cin.ignore();
        ros::spinOnce();

        // Check if latest point cloud has been registered
        // ROS_INFO("Point Cloud Registered: %d", pcl_registered_flag);
        if (pcl_registered_flag == 1){
          // only publish data when having got data
          if (gotDataFlag == 1){
              cout << "\nMove Table And Press Enter To Process Point Cloud\n";
              cin.ignore();
              ROS_INFO("Publishing Processed Point Cloud #%d", number_PCDdone.data);
              pcl_registered_flag = 0;
              pub.publish(number_PCDdone);
              gotDataFlag = 0;
              ++pcd_index;
          }
        }
        // ros::spinOnce();
        loop_rate.sleep();
    }
    return 0;
}


pcl::PointCloud<PointT>::Ptr
cloud_filter(pcl::PointCloud<PointT>::Ptr &cloud)
{
    pcl::PointCloud<PointT>::Ptr cloud_filtered (new pcl::PointCloud<PointT>);

  //****************************************************//
    // Create the filtering object - passthrough
    pcl::PassThrough<PointT> passz;
    passz.setInputCloud (cloud);
    passz.setFilterFieldName ("z");
    passz.setFilterLimits (0.25, 0.75);
    // passz.setFilterLimits (0, 2.0);

    // passz.setFilterLimits (0.5, 1.5);

    // passz.setFilterLimits (-2.0, 4.0);
    //pass.setFilterLimitsNegative (true);
    passz.filter (*cloud_filtered);

    pcl::PassThrough<PointT> passy;
    passy.setInputCloud (cloud_filtered);
    passy.setFilterFieldName ("y");
    passy.setFilterLimits (-0.30, 0.10);
    // passy.setFilterLimits (-0.5, 0.5);

    // passy.setFilterLimits (-2.0, 2.0);
    //pass.setFilterLimitsNegative (true);
    passy.filter (*cloud_filtered);

    pcl::PassThrough<PointT> passx;
    passx.setInputCloud (cloud_filtered);
    passx.setFilterFieldName ("x");
    passx.setFilterLimits (-0.15, 0.15);
    // passx.setFilterLimits (-0.5, 0.5);

    // passx.setFilterLimits (-3.0, 3.0);
    //pass.setFilterLimitsNegative (true);
    passx.filter (*cloud_filtered);
  //****************************************************//



  //****************************************************//
    // // segment ground
    // pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
    // pcl::PointIndices::Ptr inliers (new pcl::PointIndices);
    // // Create the segmentation object
    // pcl::SACSegmentation<PointT> seg;
    // // Optional
    // seg.setOptimizeCoefficients (true);
    // // Mandatory
    // seg.setModelType (pcl::SACMODEL_PLANE);  // plane
    // seg.setMethodType (pcl::SAC_RANSAC);
    // seg.setDistanceThreshold (0.010);

    // seg.setInputCloud (cloud_filtered);
    // seg.segment (*inliers, *coefficients);

    // pcl::ExtractIndices<PointT> extract;
    // extract.setInputCloud(cloud_filtered);
    // extract.setIndices(inliers);
    // extract.setNegative(true);
    // extract.filter(*cloud_filtered);
  //****************************************************//


  //****************************************************//
    // Create the filtering object - StatisticalOutlierRemoval filter
    // -------- UNCOMMENT THIS --------------------
    pcl::StatisticalOutlierRemoval<PointT> sor;
    sor.setInputCloud (cloud_filtered);
    sor.setMeanK (50);
    sor.setStddevMulThresh (1.0);
    sor.filter (*cloud_filtered);

  //****************************************************//

    // pcl::PointCloud<PointT>::Ptr cloud_write (new pcl::PointCloud<PointT>);
    // cloud_write.width = cloud_filtered.points.size();
    // cloud_write.height = 1;
    // cloud_write.is_dense = false;

    // return cloud;

    // VISUALISE
    // pcl::visualization::CloudViewer viewer ("Simple Cloud Viewer");
    // viewer.showCloud (cloud_filtered);
    // while (!viewer.wasStopped ())
    // {
    // }
   
    return cloud_filtered;

}
