// Copyright (c) 2015-2016, ETH Zurich, Wyss Zurich, Zurich Eye
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the ETH Zurich, Wyss Zurich, Zurich Eye nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL ETH Zurich, Wyss Zurich, Zurich Eye BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <ze/vi_simulation/imu_bias_simulator.hpp>
#include <ze/matplotlib/matplotlibcpp.hpp>
#include <ze/splines/bspline_pose_minimal.hpp>

//! A couple of useful functions to evaluation simulation results with IMUs.
namespace ze {

// -----------------------------------------------------------------------------
//! Takes the accel / gyro bias noise density in the continuous-case and
//! a constant vector valued bias otherwise.
ImuBiasSimulator::Ptr generateImuBias(real_t start,
                             real_t end,
                             const std::string& type,
                             Vector3 imu_acc_bias,
                             Vector3 imu_gyr_bias)
{
  //! continuous bias model
  if (type == "continuous")
  {

    return std::make_shared<ContinuousBiasSimulator>(
          imu_acc_bias,
          imu_gyr_bias,
          start,
           end,
          100); // This is an arbitrary value.
  }
  //! a simple constant bias
  else
  {
    return std::make_shared<ConstantBiasSimulator>(imu_acc_bias, imu_gyr_bias);
  }
}

//-----------------------------------------------------------------------------
void plotOrientation(
    const std::vector<real_t>& times,
    const Eigen::Matrix<real_t, 3, Eigen::Dynamic>& points,
    const std::string& label,
    const Eigen::Matrix<real_t, 3, Eigen::Dynamic>* ref_points = nullptr)
{
  plt::figure("orientation");
  plt::subplot(3, 1, 1);
  plt::title("Orientation");
  plt::labelPlot(label, times, points.row(0));
  if(ref_points)
  {
    plt::labelPlot("reference", times, ref_points->row(0));
  }
  plt::legend();

  plt::subplot(3, 1, 2);
  plt::labelPlot(label, times, points.row(1));
  if(ref_points)
  {
    plt::labelPlot("reference", times, ref_points->row(1));
  }
  plt::subplot(3, 1, 3);
  plt::labelPlot(label, times, points.row(2));
  if(ref_points)
  {
    plt::labelPlot("reference", times, ref_points->row(2));
  }
  plt::show(false);
}

//-----------------------------------------------------------------------------
void plotOrientation(
    const std::vector<real_t>& times,
    const QuaternionVector& orientation,
    const std::string& label,
    const BSplinePoseMinimalRotationVector::Ptr reference = nullptr)
{
  CHECK_EQ(times.size(), orientation.size());

  Eigen::Matrix<real_t, 3, Eigen::Dynamic> points(3, orientation.size());
  Eigen::Matrix<real_t, 3, Eigen::Dynamic> ref_points(3, orientation.size());

  for (size_t i = 0; i < orientation.size(); ++i)
  {
    ze::sm::RotationVector rv(Matrix3(orientation[i].getRotationMatrix()));
    points.col(i) = rv.getParameters();
    // Hanlde sign flips in the rotation vector.
    if (i > 0 && (points.col(i-1) - points.col(i)).norm() >
        (points.col(i-1) + points.col(i)).norm())
    {
      points.col(i) = -points.col(i);
    }
    if (reference)
    {
      ref_points.col(i) = reference->eval(times[i]).tail<3>();
    }
  }

  if (reference)
  {
    plotOrientation(times, points, label, &ref_points);
  }
  else
  {
    plotOrientation(times, points, label);
  }
}

//-----------------------------------------------------------------------------
void plotOrientation(
    const std::vector<real_t>& times,
    const std::vector<Matrix3>& orientation,
    const std::string& label,
    const BSplinePoseMinimalRotationVector::Ptr& trajectory = nullptr)
{
  CHECK_EQ(times.size(), orientation.size());

  Eigen::Matrix<real_t, 3, Eigen::Dynamic> points(3, orientation.size());
  Eigen::Matrix<real_t, 3, Eigen::Dynamic> ref_points(3, orientation.size());

  for (size_t i = 0; i < orientation.size(); ++i)
  {
    ze::sm::RotationVector rv(orientation[i]);
    points.col(i) = rv.getParameters();
    if (trajectory)
    {
      ref_points.col(i) = trajectory->eval(times[i]).tail<3>();
    }
  }

  if (trajectory)
  {
    plotOrientation(times, points, label, &ref_points);
  }
  else
  {
    plotOrientation(times, points, label);
  }
}

//-----------------------------------------------------------------------------
void plotOrientationError(
    const std::vector<real_t>& times,
    const std::vector<Matrix3>& est,
    const std::string& label,
    const BSplinePoseMinimalRotationVector::Ptr trajectory)
{
  CHECK_EQ(times.size(), est.size());

  Eigen::Matrix<real_t, 1, Eigen::Dynamic> err(1, est.size());

  for (size_t i = 0; i < est.size(); ++i)
  {
    Quaternion q1(est[i]);
    Quaternion q2(trajectory->orientation(times[i]));
    err(i) = q1.getDisparityAngle(q2);
  }
  plt::figure("orientation_offset");
  plt::title("Orientation Offset");
  plt::labelPlot(label, times, err.row(0));
  plt::legend();

  plt::show(false);
}

//-----------------------------------------------------------------------------
void plotImuMeasurements(
    const std::vector<real_t>& times,
    const ImuAccGyrContainer& measurements)
{
  CHECK_EQ(static_cast<int>(times.size()), measurements.cols());

  plt::figure("imu_measurements");
  plt::subplot(3, 1, 1);
  plt::title("Imu Measurements (Accel)");
  plt::plot(times, measurements.row(0));

  plt::subplot(3, 1, 2);
  plt::plot(times, measurements.row(1));

  plt::subplot(3, 1, 3);
  plt::plot(times, measurements.row(2));

  plt::show(false);

  plt::figure();
  plt::subplot(3, 1, 1);
  plt::title("Imu Measurements (Gyro)");
  plt::plot(times, measurements.row(3));

  plt::subplot(3, 1, 2);
  plt::plot(times, measurements.row(4));

  plt::subplot(3, 1, 3);
  plt::plot(times, measurements.row(5));

  plt::show(false);
}

} // namespace ze
