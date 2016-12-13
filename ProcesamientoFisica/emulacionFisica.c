// DEFINES
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS // Ni idea.

// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h> // Libreria de OpenCl.
#include "printInfo.h" // Funciones para info de devices y plataformas.
#include "adcUtilsOpenCL.h" // Provisto por la catedra.

// CODIGO OPENCL KERNEL
const char* programSource = readSource((char*)"kernelFisica.cl");

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

	// INTERACCION CON EL USUARIO
	int N = 0;
	int K = 0;
	int J = 0;
	int x, y = 0;
	printf("Ingrese el numero de filas y columnas de la matriz: ");
	scanf("%d", &N);
	double *T = (double*)calloc(N*N,sizeof(double));
	size_t datasize = N * N * sizeof(double);
	printf("Ingrese el numero de iteraciones deseadas: ");
	scanf("%d", &K);
	printf("Ingrese el numero de fuentes de calor: ");
	scanf("%d", &J);
	for (int i = 0; i < J; i++) {
		printf("Ingrese la posicion en x de la fuente %d: ", i+1);
		scanf("%d", &x);
		printf("Ingrese la posicion en y de la fuente %d: ", i+1);
		scanf("%d", &y);
		printf("Ingrese la temperatura de la fuente %d: ", i+1);
		scanf("%lf", &T[(y-1) * N + x-1]);
		
	}


	// CREACION DE BUFFER T
	cl_mem bufferT;
	bufferT = clCreateBuffer(context, CL_MEM_READ_WRITE, datasize, NULL, &status);
	if (status != CL_SUCCESS)
	{
		printf("Error al crear el buffer de entrada\n");
		return 1;
	}
	// ESCRITURA MATRIZ T -> BUFFER T
	status = clEnqueueWriteBuffer(cmdQueue, bufferT, CL_TRUE, 0, datasize, T, 0, NULL, NULL);
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

	// ESPACIO Y DIMENSIONES GLOBALES
	size_t temperatureGlobalWorksize[2];
	temperatureGlobalWorksize[0] = N;
	temperatureGlobalWorksize[1] = N;

	// ESPACIO Y DIMENSIONES LOCALES (WORKGROUP)
	size_t temperatureLocalWorksize[2];
	temperatureLocalWorksize[0] = N;
	temperatureLocalWorksize[1] = N;

	// CREACION Y COMPILACION DEL KERNEL
	cl_kernel temperature;
	temperature = clCreateKernel(program, "temperature", &status);
	status = clSetKernelArg(temperature, 0, sizeof(cl_mem), &bufferT);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear primer argumento del kernel\n");
		return 1;
	}
	status = clSetKernelArg(temperature, 1, sizeof(cl_int), &N);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear segundo argumento del kernel\n");
		return 1;
	}
	status = clSetKernelArg(temperature, 2, sizeof(cl_int), &K);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear tercer argumento del kernel\n");
		return 1;
	}
	status = clSetKernelArg(temperature, 3, datasize, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al setear cuarto argumento del kernel\n");
		return 1;
	}


	// EJECUCION DEL KERNEL
	status = clEnqueueNDRangeKernel(cmdQueue, temperature, 2, NULL, temperatureGlobalWorksize, temperatureLocalWorksize, 0, NULL, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al ejecutar kernel\n");
		return 1;
	}

	// LECTURA BUFFER T -> MATRIZ T
	status = clEnqueueReadBuffer(cmdQueue, bufferT, CL_TRUE, 0, datasize, T, 0, NULL, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al leer buffer T\n");
		return 1;
	}

	// PRINT DE MATRIZ T
	printf("Matriz resultante luego de %d iteraciones\n", K);
	for (int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++)
			printf("[%0.2lf] ", T[i * N + j]);
		printf("\n");
	}

	// LIBERACION DE RECURSOS
	clReleaseDevice(device_id);
    clReleaseKernel(temperature);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufferT);
    clReleaseContext(context);

	// LIBERACION DE MEMORIA
    free(platforms);
    free(devices);
    free(T);

   	return 0;
}
