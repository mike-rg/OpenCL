#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "printInfo.h"


// Imprime info de los dipositivos.
cl_uint devicesInfo(cl_device_id device)
{	
	size_t size = 0;
	char *buffer = NULL;
	cl_int status;
	cl_uint performance;

	// Nombre del dispositivo
	status = clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener nombre del dispositivo (1era llamada)\n");
		return 0;
	}
	buffer = (char*) malloc (size);
	status = clGetDeviceInfo(device, CL_DEVICE_NAME, size, buffer, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener nombre del dispositivo (2da llamada)\n");
		return 0;
	}
	printf("Nombre: %s\n", buffer);
	free(buffer); // Libero el buffer.

	// Vendedor
	status = clGetDeviceInfo(device, CL_DEVICE_VENDOR, 0, NULL, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener vendedor del dispositivo (1era llamada)\n");
		return 0;
	}
	buffer = (char*) malloc (size);
	status = clGetDeviceInfo(device, CL_DEVICE_VENDOR, size, buffer, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener vendedor del dispositivo (2da llamada)\n");
		return 0;
	}
	printf("Vendedor: %s\n", buffer);
	free(buffer);
		
	// Perfil
	status = clGetDeviceInfo(device, CL_DEVICE_PROFILE, 0, NULL, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener perfil del dispositivo (1era llamada)\n");
		return 0;
	}
	buffer = (char*) malloc (size);
	status = clGetDeviceInfo(device, CL_DEVICE_PROFILE, size, buffer, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener perfil del dispositivo (2da llamada)\n");
		return 0;
	}
	printf("Perfil: %s\n", buffer);
	free(buffer);
		
	// Tipo
	cl_device_type type;
	status = clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener tipo del dispositivo\n");
		return 0;
	}
	if (type & CL_DEVICE_TYPE_CPU)
	    printf("Tipo: CPU\n");
  	if (type & CL_DEVICE_TYPE_GPU)
	    printf("Tipo: GPU\n");
  	if (type & CL_DEVICE_TYPE_ACCELERATOR)
   		printf("Tipo: ACELERADORA\n");
	if (type & CL_DEVICE_TYPE_DEFAULT)
		printf("Tipo: DEFAULT\n");
  
	// Memoria Global (RAM)
	cl_ulong mem_size;
	status = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener memoria global del dispositivo\n");
		return 0;
	}
	printf("Memoria Global (RAM): %u MByte\n", (unsigned int)(mem_size / (1024 * 1024)));

	// Max Work-Group Size
	size_t workgroup_size;
	status = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(workgroup_size), &workgroup_size, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener size maximo de work-group\n");
		return 0;
	}
	printf("Max Work-Group Size: %u\n", workgroup_size);

	// Dimensiones Permitidas
    size_t workitem_dims;
	status = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(workitem_dims), &workitem_dims, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener dimensiones\n");
		return 0;
	}
	printf("Dimensiones Permitidas: %u\n", workitem_dims);

	// Max Work-Items por Dimension
	size_t workitem_size[3];
	status = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitem_size), &workitem_size, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener max work-items por dimension\n");
		return 0;
	}
	printf("Max Work-Items por Dimension: (%u, %u, %u)\n", workitem_size[0], workitem_size[1], workitem_size[2]);

	// Unidades de Computo
	cl_uint compute_units;
	status = clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener unidades de computo\n");
		return 0;
	}
	printf("Unidades de Computo: %u\n", compute_units);

	// Frecuencia Maxima del Reloj
	cl_uint clock_frequency;
	status = clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener frecuencia maxima del reloj\n");
		return 0;
	}
	printf("Frecuencia Maxima del Reloj: %u MHz\n", clock_frequency);

	// Rendimiento
	performance = compute_units * clock_frequency;
	printf("Rendimiento (Unidades de Computo * Frecuencia de Reloj): %u\n", performance);

	// Salto de linea entre dos dispositivos
	printf("\n");
	return performance;
}


// Imprime info de una plataforma dada.
cl_int platformsInfo(cl_platform_id platform_id)
{	
	size_t size = 0;
	cl_int status;
	// Nombre de la plataforma.
	status = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 0, NULL, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener el nombre de la plataforma (1era llamada)\n");
		return 1;
	}
	char platformName[size];
	status = clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, size, platformName, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener el nombre de la plataforma (2da llamada)\n");
		return 1;
	}
	printf("Nombre: %s\n", platformName);

	// Vendedor de la plataforma.
	status = clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, 0, NULL, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener el vendedor de la plataforma (1era llamada)\n");
		return 1;
	}
	char platformVendor[size];
	status = clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, size, platformVendor, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener el vendedor de la plataforma (2da llamada)\n");
		return 1;
	}
	printf("Vendedor: %s\n", platformVendor);

	// Version de la plataforma.
	status = clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION, 0, NULL, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener la version de la plataforma (1era llamada)\n");
		return 1;
	}
	char platformVersion[size];
	status = clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION, size, platformVersion, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener la version de la plataforma (2da llamada)\n");
		return 1;
	}
	printf("Version: %s\n", platformVersion);

	// Perfil de la plataforma.
	status = clGetPlatformInfo(platform_id, CL_PLATFORM_PROFILE, 0, NULL, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener el perfil de la plataforma (1era llamada)\n");
		return 1;
	}
	char platformProfile[size];
	status = clGetPlatformInfo(platform_id, CL_PLATFORM_PROFILE, size, platformProfile, &size);
	if (status != CL_SUCCESS)
	{
		printf("Error al obtener el perfil de la plataforma (2da llamada)\n");
		return 1;
	}
	printf("Perfil: %s\n", platformProfile);

	// Valor de retorno exitoso.
	return CL_SUCCESS;
}