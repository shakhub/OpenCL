#include "CL/cl.h"
#include "algoincludes.h"
#include "algoopencl.h"
#define __ALGOVECADD_CL
#include "algovecadd_cl.h"
#define VECSIZE 256

cl_kernel OpenCLVectorAdd;
cl_kernel OpenCLVectorSub;
size_t GWSize[2];
size_t LWSize[2] = {192,192}; // from clGetKernelWorkGroupInfo , LWSize = 192 always for imx6 GPU
/*
LONG HostVector1[VECSIZE] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
							1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
							1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
							1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40};
LONG HostVector2[VECSIZE] = {10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,210,220,230,240,250,260,270,280,290,300,310,320,330,340,350,360,370,380,390,400,
							10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,210,220,230,240,250,260,270,280,290,300,310,320,330,340,350,360,370,380,390,400,
							10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,210,220,230,240,250,260,270,280,290,300,310,320,330,340,350,360,370,380,390,400,
							10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,210,220,230,240,250,260,270,280,290,300,310,320,330,340,350,360,370,380,390,400};
*/
LONG HostVector1[VECSIZE*VECSIZE];
LONG HostVector2[VECSIZE*VECSIZE];
cl_mem GPUVector1;
cl_mem GPUVector2;
cl_mem GPUOutputVector;
//cl_mem GPUOutputVectorFinal;
char *binaryFileName[MAX_CAMERAS] = {"algoclkernelsourceCam0.clbin","algoclkernelsourceCam1.clbin"};
char *sourceFileName = "algoclkernelsource.cl";

static char outputFileName[100];// = "output.txt";
FILE *fp; 
LONG frame = 0;
LONG fileNum = 0;

static void createNewFile(LONG *fileNumber)
{
	
	LONG fileNum;
	fclose(fp);
	fileNum = ++*fileNumber; // increment the value the pointer is pointing to 
	sprintf(outputFileName,"output-%ld.txt",fileNum);
	fp = fopen(outputFileName,"w");	
	
}

