/*-------------------------------------------------------------
Copyright 2019 Wenxin Liu, Kartik Mohta, Giuseppe Loianno

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
--------------------------------------------------------------*/

#include <sdd_vio/utils/calib_utils.h>
#include <vector>

namespace sdd_vio {
namespace utils {

Eigen::Isometry3d getTransformEigen(const ros::NodeHandle &nh,
                                    const std::string &field) {
  Eigen::Isometry3d T;
  cv::Mat c = getTransformCV(nh, field);

  T.linear()(0, 0)   = c.at<double>(0, 0);
  T.linear()(0, 1)   = c.at<double>(0, 1);
  T.linear()(0, 2)   = c.at<double>(0, 2);
  T.linear()(1, 0)   = c.at<double>(1, 0);
  T.linear()(1, 1)   = c.at<double>(1, 1);
  T.linear()(1, 2)   = c.at<double>(1, 2);
  T.linear()(2, 0)   = c.at<double>(2, 0);
  T.linear()(2, 1)   = c.at<double>(2, 1);
  T.linear()(2, 2)   = c.at<double>(2, 2);
  T.translation()(0) = c.at<double>(0, 3);
  T.translation()(1) = c.at<double>(1, 3);
  T.translation()(2) = c.at<double>(2, 3);
  return T;
}

cv::Mat getTransformCV(const ros::NodeHandle &nh,
                       const std::string &field) {
  cv::Mat T;
  try {
    // first try reading kalibr format
    T = getKalibrStyleTransform(nh, field);
  } catch (std::runtime_error &e) {
    // maybe it's the old style format?
    ROS_DEBUG_STREAM("cannot read transform " << field
                     << " in kalibr format, trying old one!");
    try {
      T = getVec16Transform(nh, field);
    } catch (std::runtime_error &e) {
      std::string msg = "cannot read transform " + field + " error: " + e.what();
      ROS_ERROR_STREAM(msg);
      throw std::runtime_error(msg);
    }
  }
  return T;
}

cv::Mat getVec16Transform(const ros::NodeHandle &nh,
                          const std::string &field) {
  std::vector<double> v;
  nh.getParam(field, v);
  if (v.size() != 16) {
    throw std::runtime_error("invalid vec16!");
  }
  cv::Mat T = cv::Mat(v).clone().reshape(1, 4); // one channel 4 rows
  return T;
}

cv::Mat getKalibrStyleTransform(const ros::NodeHandle &nh,
                                const std::string &field) {
  cv::Mat T = cv::Mat::eye(4, 4, CV_64FC1);
  XmlRpc::XmlRpcValue lines;
  if (!nh.getParam(field, lines)) {
    throw (std::runtime_error("cannot find transform " + field));
  }
  if (lines.size() != 4 || lines.getType() != XmlRpc::XmlRpcValue::TypeArray) {
    throw (std::runtime_error("invalid transform " + field));
  }
  for (int i = 0; i < lines.size(); i++) {
    if (lines.size() != 4 || lines.getType() != XmlRpc::XmlRpcValue::TypeArray) {
      throw (std::runtime_error("bad line for transform " + field));
    }
    for (int j = 0; j < lines[i].size(); j++) {
      if (lines[i][j].getType() != XmlRpc::XmlRpcValue::TypeDouble) {
        throw (std::runtime_error("bad value for transform " + field));
      } else {
        T.at<double>(i,j) = static_cast<double>(lines[i][j]);
      }
    }
  }
  return T;
}

} // namespace utils
} // namespace sdd_vio
