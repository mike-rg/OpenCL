__kernel																			
void temperature (__global double *T,											
				   int N, int K,												
				   __local double *TPrima)										
{																				
																				
	int column = get_global_id(0);												
	int row = get_global_id(1);													
	int neighbors;																
	double sum = 0;	// Suma de temperaturas	de los vecinos						
	if (!T[row * N + column]) { // Si no es una fuente							
		for (int iter = 0; iter < K; iter ++) {									
			neighbors = 0; // Num de vecinos									
			if (column > 0) { // Tiene un vecino a su izquierda					
				sum += T[row * N + column - 1]; 								
				neighbors += 1;													
			}																	
			if (column < N - 1) { // Tiene un vecino a su derecha				
				sum += T[row * N + column + 1]; 								
				neighbors += 1;													
			}																	
			if (row > 0) { // Tiene un vecino arriba							
				sum += T[(row - 1) * N + column]; 								
				neighbors += 1;													
			}																	
			if (row < N - 1) { // Tiene un vecino debajo						
				sum += T[(row + 1) * N + column]; 								
				neighbors += 1;													
			}																	
			// Calcula el nuevo valor de temperatura en TPrima					
			TPrima[row * N + column] = (T[row * N + column] + sum)/(1 + neighbors);
			barrier(CLK_GLOBAL_MEM_FENCE);	// Barrera	de sincronizacion		
			// Actualiza T 							 							
			T[row * N + column] = TPrima[row * N + column];						
		}																		
	}																			
	else																		
		barrier(CLK_GLOBAL_MEM_FENCE);											
}																				
