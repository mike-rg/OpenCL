#ifndef __PRINTINFO__
#define __PRINTINFO__


// La siguiente funcion imprime por pantalla informacion sobre una plataforma
// previamente elegida.
// Dicha informacion incluye Nombre, Vendedor, Version y Perfil.
cl_int platformsInfo(cl_platform_id platform_id);



// Dada una plataforma, la siguiente funcion imprime por pantalla informacion
// sobre cada uno de los dispositivos de la misma.
// Esta info incluye Nombre, Vendedor, Perfil, Tipo, Ram, Cache, Clock-Frecuency
// Performance, etc.
cl_uint devicesInfo(cl_device_id device);

#endif
