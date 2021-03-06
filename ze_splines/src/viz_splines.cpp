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

#include <ze/splines/viz_splines.hpp>

#include <ze/matplotlib/matplotlibcpp.hpp>
#include <ze/visualization/viz_interface.hpp>

namespace ze {

SplinesVisualizer::SplinesVisualizer(const std::shared_ptr<Visualizer>& viz)
  : viz_(viz)
{}

void SplinesVisualizer::plotSpline(const BSpline& bs,
                                   const size_t dimension,
                                   real_t step_size)
{
  real_t start = bs.t_min();
  real_t end = bs.t_max();
  size_t samples = (end - start) / step_size - 1;
  size_t spline_dimension = bs.dimension();
  CHECK_LE(dimension, spline_dimension);

  ze::VectorX points(samples);
  ze::VectorX times(samples);

  for (size_t i = 0; i < samples; ++i)
  {
    points(i) = (bs.eval(start + i * step_size))(dimension);
    times(i) = start + i * step_size;
  }

  plt::plot(times, points);
  plt::show();
}

void SplinesVisualizer::displaySplineTrajectory(const BSpline& bs,
                                                const std::string& topic,
                                                const size_t id,
                                                const Color& color,
                                                real_t step_size)
{
  CHECK_LE(bs.dimension(), 3) << "For splines with more than 3 dimensions specify"
                              << "which dimenions to draw.";

  switch (bs.dimension())
  {
    case 1:
      return displaySplineTrajectory(bs, topic, id, color, {0}, step_size);
    case 2:
      return displaySplineTrajectory(bs, topic, id, color, {0, 1}, step_size);
    case 3:
      return displaySplineTrajectory(bs, topic, id, color, {0, 1, 2}, step_size);
  }
}

void SplinesVisualizer::displaySplineTrajectory(
    const BSpline& bs,
    const std::string& topic,
    const size_t id,
    const Color& color,
    const std::vector<size_t>& draw_dimensions,
    real_t step_size)
{
  real_t start = bs.t_min();
  real_t end = bs.t_max();
  size_t samples = (end - start) / step_size - 1;
  size_t dimension = bs.dimension();

  ze::Positions points(3, samples);
  points.setZero();

  //! @todo: this is terribly inefficient.
  for (size_t i = 0; i < samples; ++i)
  {
    VectorX v = bs.eval(start + i * step_size);
    for (size_t j = 0; j < draw_dimensions.size(); ++j)
    {
      points(j, i) = v(draw_dimensions[j]);
    }
  }

  viz_->drawPoints(topic, id, points, color);
}

void SplinesVisualizer::displaySplineTrajectory(
    const BSplinePoseMinimalRotationVector& bs,
    const std::string& topic,
    const size_t id,
    real_t step_size)
{
  ze::TransformationVector poses;

  real_t start = bs.t_min();
  real_t end = bs.t_max();
  size_t samples = (end - start) / step_size - 1;

  for (size_t i = 0; i <= samples; ++i)
  {
    Transformation T(bs.transformation(start + i * step_size));
    poses.push_back(T);
  }

  viz_->drawCoordinateFrames(topic, id, poses,  1);
}

void SplinesVisualizer::plotSpline(
    const BSplinePoseMinimalRotationVector& bs,
    real_t step_size)
{
  real_t start = bs.t_min();
  real_t end = bs.t_max();
  size_t samples = (end - start) / step_size - 1;

  ze::MatrixX points(6, samples);
  ze::VectorX times(samples);

  for (size_t i = 0; i < samples; ++i)
  {
    points.col(i) = bs.eval(start + i * step_size);
    times(i) = start + i * step_size;
  }

  // translation
  plt::subplot(3, 1, 1);
  plt::plot(times, points.row(0));
  plt::subplot(3, 1, 2);
  plt::plot(times, points.row(1));
  plt::subplot(3, 1, 3);
  plt::plot(times, points.row(2));
  plt::show(false);

  // rotation
  plt::figure();
  plt::subplot(3, 1, 1);
  plt::plot(times, points.row(3));
  plt::subplot(3, 1, 2);
  plt::plot(times, points.row(4));
  plt::subplot(3, 1, 3);
  plt::plot(times, points.row(5));
  plt::show();
}

} // namespace ze
