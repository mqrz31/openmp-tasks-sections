CC = gcc
CFLAGS = -O3 -fopenmp -std=c11 -Wall -Wextra -Wpedantic
EXES = analise_inicial analise_sections simulacao_inicial simulacao_sections contalinhas_inicial contalinhas_tasks
all: $(EXES)
analise_inicial: analise_inicial.c
	$(CC) $(CFLAGS) $< -o $@ -lm
analise_sections: analise_sections.c
	$(CC) $(CFLAGS) $< -o $@ -lm
simulacao_inicial: simulacao_inicial.c
	$(CC) $(CFLAGS) $< -o $@
simulacao_sections: simulacao_sections.c
	$(CC) $(CFLAGS) $< -o $@
contalinhas_inicial: contalinhas_inicial.c contalinhas_utils.h
	$(CC) $(CFLAGS) $< -o $@
contalinhas_tasks: contalinhas_tasks.c contalinhas_utils.h
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -f $(EXES)
.PHONY: all clean
