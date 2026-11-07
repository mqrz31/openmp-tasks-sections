# Atividades OpenMP - Tasks e Sections

Exercicios de OpenMP em C (sections e tasks).

## Testes

GitHub Actions compila seis programas e executa os testes com 1, 2 e 4 threads, tres repeticoes por configuracao.

Para reproduzir no Linux ou WSL:

```bash
make -C codigos
python3 codigos/executar_benchmark.py --threads 1,2,4 --repeticoes 3
```

No GitHub, abra a aba Actions e selecione **Testes OpenMP - Tasks e Sections**. Abra a execucao e o job **executar-testes**. O passo `Exibir saidas completas` mostra as versoes inicial e modificada para as tres atividades.

Os resultados detalhados ficam nos artefatos `openmp-logs-e-medicoes` da execucao.
