#include <omp.h>
#include <map>
#include <sstream>
#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/framework/common_shape_fns.h"

#if GOOGLE_CUDA
#include "direct_sparse_to_dense_gpu.h"
#endif //GOOGLE_CUDA


/** DirectSparseConvKD
  * \ingroup CXX11_NeuralNetworks_Module
  * 
  * \brief Applies a 3D convolution over a multichannel input voxel block.
  * 
  * The input parameter is expected to be a tensor with a rank of 4 or more (channels, depth, height, width, and optionally others).
  * The kernel parameter is expected to be a 5D tensor (filters, channels, kernel_depth, kernel_height, kernel_width).
  * 
  * The result can be assigned to a tensor of rank equal to the rank of the input. The dimensions of the result will be filters, depth, height, width (and others if applicable).
  */

REGISTER_OP("DirectSparseToDense")
  .Attr("T: realnumbertype")
  .Attr("Tindices: {int32, int64}")
  .Input("in_indices: Tindices")
  .Input("in_values: T")
  .Input("in_shape: Tindices")
  .Input("in_block_channel_mapping: int32")
  .Output("out_values: T")
  .Attr("dim: int = 5");

REGISTER_OP("DirectSparseToDenseBackprop")
  .Attr("T: realnumbertype")
  .Attr("Tindices: {int32, int64}")
  .Input("in_indices: Tindices")
  .Input("in_values: T")
  .Input("in_shape: Tindices")
  .Input("in_block_channel_mapping: int32")
  .Input("out_values: T")
  .Input("grads: T")
  .Output("backprops: T")
  .Attr("dim: int = 5");

REGISTER_OP("DirectDenseToSparse")
  .Attr("T: realnumbertype")
  .Attr("Tindices: {int32, int64}")
  .Input("in_values: T")
  .Input("in_shape: Tindices") //dummy to define type of Tindices 
  .Output("out_indices: Tindices")
  .Output("out_values: T")
  .Output("out_shape: Tindices")
  .Output("out_block_channel_mapping: int32")
  .Attr("dim: int = 5");

REGISTER_OP("DirectDenseToSparseBackprop")
  .Attr("T: realnumbertype")
  .Attr("Tindices: {int32, int64}")
  .Input("in_values: T")
  .Input("out_indices: Tindices")
  .Input("out_values: T")
  .Input("out_shape: Tindices")
  .Input("out_block_channel_mapping: int32")
  .Input("grads: T")
  .Output("backprops: T")
  .Attr("dim: int = 5");

#include "tensorflow/core/framework/op_kernel.h"

typedef Eigen::ThreadPoolDevice CPUDevice;

using namespace tensorflow;

template <typename DeviceT, typename T, typename Tindices, template<typename, typename, typename, int> class FunctorT>
class DirectSparseToDense : public OpKernel {
 public:
 explicit DirectSparseToDense(OpKernelConstruction* context) : OpKernel(context) {
     OP_REQUIRES_OK(context, context->GetAttr("dim", &dim));
   }

  void Compute(OpKernelContext* context) override {
    //functor requires kernel context since output shape is not known befor computing results
    if(dim == 5){
      FunctorT<DeviceT, T, Tindices, 5>()(context);
    } //TODO: add more dimensions
  }
 private:
   int32 dim;
};

#if GOOGLE_CUDA
#define REGISTER_GPU_TYPE(type, indice_type)      \
  REGISTER_KERNEL_BUILDER(Name("DirectSparseToDense").Device(DEVICE_GPU).TypeConstraint<type>("T").TypeConstraint<indice_type>("Tindices"), \
                          DirectSparseToDense<GPUDevice, type, indice_type, functor::DirectSparseToDenseFunctor>); \
  REGISTER_KERNEL_BUILDER(Name("DirectSparseToDenseBackprop").Device(DEVICE_GPU).TypeConstraint<type>("T").TypeConstraint<indice_type>("Tindices"), \
                          DirectSparseToDense<GPUDevice, type, indice_type, functor::DirectSparseToDenseBackpropFunctor>); \
  REGISTER_KERNEL_BUILDER(Name("DirectDenseToSparse").Device(DEVICE_GPU).TypeConstraint<type>("T").TypeConstraint<indice_type>("Tindices"), \
                          DirectSparseToDense<GPUDevice, type, indice_type, functor::DirectDenseToSparseFunctor>); \
  REGISTER_KERNEL_BUILDER(Name("DirectDenseToSparseBackprop").Device(DEVICE_GPU).TypeConstraint<type>("T").TypeConstraint<indice_type>("Tindices"), \
                          DirectSparseToDense<GPUDevice, type, indice_type, functor::DirectDenseToSparseBackpropFunctor>); 

#define REGISTER_GPU_ALL(type) \
  REGISTER_GPU_TYPE(type, int64); \
  REGISTER_GPU_TYPE(type, int32);


REGISTER_GPU_ALL(float);
//REGISTER_GPU_ALL(double);
//REGISTER_GPU_ALL(int32);
//REGISTER_GPU_ALL(complex64);
//REGISTER_GPU_ALL(complex128);
#undef REGISTER_GPU_TYPE
#undef REGISTER_GPU_ALL
#endif //GOOGLE_CUDA

