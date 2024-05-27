#include <mpi.h>
#include <stdio.h>

// cd ../ejer05
// cd ejer05
// mpiexec -n 8 matrizMPI.exe

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int ip, np;
    MPI_Comm_rank(MPI_COMM_WORLD, &ip);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    int n = 20;
    double m[n][n];  
    double v[n];     
    double r[n];

    // Inicialización de la matriz y el vector en el proceso raíz (ip = 0)
    if (ip == 0) {
        for (int i = 0; i < n; i++) {
            v[i] = 2; //i + 1;
            for (int j = 0; j < n; j++) {
                m[i][j] = 2; //i * n + j + 1;  // Ejemplo de matriz
            }
        }
    }

    // Compartir el vector v a todos los procesos
    MPI_Bcast(v, n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Determinar el número de filas que cada proceso manejará
    int nro_filas = n / np;
    int sobrande_filas = n % np;

    // Procesos adicionales toman una fila extra si hay un resto
    int nro_filas_actual = nro_filas + (ip < sobrande_filas ? 1 : 0);

    // Preparar submatrices y subresultados para cada proceso
    double ml[nro_filas_actual][n];
    double rl[nro_filas_actual];

    // Crear los arreglos de desplazamientos y cuentas para MPI_Scatterv
    int nro_elem_env[np]; // Número de elementos a enviar a cada proceso
    int pos_env[np];     // Desplazamientos en el buffer de envío

    // Crear los arreglos de desplazamientos y cuentas para MPI_Gatherv
    int nro_elem_rec[np]; // Número de elementos a recibir de cada proceso
    int pos_rec[np]; // Desplazamientos en el buffer de recepción

    //Calculo de desplazamientos y cuentas
    int sum_env = 0,sum_rec = 0;;
    for (int i = 0; i < np; i++) {

        // para MPI_Scatterv 
        nro_elem_env[i] = (nro_filas + (i < sobrande_filas ? 1 : 0)) * n;
        pos_env[i] = sum_env;
        sum_env += nro_elem_env[i];
        
        //para MPI_Gatherv
        nro_elem_rec[i] = nro_filas + (i < sobrande_filas ? 1 : 0);
        pos_rec[i] = sum_rec;
        sum_rec += nro_elem_rec[i];
    }

    // Scatter de la matriz
    MPI_Scatterv(&(m[0][0]), nro_elem_env, pos_env, MPI_DOUBLE, &(ml[0][0]), nro_filas_actual * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // Cálculo parcial del vector resultante
    for (int i = 0; i < nro_filas_actual; i++) {
        rl[i] = 0.0;
        for (int j = 0; j < n; j++) {
            rl[i] += ml[i][j] * v[j];
        }
    }

    // Gather del resultado parcial
    MPI_Gatherv(rl, nro_filas_actual, MPI_DOUBLE, r, nro_elem_rec, pos_rec, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // El proceso raíz imprime el resultado
    if (ip == 0) {
        printf("Resultado de A * v:\n");
        for (int i = 0; i < n; i++) {
            printf("%f\n", r[i]);
        }
    }

    MPI_Finalize();
    return 0;
}
