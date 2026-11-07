#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <omp.h>

#ifndef NUM_PARTICLES
#define NUM_PARTICLES (1024LL * 1024 * 10)
#endif
#define DT 0.01
#define GRAVITY (-9.81)

typedef struct {
    double x, y, z;
    double vx, vy, vz;
} Particle;

static inline uint32_t mistura(uint32_t x) {
    x ^= x >> 16; x *= 0x7feb352du;
    x ^= x >> 15; x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}
static inline double uniforme(uint32_t i, uint32_t campo) {
    return mistura(i * 17u + campo * 1000003u + 1234567u) / 4294967295.0;
}

int main(void) {
    Particle *p = malloc((size_t)NUM_PARTICLES * sizeof(*p));
    double *vy_antiga = malloc((size_t)NUM_PARTICLES * sizeof(*vy_antiga));
    if (!p || !vy_antiga) {
        perror("malloc falhou"); free(p); free(vy_antiga); return 1;
    }
    printf("Inicializando particulas...\n");
    #pragma omp parallel for schedule(static)
    for (long long i = 0; i < NUM_PARTICLES; ++i) {
        uint32_t k = (uint32_t)i;
        p[i].x = uniforme(k, 0) * 100.0;
        p[i].y = uniforme(k, 1) * 100.0;
        p[i].z = uniforme(k, 2) * 100.0;
        p[i].vx = uniforme(k, 3) * 10.0 - 5.0;
        p[i].vy = uniforme(k, 4) * 10.0 - 5.0;
        p[i].vz = uniforme(k, 5) * 10.0 - 5.0;
    }
    printf("Particulas inicializadas.\nSimulando um passo (versao sections)...\n");
    double inicio = omp_get_wtime();

    /* A posicao usa a velocidade anterior; copiar vy evita data race. */
    #pragma omp parallel for schedule(static)
    for (long long i = 0; i < NUM_PARTICLES; ++i)
        vy_antiga[i] = p[i].vy;

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            for (long long i = 0; i < NUM_PARTICLES; ++i) {
                p[i].x += p[i].vx * DT;
                p[i].y += vy_antiga[i] * DT;
                p[i].z += p[i].vz * DT;
            }
        }
        #pragma omp section
        {
            for (long long i = 0; i < NUM_PARTICLES; ++i)
                p[i].vy += GRAVITY * DT;
        }
    }

    double tempo = omp_get_wtime() - inicio;
    printf("Simulacao concluida.\n");
    long long ids[3] = {0, NUM_PARTICLES / 2, NUM_PARTICLES - 1};
    for (int j = 0; j < 3; ++j) {
        long long i = ids[j];
        printf("Particula %lld: Pos(%.12f,%.12f,%.12f) Vel(%.12f,%.12f,%.12f)\n",
               i, p[i].x, p[i].y, p[i].z, p[i].vx, p[i].vy, p[i].vz);
    }
    printf("Tempo de execucao (simulacao): %.9f segundos\n", tempo);
    free(vy_antiga);
    free(p);
    return 0;
}
