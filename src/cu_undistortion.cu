#include <imp/cu_core/cu_linearmemory.cuh>
#include <imp/cu_core/cu_texture.cuh>
#include <imp/cu_imgproc/cu_remap.cuh>
#include <imp/cu_imgproc/cu_undistortion.cuh>

namespace ze {
namespace cu {

template<typename CameraModel,
         typename DistortionModel>
__global__
void k_computeUndistortionMap(
    Pixel32fC2* dst,
    size_t dst_stride,
    std::uint32_t width,
    std::uint32_t height,
    const float* d_cam_params,
    const float* d_dist_coeffs)
{
  const int x = blockIdx.x*blockDim.x + threadIdx.x;
  const int y = blockIdx.y*blockDim.y + threadIdx.y;

  if (x < width && y < height)
  {
    float px[2]{static_cast<float>(x), static_cast<float>(y)};
    CameraModel::backProject(d_cam_params, px);
    DistortionModel::distort(d_dist_coeffs, px);
    CameraModel::project(d_cam_params, px);
    dst[y*dst_stride + x][0] = px[0];
    dst[y*dst_stride + x][1] = px[1];
  }
}

template <typename CameraModel,
          typename DistortionModel,
          typename Pixel>
ImageUndistorter<CameraModel, DistortionModel, Pixel>::ImageUndistorter(
    Size2u img_size,
    Eigen::VectorXf& camera_params,
    Eigen::VectorXf& dist_coeffs)
  : undistortion_map_(img_size),
    fragm_(img_size)
{
  ze::LinearMemory32fC1 h_cam_params(
        reinterpret_cast<Pixel32fC1*>(camera_params.data()),
        camera_params.rows(), true);
  ze::LinearMemory32fC1 h_dist_coeffs(
        reinterpret_cast<Pixel32fC1*>(dist_coeffs.data()),
        dist_coeffs.rows(), true);

  cu::LinearMemory32fC1 d_cam_params(h_cam_params);
  cu::LinearMemory32fC1 d_dist_coeffs(h_dist_coeffs);

  k_computeUndistortionMap<CameraModel, DistortionModel>
      <<<
        fragm_.dimGrid, fragm_.dimBlock
      >>> (undistortion_map_.data(),
           undistortion_map_.stride(),
           undistortion_map_.width(),
           undistortion_map_.height(),
           d_cam_params.cuData(),
           d_dist_coeffs.cuData());
}

template <typename CameraModel,
          typename DistortionModel,
          typename Pixel>
void ImageUndistorter<CameraModel, DistortionModel, Pixel>::undistort(
    ImageGpu<Pixel>& dst,
    const ImageGpu<Pixel>& src) const
{
  CHECK_EQ(src.size(), dst.size());
  CHECK_EQ(src.size(), undistortion_map_.size());
  std::shared_ptr<Texture2D> src_tex =
      src.genTexture(false, cudaFilterModeLinear);
  IMP_CUDA_CHECK();
  k_remap
      <<<
        fragm_.dimGrid, fragm_.dimBlock
      >>> (dst.data(),
           dst.stride(),
           undistortion_map_.data(),
           undistortion_map_.stride(),
           dst.width(),
           dst.height(),
           *src_tex);
  IMP_CUDA_CHECK();
}

template <typename CameraModel,
          typename DistortionModel,
          typename Pixel>
const ImageGpu32fC2& ImageUndistorter<CameraModel, DistortionModel, Pixel>::getUndistortionMap() const
{
  return undistortion_map_;
}

// Explicit template instantiations
template class ImageUndistorter<PinholeGeometry, EquidistantDistortion, Pixel32fC1>;
template class ImageUndistorter<PinholeGeometry, RadialTangentialDistortion, Pixel32fC1>;

} // cu namespace
} // ze namespace
