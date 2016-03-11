#include "algoincludes.h"

#define __ALGOOPENCL
#include "algoopencl.h"

void OpenCLErrorPrint(UBYTE lnIdx,cl_int *data)
{
	BYTE disp_ptr[256];
	sprintf(disp_ptr,"%ld",(LONG)*data);
	Screen_Print(lnIdx,disp_ptr);
}
static void wrtieToFILE(char *buffer,size_t len)
{
	FILE *fp;
	fp = fopen("buildInfo.txt","wb");
	fwrite(buffer,sizeof(unsigned char),len,fp);
	fclose(fp);
}
void __OPENCL__createCLBinaries(char * clFileName, char* binaryFileName, CLParams *clObj)
{
	/*
	CLFileName      : Name of the *.cl file which has the OpenCL kernels
	binaryFileName  : Name of the *.clbin which will be created per camera. This is the binary file created with OpenCL per camera
	CLParams		: Opencl pointer to opencl framework object
	*/

	
	LONG size;
	char *sourceBuffer;
	FILE *fp = fopen(clFileName,"rb");
	if(!fp)
	{
		exit(1);
	}
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	rewind(fp);
	sourceBuffer = (char*) malloc(size*sizeof(char*));
	fread(sourceBuffer, 1, size, fp);
	sourceBuffer[size] = '\0';
	fclose (fp);

	clObj->program = clCreateProgramWithSource(clObj->context, 1, (const char**)&sourceBuffer, 0, &clObj->error);
	free(sourceBuffer);
	sourceBuffer = NULL;

	// Build the program (OpenCL JIT compilation)
	clObj->error = clBuildProgram(clObj->program, 1, &clObj->devices[0], NULL, NULL, NULL);

	if(clObj->error)
	{
		// Write the build log into a text file
		size_t len;
        char buffer[2048];
		cl_build_status bldstatus;

		clObj->error = clGetProgramBuildInfo(clObj->program,clObj->devices[0],CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len); 
		wrtieToFILE(buffer,len);
	}

	{
		BYTE i=0;
		unsigned char **binaries;
		size_t binarySize;
		FILE *fp;
		//Get the list of binary sizes
		clObj->error = clGetProgramInfo(clObj->program,CL_PROGRAM_BINARY_SIZES,sizeof(size_t),(size_t *)&binarySize,NULL);

		binaries = (unsigned char**) malloc(sizeof(unsigned char*));
		binaries[0] = (unsigned char*) malloc(binarySize*sizeof(unsigned char*));

		clObj->error = clGetProgramInfo(clObj->program,CL_PROGRAM_BINARIES,sizeof(unsigned char*),(unsigned char**)binaries,NULL);

		fp = fopen(binaryFileName,"wb");
		fwrite(binaries[0],sizeof(unsigned char),binarySize,fp);
		fclose(fp);
		free(binaries);
		binaries = NULL;
	}
}
void __OPENCL__buildProgramFromBinaries(char* binaryFileName,  CLParams *clObj)
{

	FILE *fp;
	size_t binary_size;
	char *binary_buf;
	cl_int binary_status;

	fp = fopen(binaryFileName,"r");
	if(!fp)
	{
		exit(1);
	}
	binary_buf = (char*) malloc(0x100000); // 0x100000 is the max binary size
	binary_size = fread(binary_buf,1,0x100000,fp);

	fclose(fp);

	//Create program from binary file
	clObj->program = clCreateProgramWithBinary(clObj->context, 1, &clObj->devices[0],(const size_t*)&binary_size,(const unsigned char**)&binary_buf, &binary_status, &clObj->error);
	clObj->error = clBuildProgram(clObj->program,0,NULL,NULL,NULL,NULL);
	free(binary_buf);
	binary_buf = NULL;
}
cl_int __OPENCL__deviceToHost(CLParams *clObj,cl_mem deviceMem,void* hostMem,size_t size, cl_event *event)
{
	/*Device to host data transfer
	deviceMem     -  memory location on the device(GPU)
	hostMem       - pointer to the host memory where the data from the device must be stored
	size          - length of the buffer that is being transferred 
	return        - cl_int type error code
	
	*/
	return clEnqueueReadBuffer(clObj->commandQueue, deviceMem, CL_FALSE, 0,size, hostMem, 0, NULL, event);// Copy the output in GPU memory back to CPU memory		
}
cl_int __OPENCL__hostToDevice(CLParams *clObj,cl_mem deviceMem,const void* hostMem,size_t size, cl_event *event)
{
	/*Host to device data transfer
	deviceMem     -  memory location on the device(GPU)
	hostMem       - pointer to the host memory where the data from the device must be stored
	size          - length of the buffer that is being transferred 
	return        - cl_int type error code
	*/
	return clEnqueueWriteBuffer(clObj->commandQueue,deviceMem,CL_FALSE,0,size,hostMem,0,NULL,NULL);
}
cl_kernel __OPENCL__createKernel(CLParams *clObj, const char* kernel_name)
{
	return clCreateKernel(clObj->program,kernel_name,&clObj->error);
}
static void GetDeviceInfo (UBYTE cmnum)
{
	CLParams *clObj = &clParams[cmnum];
	cl_bool imageSupport;
		
	clGetDeviceInfo(clObj->devices[0],CL_DEVICE_IMAGE_SUPPORT,sizeof(cl_bool),&imageSupport,NULL);	
	OpenCLErrorPrint(2,&imageSupport);
	
	
} 
void AlgoOpenCLInit(UBYTE cmnum)
{
	BYTE disp_ptr[128];
	CLParams *clObj = &clParams[cmnum];
	

	clObj->error  = 0;
	clObj->error = clGetPlatformIDs(1, &clObj->platform, NULL);//Get an OpenCL platform. Only 1 platform is considered
	clObj->error = clGetDeviceIDs(clObj->platform, CL_DEVICE_TYPE_GPU, 1, clObj->devices, NULL);//get a device ID
	
	{
		cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties) clObj->platform,0};
		clObj->context = clCreateContext(properties,1,&clObj->devices[0],NULL,NULL,&clObj->error);// Create a context to run OpenCL enabled GPU	
		//clObj->context = clCreateContextFromType(0, CL_DEVICE_TYPE_GPU, NULL, NULL, NULL);// Create a context to run OpenCL enabled GPU	
		clObj->commandQueue = clCreateCommandQueue(clObj->context, clObj->devices[0], 0, &clObj->error);// Create a command-queue on the GPU device
		//clGetCommandQueueInfo(clObj->commandQueue,CL_QUEUE_PROPERTIES,sizeof(cl_command_queue_properties),&clObj->properties,NULL);
	}		
	//GetDeviceInfo(cmnum);
	
}