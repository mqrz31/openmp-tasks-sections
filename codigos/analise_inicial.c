#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <omp.h>

#ifndef N
#define N (1024LL * 1024 * 100)
#endif

int main(void) {
    int *data = malloc((size_t)N * sizeof(*data));
    if (!data) { perror("malloc falhou"); return 1; }

    printf("Inicializando dados...\n");
    #pragma omp parallel for schedule(static)
    for (long long i = 0; i < N; ++i)
        data[i] = (int)(i % 1000) - 500;
    printf("Dados inicializados.\n");

    int min_val = INT_MAX, max_val = INT_MIN;
    long long sum = 0, sum_sq = 0;
    printf("Calculando metricas (versao inicial)...\n");
    double inicio = omp_get_wtime();

    #pragma omp parallel for reduction(min:min_val) schedule(static)
    for (long long i = 0; i < N; ++i)
        if (data[i] < min_val) min_val = data[i];

    #pragma omp parallel for reduction(max:max_val) schedule(static)
    for (long long i = 0; i < N; ++i)
        if (data[i] > max_val) max_val = data[i];

    #pragma omp parallel for reduction(+:sum,sum_sq) schedule(static)
    for (long long i = 0; i < N; ++i) {
        sum += data[i];
        sum_sq += (long long)data[i] * data[i];
    }

    double tempo = omp_get_wtime() - inicio;
    double media = (double)sum / N;
    double variancia = (double)sum_sq / N - media * media;
    double desvio = sqrt(variancia);

    printf("--- Resultados ---\n");
    printf("Min: %d\nMax: %d\nSoma: %lld\nSoma dos quadrados: %lld\n", min_val, max_val, sum, sum_sq);
    printf("Media: %.10f\nDesvio padrao: %.10f\n", media, desvio);
    printf("Tempo de execucao (calculo): %.9f segundos\n", tempo);
    free(data);
    return 0;
}
