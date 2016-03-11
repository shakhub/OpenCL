
//__kernel void ImageDiff(__global unsigned char* inputCurr,__global unsigned char* inputPrev,__global unsigned char* output,unsigned int image_width,unsigned int image_height, __global unsigned int *count)
//{
//	 unsigned int x = get_global_id(0);
//	 unsigned int y = get_global_id(1);
//	 unsigned int i = x + y * image_width;
	
	
//	 count[0] = 10;
//	 if(x>=0 && x<image_width && y>=0 && y<image_height)
//	 {		
//		// output[i] = (abs((int)inputCurr[i]-(int)inputPrev[i]) > 10) * 255;
//		// count[0] = (unsigned int) output[i];		
//		 output[i] = inputCurr[i];
//	 }
	
// }

 // __constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
 
 // __kernel void ImageDiff(__read_only image2d_t input1, __read_only image2d_t input2, __write_only image2d_t output, __global int *count)
// {
	// const int2 pos = {get_global_id(0), get_global_id(1)};

	// uint4 diff;
	// uint4 pixelCurr = read_imageui(input1,sampler,pos);
	// uint4 pixelPrev = read_imageui(input2,sampler,pos);
	
	
	// diff.x = select(0,255,abs_diff(pixelCurr.x,pixelPrev.x)>10);
	// diff.y = diff.x;
	// diff.z = diff.x;
	// diff.w = diff.x;
	// count[0] = diff.x;
	// //write_imageui(output,pos,diff);

	
// }
 
__kernel void VectorAdd(__global int* restrict c, __global const int* restrict a,__global const int* restrict b, unsigned int len) 
{
 
	unsigned int x = get_global_id(0);
	unsigned int y = get_global_id(1);
	unsigned int i = x + y * len;
 
	//int4 vecA = vload4(i,a);
	//int4 vecB = vload4(i,b);

	//vstore4(vecA+vecB,i,c);
	//unsigned int x = get_global_id(0);
	//unsigned int y = get_global_id(1);
	//unsigned int i = x + y * len;
	
	if(x>=0 && x < len && y>=0 && y<len)
		c[i] = a[i]+b[i];
}

//__kernel void VectorSub(__global int* c, __global const int* a,__global const int* b, unsigned int len) 
//{
 // Index of the elements to add 
// unsigned int n = get_global_id(0);
 // Sum the nth element of vectors a and b and store in c 
// if(n<len)
// c[n] = b[n] - a[n];
//}

