#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "adcUtilsOpenCL.h"

/*
 * Esta funcion lee en un archivo de texto y lo almacena como un puntero char
*/
char* readSource(char* kernelPath) {

   cl_int status;
   FILE *fp;
   char *source;
   long int size;

   printf("Program file is: %s\n", kernelPath);

   fp = fopen(kernelPath, "rb");
   if(!fp) {
      printf("Could not open kernel file\n");
      exit(-1);
   }
   status = fseek(fp, 0, SEEK_END);
   if(status != 0) {
      printf("Error seeking to end of file\n");
      exit(-1);
   }
   size = ftell(fp);
   if(size < 0) {
      printf("Error getting file position\n");
      exit(-1);
   }

   rewind(fp);

   source = (char *)malloc(size + 1);

   int i;
   for (i = 0; i < size+1; i++) {
      source[i]='\0';
   }

   if(source == NULL) {
      printf("Error allocating space for the kernel source\n");
      exit(-1);
   }

   fread(source, 1, size, fp);
   source[size] = '\0';

   return source;
}

void storeImage(float *imageOut, const char *filename, int rows, int cols, 
                const char* refFilename) {

   FILE *ifp, *ofp;
   unsigned char tmp;
   int offset;
   unsigned char *buffer;
   int i, j;

   int bytes;

   int height, width;

   ifp = fopen(refFilename, "rb");
   if(ifp == NULL) {
      perror(filename);
      exit(-1);
   }

   fseek(ifp, 10, SEEK_SET);
   fread(&offset, 4, 1, ifp);

   fseek(ifp, 18, SEEK_SET);
   fread(&width, 4, 1, ifp);
   fread(&height, 4, 1, ifp);

   fseek(ifp, 0, SEEK_SET);

   buffer = (unsigned char *)malloc(offset);
   if(buffer == NULL) {
      perror("malloc");
      exit(-1);
   }

   fread(buffer, 1, offset, ifp);

   printf("Writing output image to %s\n", filename);
   ofp = fopen(filename, "wb");
   if(ofp == NULL) {
      perror("opening output file");
      exit(-1);
   }
   bytes = fwrite(buffer, 1, offset, ofp);
   if(bytes != offset) {
      printf("error writing header!\n");
      exit(-1);
   }

   // NOTA Los formatos bmp almacenan los datos en orden raster inverso
   // (ver comentario en funcion readImage), asi que tenemos que voltearla
   // al reves.
   int mod = width % 4;
   if(mod != 0) {
      mod = 4 - mod;
   }
   for(i = height-1; i >= 0; i--) {
      for(j = 0; j < width; j++) {
         tmp = (unsigned char)imageOut[i*cols+j];
         fwrite(&tmp, sizeof(char), 1, ofp);
      }
      // En el formato bmp, las filas deben ser un multiplo de 4 bytes.
      // Asi que si no estamos en un multiplo de 4, agregar relleno basura.
      for(j = 0; j < mod; j++) {
         fwrite(&tmp, sizeof(char), 1, ofp);
      }
   } 

   fclose(ofp);
   fclose(ifp);

   free(buffer);
}

/*
 * Lea la imagen bmp y la convierte en un vector. Devuelve tambien el ancho
 * y el largo.
*/
float* readImage(const char *filename, int* widthOut, int* heightOut) {

   uchar* imageData;

   int height, width;
   uchar tmp;
   int offset;
   int i, j;

   printf("Reading input image from %s\n", filename);
   FILE *fp = fopen(filename, "rb");
   if(fp == NULL) {
       perror(filename);
       exit(-1);
   }

   fseek(fp, 10, SEEK_SET);
   fread(&offset, 4, 1, fp);

   fseek(fp, 18, SEEK_SET);
   fread(&width, 4, 1, fp);
   fread(&height, 4, 1, fp);

   printf("width = %d\n", width);
   printf("height = %d\n", height);

   *widthOut = width;
   *heightOut = height;    

   imageData = (uchar*)malloc(width*height);
   if(imageData == NULL) {
       perror("malloc");
       exit(-1);
   }

   fseek(fp, offset, SEEK_SET);
   fflush(NULL);

   int mod = width % 4;
   if(mod != 0) {
       mod = 4 - mod;
   }

   // NOTA los mapas de bits se almacenan en orden raster invertido. Asi que
   // empezamos leyendo desde el pixel inferior izquierdo, yendo luego de
   // izquierda a derecha, lee de abajo hacia arriba la imagen. Para el
   // analisis de imagen, queremos que la imagen este a la derecha.

   // Primero leemos la imagen al reves

   for(i = 0; i < height; i++) {

      // agregamos la informacion actual de la imagen
      for(j = 0; j < width; j++) {
         fread(&tmp, sizeof(char), 1, fp);
         imageData[i*width + j] = tmp;
      }

      // Para el formato bmp, cada fila tiene que ser un multiplo de 4,
      // así que necesito leer en los datos basura y tirarlo
      for(j = 0; j < mod; j++) {
         fread(&tmp, sizeof(char), 1, fp);
      }
   }

   // Luego lo volteamos
   int flipRow;
   for(i = 0; i < height/2; i++) {
      flipRow = height - (i+1);
      for(j = 0; j < width; j++) {
         tmp = imageData[i*width+j];
         imageData[i*width+j] = imageData[flipRow*width+j];
         imageData[flipRow*width+j] = tmp;
      }
   }

   fclose(fp);

   // Imagen de entrada en el host
   float* floatImage = NULL;
   floatImage = (float*)malloc(sizeof(float)*width*height);
   if(floatImage == NULL) {
      perror("malloc");
      exit(-1);
   }

   // Convierte la imagen BMP a float
   for(i = 0; i < height; i++) {
      for(j = 0; j < width; j++) {
         floatImage[i*width+j] = (float)imageData[i*width+j];
      }
   }

   free(imageData);
   return floatImage;
}
