#include "contalinhas_utils.h"

static long long traverse_directory(const char *dirpath) {
    struct dirent **nomes = NULL;
    int n = scandir(dirpath, &nomes, NULL, alphasort);
    if (n < 0) {
        fprintf(stderr, "Erro ao escanear %s: %s\n", dirpath, strerror(errno));
        return 0;
    }
    long long total_lines = 0;
    for (int i = 0; i < n; ++i) {
        const char *nome = nomes[i]->d_name;
        if (strcmp(nome, ".") && strcmp(nome, "..")) {
            char *caminho = join_path(dirpath, nome);
            if (!caminho) { fprintf(stderr, "Sem memoria para caminho\n"); free(nomes[i]); continue; }
            struct stat st;
            /* lstat ignora links simbolicos, evitando loops por diretorios apontados. */
            if (lstat(caminho, &st) == 0) {
                if (S_ISDIR(st.st_mode))
                    total_lines += traverse_directory(caminho);
                else if (S_ISREG(st.st_mode) && is_source_file(nome))
                    total_lines += count_lines_in_file(caminho);
            }
            free(caminho);
        }
        free(nomes[i]);
    }
    free(nomes);
    return total_lines;
}

int main(int argc, char **argv) {
    const char *inicio = argc > 1 ? argv[1] : ".";
    printf("Contando linhas a partir de: %s (sequencial)\n", inicio);
    double t0 = omp_get_wtime();
    long long total = traverse_directory(inicio);
    double dt = omp_get_wtime() - t0;
    printf("--- Resultado ---\nTotal de linhas encontradas: %lld\n", total);
    printf("Tempo de execucao (contagem): %.9f segundos\n", dt);
    return 0;
}
