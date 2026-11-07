#include "contalinhas_utils.h"

/* Todas as tasks somam em total compartilhado, protegidas por atomic.
   Cada task recebe copia privada do ponteiro caminho e libera sua memoria. */
static void traverse_directory_tasks(const char *dirpath, long long *total) {
    struct dirent **nomes = NULL;
    int n = scandir(dirpath, &nomes, NULL, alphasort);
    if (n < 0) {
        fprintf(stderr, "Erro ao escanear %s: %s\n", dirpath, strerror(errno));
        return;
    }
    for (int i = 0; i < n; ++i) {
        const char *nome = nomes[i]->d_name;
        if (strcmp(nome, ".") && strcmp(nome, "..")) {
            char *caminho = join_path(dirpath, nome);
            if (!caminho) { fprintf(stderr, "Sem memoria para caminho\n"); free(nomes[i]); continue; }
            struct stat st;
            if (lstat(caminho, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    #pragma omp task firstprivate(caminho, total)
                    {
                        traverse_directory_tasks(caminho, total);
                        free(caminho);
                    }
                } else if (S_ISREG(st.st_mode) && is_source_file(nome)) {
                    #pragma omp task firstprivate(caminho, total)
                    {
                        long long quantidade = count_lines_in_file(caminho);
                        #pragma omp atomic update
                        *total += quantidade;
                        free(caminho);
                    }
                } else {
                    free(caminho);
                }
            } else {
                free(caminho);
            }
        }
        free(nomes[i]);
    }
    free(nomes);
    /* A barreira final da regiao parallel espera todas as tasks descendentes. */
}

int main(int argc, char **argv) {
    const char *inicio = argc > 1 ? argv[1] : ".";
    printf("Contando linhas a partir de: %s (OpenMP tasks)\n", inicio);
    long long total = 0;
    double t0 = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp single
        {
            traverse_directory_tasks(inicio, &total);
        }
    }
    double dt = omp_get_wtime() - t0;
    printf("--- Resultado ---\nTotal de linhas encontradas: %lld\n", total);
    printf("Tempo de execucao (contagem): %.9f segundos\n", dt);
    return 0;
}
