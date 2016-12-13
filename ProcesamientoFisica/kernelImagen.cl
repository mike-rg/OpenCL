__kernel																
void rotation (__global float * imageIn,								
			    __global float * imageOut,								
			    int W, int H, int x0, int y0,							
				float sinTheta, float cosTheta)							
{																		
	int x1 = get_global_id(0);											
	int y1 = get_global_id(1);											
	float x2 = cosTheta * (x1 - x0) - sinTheta * (y1 - y0) + x0;	  				
	float y2 = sinTheta * (x1 - x0) + cosTheta * (y1 - y0) + y0;		
	if ((((int)x2>=0) && ((int)x2< W))	&& (((int)y2>=0) && ((int)y2< H)))
	{																	
		imageOut[y1*W+x1] = imageIn[(int)y2*W+(int)x2];		   			
	}																	
}

__kernel																		
void horizontal (__global float *imageIn,									
				  __global float *imageOut,									
				  int width)												
{																			
	int column = get_global_id(0);											
	int row = get_global_id(1);												
	// Desplazamiento en el eje x respecto a la columna actual.				
	int offset = width - 1 - column * 2;									
	// Copia pixeles al buffer de salida									
	imageOut[row * width + column] = imageIn[row * width + column + offset];
	imageOut[row * width + column + offset] = imageIn[row * width + column];
}																			

__kernel																			 
void vertical (__global float *imageIn,											
				__global float *imageOut,											
				int width,															
				int height)															
{																					
	int column = get_global_id(0);													
	int row = get_global_id(1);														
	// Desplazamiento en el eje y respecto a la fila actual.						
	int offset = height - 1 - row * 2;												
	// Copia pixeles al buffer de salida											
	imageOut[row * width + column] = imageIn[row * width + column + offset * width];
	imageOut[row * width + column + offset * width] = imageIn[row * width + column];
}																					
