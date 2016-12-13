#ifndef __ADCUTILS__
#define __ADCUTILS__

typedef unsigned char uchar;

// Esta función lee en un archivo de texto y lo almacena como un puntero char.
// Uso:
// const char* source = readSource("rotation.cl");
// cl_program program;
// program = clCreateProgramWithSource(context, 1, &source, NULL, NULL);
char* readSource(char* kernelPath);

// Lee la imagen bmp y la convierte a un arreglo. Devuelve el acho y largo de
// la image. Uso:
// int imageHeight, imageWidth;
// const char* inputFile = "input.bmp";
// float* inputImage = readImage(inputFile, &imageWidth, &imageHeight);
// int dataSize = imageHeight*imageWidth*sizeof(float);
float* readImage(const char *filename, int* widthOut, int* heightOut);

// Escribe la imagen de bmp. Uso:
// float* outputImage = NULL;
// outputImage = (float*)malloc(dataSize);
// const char* outputFile = "output.bmp";
// storeImage(outputImage, outputFile, imageHeight, imageWidth, outputFile);
void storeImage(float *imageOut, const char *filename, int rows, int cols, const char* refFilename);

#endif
