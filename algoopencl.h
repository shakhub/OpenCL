#ifndef __ALGOOPENCL_H
#define __ALGOOPENCL_H

#include<CL/cl.h>

#define CL_MAX_DEVICES 10 //max devices present (gpu,cpu,fpga,dsp...)
typedef struct CL_parameters CLParams;
struct CL_parameters
{
	cl_command_queue_properties properties;
	cl_int error;
	cl_platform_id platform;
	cl_device_id devices[CL_MAX_DEVICES];
	cl_context context;
	cl_command_queue commandQueue;
	cl_program program;
};
#ifdef __ALGOOPENCL

CLParams clParams[MAX_CAMERAS];
#else
extern CLParams clParams[MAX_CAMERAS];
#endif //__ALGOOPENCL

void __OPENCL__createCLBinaries(char * , char* , CLParams *);
void __OPENCL__buildProgramFromBinaries(char* ,  CLParams *);
cl_int __OPENCL__deviceToHost(CLParams* ,cl_mem device,void* hostPtr,size_t , cl_event *);
cl_int __OPENCL__hostToDevice(CLParams* ,cl_mem device,const void* hostPtr,size_t , cl_event *);
cl_kernel __OPENCL__createKernel(CLParams *, const char*);
void AlgoOpenCLInit(UBYTE);
void OpenCLErrorPrint(UBYTE ,cl_int *);
#endif //__ALGOOPENCL_H