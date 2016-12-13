// DEFINES
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS // Ni idea.
#define M_PI acos(-1.0) // Definicion de Pi

// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h> // Libreria de OpenCl.
#include "printInfo.h" // Funciones para info de devices y plataformas.
#include "adcUtilsOpenCL.h" // Provisto por la catedra.
#include <math.h> // Para funciones cos, sin y acos.

// CODIGO OPENCL KERNEL
const char* programSource = readSource((char*)"kernelImagen.cl");

// CODIGO HOST
int main () {
	cl_int status;
	cl_int max_performance = 0;
	
	// MODELO DE PLATAFORMAS
	cl_uint numPlatforms = 0;
	cl_platform_id *platforms = NULL;
	status = clGetPlatformIDs(0, NULL, &numPlatforms);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener el nro de plataformas\n");
		return 1;
	}
	printf("\nNUMERO DE PLATAFORMAS: %d\n", numPlatforms);
	platforms = (cl_platform_id*) malloc (numPlatforms*sizeof(cl_platform_id));
	status = clGetPlatformIDs(numPlatforms, platforms, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener las plataformas\n");
		return 1;
	}
	cl_platform_id platform_id = platforms[0];

	// INFO DE LA PLATAFORMA ELEGIDA
	printf("- Plataforma 1\n");
	status = platformsInfo(platform_id);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener informacion de la plataforma\n");
		return 1;
	}

	// MODELO DE DISPOSITIVOS (DEVICES)
	cl_uint numDevices = 0;
	cl_device_id *devices = NULL;
	status = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener el nro de dispositivos\n");
		return 1;
	}
	printf("\nNUMERO DE DISPOSITIVOS: %d\n", numDevices);
	devices = (cl_device_id*) malloc (numDevices*sizeof(cl_device_id));
	status = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener los dispositivos\n");
		return 1;
	}

	// INFO DE LOS DISPOSITIVOS
	cl_uint performance = 0;
	cl_int index; // Util solo para el print de device elegido.
	cl_device_id device_id;

	// PERFORMANCE DEL DISPOSITIVO
	for (int i = 0; i < numDevices; i++)
	{
		printf("- Dispositivo %d\n", i+1);
		performance = devicesInfo(devices[i]);
		if (performance > max_performance)
		{
			max_performance = performance;
			device_id = devices[i];
			index = i+1;
		}
		if (performance == 0)
		{
			printf("Error al obtener informacion del dispositivo: %d\n", i);
			return 1;
		}
	}
	printf("DISPOSITIVO %d SELECCIONADO (Mejor Rendimiento)\n", index);

	// CONTEXTO
	cl_context context;
	context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
	if (status != CL_SUCCESS)
	{
		printf("Error al crear el contexto\n");
		return 1;
	}

	// COLA DE COMANDOS
	cl_command_queue cmdQueue;
	cmdQueue = clCreateCommandQueue(context, device_id, 0, &status);
	if (status != CL_SUCCESS)
	{
		printf("Error al crear la cola de comandos\n");
		return 1;
	}

	// LECTURA DE INPUT
	int width;
	int height;
	float *imageIn = NULL;
	const char *inputFilename = "input.bmp";
	imageIn = readImage(inputFilename, &width, &height);
	size_t datasize = width * height * sizeof(float);

	// ARREGLO QUE REPRESENTA UNA IMAGEN PROCESADA
	float *imageOut = NULL;
    imageOut = (float*)malloc(sizeof(float)*width*height);

    // INTERACCION CON EL USUARIO
	char r, h, v; // Terna de procesos.
	// Terna de parametros de rotacion.
	float degrees;
	int x0, y0;
	bool flag_process = 1;
   	while (flag_process) {
	    printf("Desea rotar la imagen? (Y/N)\n");
   		scanf(" %c", &r);
    	if (r == 'Y')
    	{
	    	printf("Ingrese el angulo de rotacion: ");
	    	scanf("%f",&degrees);
	    	printf("Ingrese el origen X de la rotacion: ");
	    	scanf("%d",&x0);
	    	printf("Ingrese el origen Y de la rotacion: ");
	    	scanf("%d",&y0);
    		flag_process = 0;
    	} else if (r == 'N')
    			flag_process = 0;
    }
    flag_process = 1;
   	while (flag_process) {
	    printf("Desea espejar horizontalmente? (Y/N)\n");
   		scanf(" %c", &h);
    	if (h == 'Y' || h == 'N')
    		flag_process = 0;
    }
    flag_process = 1;
   	while (flag_process) {
	    printf("Desea espejar verticalmente? (Y/N)\n");
   		scanf(" %c", &v);
    	if (v == 'Y' || v == 'N')
    		flag_process = 0;
    }

    // FLAGS DE PROCESOS QUE VAN A EJECUTARSE
	bool rotationProcess = r == 'Y'; 
	bool horizontalProcess = h == 'Y';
	bool verticalProcess = v == 'Y';

	// CALCULOS AUXILIARES PARA LOS ANGULOS
	float radians = M_PI * degrees / 180;
	float cosTheta = cos(radians);
    float sinTheta = sin(radians);

	// CREACION DE BUFFERS
	cl_mem bufferIn;
	cl_mem bufferOut;
	cl_float empty = 0; // Para llenar el buffer de salida con ceros (Limpiarlo);
	bufferIn = clCreateBuffer(context, CL_MEM_READ_WRITE, datasize, NULL, &status);
	if (status != CL_SUCCESS)
	{
		printf("Error al crear el buffer de entrada\n");
		return 1;
	}
	bufferOut = clCreateBuffer(context, CL_MEM_READ_WRITE, datasize, NULL, &status);
	if (status != CL_SUCCESS)
	{
		printf("Error al crear el buffer de salida\n");
		return 1;
	}

	// LIMPIA EL BUFFER DE SALIDA POR SI TIENE DATOS BASURA
	status = clEnqueueFillBuffer(cmdQueue, bufferOut, &empty, sizeof(cl_float), 0, datasize, 0, NULL, NULL);


	// ESCRITURA IMAGEN DE ENTRADA -> BUFFER ENTRADA
	status = clEnqueueWriteBuffer(cmdQueue, bufferIn, CL_TRUE, 0, datasize, imageIn, 0, NULL, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al escribir en el buffer de entrada\n");
		return 1;
	}

	// PROGRAMA A PARTIR DEL CODIGO FUENTE
	cl_program program = clCreateProgramWithSource(context, 1, (const char**)&programSource, NULL, &status);
	if (status != CL_SUCCESS)
	{
		printf("Error al crear el programa\n");
		return 1;
	}
	status = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al compilar el programa\n");
		size_t len = 0;  
	    char *buildInfo = NULL; // Buffer con la informacion de compilacion.
	    status = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
		if (status != CL_SUCCESS)
		{
			printf("Error al obtener informacion de compilacion (1era llamada)\n");
			return 1;
		}
		buildInfo = (char*) malloc (len);
	    status = clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, len, buildInfo, NULL);
	   	if (status != CL_SUCCESS)
		{
			printf("Error al obtener informacion de compilacion (2da llamada)\n");
			return 1;
		}
		printf("Build Log:\n");
	   	printf("%s\n", buildInfo);
	   	free(buildInfo);
		return 1;
	}

	// ESPACIOS Y DIMENSIONES PARA ESPEJADO HORIZONTAL
	size_t horizontalGlobalWorkSize[2];
	horizontalGlobalWorkSize[0] = width / 2;
	horizontalGlobalWorkSize[1] = height;

	// ESPACIOS Y DIMENSIONES PARA ESPEJADO VERTICAL
	size_t verticalGlobalWorkSize[2];
	verticalGlobalWorkSize[0] = width;
	verticalGlobalWorkSize[1] = height / 2;

	// ESPACIOS Y DIMENSIONES PARA ROTACION
	size_t rotationGlobalWorksize[2];
	rotationGlobalWorksize[0] = width;
	rotationGlobalWorksize[1] = height;

	// KERNEL PARA ESPEJADO HORIZONTAL
	cl_kernel horizontal;
	horizontal = clCreateKernel(program, "horizontal", &status);
	status = clSetKernelArg(horizontal, 0, sizeof(cl_mem), &bufferIn);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear primer argumento kernel horizontal\n");
		return 1;
	}
	status = clSetKernelArg(horizontal, 1, sizeof(cl_mem), &bufferOut);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear segundo argumento kernel horizontal\n");
		return 1;
	}
	status = clSetKernelArg(horizontal, 2, sizeof(cl_int), &width);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear tercer argumento kernel horizontal\n");
		return 1;
	}

	// KERNEL PARA ESPEJADO VERTICAL
	cl_kernel vertical;
	vertical = clCreateKernel(program, "vertical", &status);
	status = clSetKernelArg(vertical, 0, sizeof(cl_mem), &bufferIn);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear primer argumento kernel vertical\n");
		return 1;
	}
	status = clSetKernelArg(vertical, 1, sizeof(cl_mem), &bufferOut);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear segundo argumento kernel vertical\n");
		return 1;
	}
	status = clSetKernelArg(vertical, 2, sizeof(cl_int), &width);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear tercer argumento kernel vertical\n");
		return 1;
	}
	status = clSetKernelArg(vertical, 3, sizeof(cl_int), &height);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear cuarto argumento kernel vertical\n");
		return 1;
	}

	// KERNEL PARA ROTACION
	cl_kernel rotation;
	rotation = clCreateKernel(program, "rotation", &status);
	status = clSetKernelArg(rotation, 0, sizeof(cl_mem), &bufferIn);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear primer argumento kernel rotacion\n");
		return 1;
	}
    status = clSetKernelArg(rotation, 1, sizeof(cl_mem), &bufferOut);
    if (status != CL_SUCCESS)
    {
        printf("Error al setear segundo argumento kernel rotacion\n");
        return 1;
    }
	status = clSetKernelArg(rotation, 2, sizeof(cl_int), &width);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear tercer argumento kernel rotacion\n");
		return 1;
	}
    status = clSetKernelArg(rotation, 3, sizeof(cl_int), &height);
    if (status != CL_SUCCESS)
    {
        printf("Error al setear cuarto argumento kernel rotacion\n");
        return 1;
    }
    status = clSetKernelArg(rotation, 4, sizeof(cl_int), &x0);
    if (status != CL_SUCCESS)
    {
        printf("Error al setear quinto argumento kernel rotacion\n");
        return 1;
    }
    status = clSetKernelArg(rotation, 5, sizeof(cl_int), &y0);
    if (status != CL_SUCCESS)
    {
        printf("Error al setear sexto argumento kernel rotacion\n");
        return 1;
    }
    status = clSetKernelArg(rotation, 6, sizeof(cl_float), &sinTheta);
    if (status != CL_SUCCESS)
    {
        printf("Error al setear septimo argumento kernel rotacion\n");
        return 1;
    }
    status = clSetKernelArg(rotation, 7, sizeof(cl_float), &cosTheta);
    if (status != CL_SUCCESS)
    {
        printf("Error al setear octavo argumento kernel rotacion\n");
        return 1;
    }

    // SI PIDIERON QUE SE EJECUTE ROTACION

    if (rotationProcess) {
		status = clEnqueueNDRangeKernel(cmdQueue, rotation, 2, NULL, rotationGlobalWorksize, NULL, 0, NULL, NULL);
		if (status != CL_SUCCESS)
		{
			printf("Error al ejecutar kernel de rotacion\n");
			return 1;
		}

		// COPIA BUFFER DE SALIDA AL ARREGLO DE SALIDA
		status = clEnqueueReadBuffer(cmdQueue, bufferOut, CL_TRUE, 0, datasize, imageOut, 0, NULL, NULL);
		if (status != CL_SUCCESS)
		{
			printf("Error al leer buffer de salida en arreglo de salida (rotacion)\n");
			return 1;
		}

		// COPIA BUFFER DE SALIDA AL BUFFER DE ENTRADA PARA ALIMENTAR A LOS OTROS PROCESOS
		status = clEnqueueCopyBuffer(cmdQueue, bufferOut, bufferIn, 0, 0, datasize, 0, NULL, NULL);

		// LIMPIA EL BUFFER DE SALIDA POR SI TIENE DATOS BASURA DE PROCESOS PREVIOS
		status = clEnqueueFillBuffer(cmdQueue, bufferOut, &empty, sizeof(cl_float), 0, datasize, 0, NULL, NULL);
	}

    // SI PIDIERON QUE SE EJECUTE HORIZONTAL
    if (horizontalProcess) {
		status = clEnqueueNDRangeKernel(cmdQueue, horizontal, 2, NULL, horizontalGlobalWorkSize, NULL, 0, NULL, NULL);
		if (status != CL_SUCCESS)
		{
			printf("Error al ejecutar kernel de espejado horizontal\n");
			return 1;
		}
		// COPIA BUFFER DE SALIDA AL ARREGLO DE SALIDA
		status = clEnqueueReadBuffer(cmdQueue, bufferOut, CL_TRUE, 0, datasize, imageOut, 0, NULL, NULL);
		if (status != CL_SUCCESS)
		{
			printf("Error al leer buffer de salida en arreglo de salida (horizontal)\n");
			return 1;
		}
		// COPIA BUFFER DE SALIDA AL BUFFER DE ENTRADA PARA ALIMENTAR A LOS OTROS PROCESOS
		status = clEnqueueCopyBuffer(cmdQueue, bufferOut, bufferIn, 0, 0, datasize, 0, NULL, NULL);
		// LIMPIA EL BUFFER DE SALIDA POR SI TIENE DATOS BASURA DE PROCESOS PREVIOS
		status = clEnqueueFillBuffer(cmdQueue, bufferOut, &empty, sizeof(cl_float), 0, datasize, 0, NULL, NULL);
	}

    // SI PIDIERON QUE SE EJECUTE VERTICAL
    if (verticalProcess) {
		status = clEnqueueNDRangeKernel(cmdQueue, vertical, 2, NULL, verticalGlobalWorkSize, NULL, 0, NULL, NULL);
		if (status != CL_SUCCESS)
		{
			printf("Error al ejecutar kernel de espejado vertical\n");
			return 1;
		}
		// COPIA BUFFER DE SALIDA AL ARREGLO DE SALIDA
		status = clEnqueueReadBuffer(cmdQueue, bufferOut, CL_TRUE, 0, datasize, imageOut, 0, NULL, NULL);
		if (status != CL_SUCCESS)
		{
			printf("Error al leer buffer de salida en arreglo de salida (vertical)\n");
			return 1;
		}
	}

	// RECONSTRUCCION DE LA IMAGEN BMP A PARTIR DEL ARREGLO DE SALIDA
	storeImage(imageOut, "out.bmp", height, width, inputFilename);

	// LIBERACION DE RECURSOS

	clReleaseDevice(device_id);
    clReleaseKernel(rotation);
    clReleaseKernel(horizontal);
    clReleaseKernel(vertical);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufferIn);
    clReleaseMemObject(bufferOut);
    clReleaseContext(context);

	// LIBERACION DE MEMORIA

    free(platforms);
    free(devices);
    free(imageIn);
    free(imageOut);

   	return 0;
}
