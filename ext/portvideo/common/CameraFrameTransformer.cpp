/*  portVideo, a cross platform camera framework
 Copyright (C) 2005-2016 Martin Kaltenbrunner <martin@tuio.org>
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "CameraFrameTransformer.h"
#include "../opencl/CameraFrameTransformer_kernel.h"

const int format_pixel_size[] = { /*UKNOWN*/ -1,/*GRAY*/ 1,/*GRAY16*/ 2, /*RGB*/ 3, /*RGB16*/ 2, /*GRAY16S*/ 2, /*RGB16S*/ 2, /*RAW8*/ 1, /*RAW16*/ 2, /*RGBA*/ 4,
/*YUYV*/ 4, /*UYVY*/2, /*YUV411*/-1, /*YUV444*/-1, /*420P*/-1, /*410P*/-1,/*YVYU*/-1, /*YUV211*/ -1, /*BAYERGRBG*/1,  -1 ,/*JPEG*/-1,/*MJPEG*/-1,/*MJPEG2*/-1,/*MJPEG4*/-1,
/*H263*/-1,/*H264*/-1,/*DVPAL*/-1,-1,-1,-1,/*DVNTSC*/-1 };

void CameraFrameTransformer::init(int src_width, int src_height, int src_format, int dst_width, int dst_height, int dst_format, int dst_xoff, int dst_yoff, bool dst_flip_h, bool dst_flip_v)
{
    assert(src_width > 0 && src_height > 0);
    assert(dst_width > 0 && dst_height > 0);
    assert(format_pixel_size[src_format] > 0 && format_pixel_size[dst_format]);

    std::vector<cl::Platform> platforms;
    std::vector<cl::Device> devices;

    cl::Platform::get(&platforms);
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    
    _device = devices[0];

    _context = cl::Context(_device);

    cl::Program::Sources source(1, std::make_pair(kernel_source, kernel_source_size));
    _program = cl::Program(_context, source);

    char options[512];
    build_kernel_options(options, src_width, src_height, src_format, dst_width, dst_height, dst_format, dst_xoff, dst_yoff, dst_flip_h, dst_flip_v);

    if(_program.build(devices, options) != CL_SUCCESS)
        std::cout << "OpenCL error build kernel: " << _program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(_device) << std::endl;

    _workItemRange = cl::NDRange(dst_height);

    _kernel = cl::Kernel(_program, "transform");

    _bufferSrcSize = src_width * src_height * format_pixel_size[src_format];
    _bufferSrc = cl::Buffer(_context, CL_MEM_READ_WRITE, _bufferSrcSize);
    _kernel.setArg(0, _bufferSrc);

    _bufferDstSize = dst_width * dst_height * format_pixel_size[dst_format];
    _bufferDst = cl::Buffer(_context, CL_MEM_READ_WRITE, _bufferDstSize);
    _kernel.setArg(1, _bufferDst);

    _queue = cl::CommandQueue(_context, _device);
}

void CameraFrameTransformer::transform(unsigned char* src, unsigned char* dst)
{    
    _queue.enqueueWriteBuffer(_bufferSrc, CL_TRUE, 0, _bufferSrcSize, src);
    _queue.enqueueNDRangeKernel(_kernel, cl::NullRange, _workItemRange);
    _queue.enqueueReadBuffer(_bufferDst, CL_TRUE, 0, _bufferDstSize, dst);
    _queue.finish();
}

void CameraFrameTransformer::build_kernel_options(char* options, int src_width, int src_height, int src_format, int dst_width, int dst_height, int dst_format, int dst_xoff, int dst_yoff, bool dst_flip_h, bool dst_flip_v)
{
    sprintf(options, "-DSRC_FORMAT=%i -DSRC_FORMAT_PIXEL_SIZE=%i -DSRC_WIDTH=%i -DSRC_HEIGHT=%i -DDST_WIDTH=%i -DDST_HEIGHT=%i -DDST_FORMAT=%i -DDST_FORMAT_PIXEL_SIZE=%i -DDST_XOFF=%i -DDST_YOFF=%i", src_format, format_pixel_size[src_format], src_width, src_height, dst_width, dst_height, dst_format, format_pixel_size[dst_format], dst_xoff, dst_yoff);
    if(dst_flip_h) strcat(options, " -DDST_FLIP_H");
    if(dst_flip_v) strcat(options, " -DDST_FLIP_V");
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          