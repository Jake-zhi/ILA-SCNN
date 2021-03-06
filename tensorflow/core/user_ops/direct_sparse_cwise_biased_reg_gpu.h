#pragma once

#if !GOOGLE_CUDA
#error This file must only be included when building with Cuda support
#endif

#include "tensorflow/core/framework/op_kernel.h"

namespace tensorflow {
typedef Eigen::GpuDevice GPUDevice;

  namespace functor {
    template <typename DeviceT, typename T, typename IndiceT, int data_dimension, int power>
    struct DirectSparseCwiseBiasedRegFunctor {
      void operator()(OpKernelContext* context) const;
    };
    template <typename DeviceT, typename T, typename IndiceT, int data_dimension, int power>
    struct DirectSparseCwiseBiasedRegBpFunctor {
      void operator()(OpKernelContext* context) const;
    };
  } //namespace functor
} //namespace tensorflow