static void cleanUpCL(CLParams *clObj)
{
	clObj->error = clFlush(clObj->commandQueue);
	clObj->error = clFinish(clObj->commandQueue);
	clReleaseContext(clObj->context);
	clReleaseKernel(OpenCLVectorAdd);
	clReleaseKernel(OpenCLVectorSub);
	clReleaseProgram(clObj->program);
	clReleaseCommandQueue(clObj->commandQueue);

}
void AlgoVecAddInit_cl(UBYTE cmnum)
{	
	CLParams *clObj = &clParams[cmnum];
	int i=0;
	int numWorkGroups;
	int numElements = VECSIZE;
	size_t size = VECSIZE*VECSIZE * sizeof(LONG);
	LONG *HostVector1_ptr;
	LONG *HostVector2_ptr;
	
	//write to a file
	frame = 0;
	fileNum = 0;
	sprintf(outputFileName,"output-%ld.txt",fileNum);
	fp=fopen(outputFileName,"w");
	fprintf(fp,"Sys reset");
	
	
	// Allocate GPU memory for source vectors AND initialize from CPU memory
	GPUVector1 = clCreateBuffer(clObj->context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, size, NULL, &clObj->error);
	GPUVector2 = clCreateBuffer(clObj->context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, size, NULL, &clObj->error);
	
	// Allocate output memory on GPU
	GPUOutputVector = clCreateBuffer(clObj->context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, size, NULL, &clObj->error);
	//GPUOutputVectorFinal = clCreateBuffer(clObj->context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, size, NULL, &clObj->error);
	
	//Map the memory buffers created by OpenCL to pointers so CPU can access it
	HostVector1_ptr	   = (LONG*)clEnqueueMapBuffer(clObj->commandQueue,GPUVector1,CL_FALSE,CL_MAP_WRITE,0,size,0,NULL,NULL,&clObj->error);
	HostVector2_ptr	   = (LONG*)clEnqueueMapBuffer(clObj->commandQueue,GPUVector2,CL_FALSE,CL_MAP_WRITE,0,size,0,NULL,NULL,&clObj->error);
	clObj->error = clFinish(clObj->commandQueue);
	
	//initialize the data
	for(i=0;i<VECSIZE*VECSIZE;i++)
	{
		HostVector1[i] = i&(VECSIZE-1);
		HostVector2[i] = i&(VECSIZE-1);
		
	}
	//copy the data into the mapped location
	memcpy(HostVector1_ptr,HostVector1,size);
	memcpy(HostVector2_ptr,HostVector2,size);

	//Un-map the memory buffer as we have finished using them from the CPU side.
	clObj->error = (cl_int) clEnqueueUnmapMemObject(clObj->commandQueue, GPUVector1,(LONG*)HostVector1_ptr,0,NULL,NULL);
	clObj->error = (cl_int) clEnqueueUnmapMemObject(clObj->commandQueue, GPUVector2,(LONG*)HostVector2_ptr,0,NULL,NULL);
	HostVector1_ptr = NULL;
	HostVector2_ptr = NULL;	
	
	__OPENCL__createCLBinaries(sourceFileName,binaryFileName[cmnum],clObj);
	//__OPENCL__buildProgramFromBinaries(binaryFileName[cmnum],clObj);

	// Create a handle to the compiled OpenCL function (Kernel)
	OpenCLVectorAdd = __OPENCL__createKernel(clObj,"VectorAdd");
	//OpenCLVectorSub = __OPENCL__createKernel(clObj,"VectorSub");

	// In the next step we associate the GPU memory with the Kernel arguments
	clObj->error  = clSetKernelArg(OpenCLVectorAdd, 0, sizeof(cl_mem), (void*)&GPUOutputVector);
	clObj->error |= clSetKernelArg(OpenCLVectorAdd, 1, sizeof(cl_mem), (void*)&GPUVector1);
	clObj->error |= clSetKernelArg(OpenCLVectorAdd, 2, sizeof(cl_mem), (void*)&GPUVector2);
	clObj->error |= clSetKernelArg(OpenCLVectorAdd, 3, sizeof(cl_uint), (void*)&numElements);
	
	/*
	clObj->error |= clSetKernelArg(OpenCLVectorSub, 0, sizeof(cl_mem), (void*)&GPUOutputVectorFinal);
	clObj->error |= clSetKernelArg(OpenCLVectorSub, 1, sizeof(cl_mem), (void*)&GPUVector1);
	clObj->error |= clSetKernelArg(OpenCLVectorSub, 2, sizeof(cl_mem), (void*)&GPUOutputVector);
	clObj->error |= clSetKernelArg(OpenCLVectorSub, 3, sizeof(cl_uint), (void*)&numElements);
	*/
	
	//clObj->error = clGetKernelWorkGroupInfo(OpenCLVectorAdd,clObj->devices[0],CL_KERNEL_WORK_GROUP_SIZE,sizeof(size_t),&LWSize,NULL);
	// This kernel only uses global data
	// one dimensional Range , reduced by 4 as we are using int4 in the kernel
	
	/*
	numWorkGroups = (VECSIZE + 191) / 192;
	GWSize[0] = numWorkGroups * 192;
	GWSize[1] = GWSize[0];
	*/
	GWSize[0] = VECSIZE;
	GWSize[1] = VECSIZE;

}
void AlgoVecAddRun_cl(UBYTE cmnum)
{
	size_t size = VECSIZE*VECSIZE * sizeof(LONG);
	BYTE disp_ptr[256];
	cl_event event;
	CLParams *clObj = &clParams[cmnum];
	LONG *HostOutputVector_ptr;	
	
	frame++; // frame counter	
	
	// Launch the Kernel on the GPU
	clObj->error = clEnqueueNDRangeKernel(clObj->commandQueue, OpenCLVectorAdd, 2, NULL,GWSize, NULL, 0, NULL, &event);	
	clObj->error = clFinish(clObj->commandQueue);	//wait for the queued OpenCL commands to execute		
	
	//if(!(frame&4095)) // create new file every  2 minutes
		// createNewFile(&fileNum);	 
	
	//Get a pointer to map the output data
	HostOutputVector_ptr = (LONG*)clEnqueueMapBuffer(clObj->commandQueue,GPUOutputVector,CL_FALSE,CL_MAP_READ,0,size,0,NULL,NULL,&clObj->error);
	clObj->error = clFinish(clObj->commandQueue);
	
	OpenCLErrorPrint(0,&(clObj->error));
	fprintf(fp,"[%ld] = %ld  [$ld] = %ld\n",VECSIZE-1,HostOutputVector_ptr[VECSIZE-1],VECSIZE*VECSIZE,HostOutputVector_ptr[VECSIZE*VECSIZE-1]);
	sprintf(disp_ptr,"[%ld] = %ld  [%ld] = %ld",VECSIZE-1,HostOutputVector_ptr[VECSIZE-1],VECSIZE*VECSIZE,HostOutputVector_ptr[VECSIZE*VECSIZE-1]);
	Screen_Print(3,disp_ptr);		
	
	//un-map the memory object
	clObj->error = (cl_int) clEnqueueUnmapMemObject(clObj->commandQueue,GPUOutputVector,(LONG*)HostOutputVector_ptr,0, NULL, NULL);
	clObj->error = clFinish(clObj->commandQueue);
}