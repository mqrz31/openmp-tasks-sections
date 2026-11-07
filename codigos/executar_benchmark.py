#!/usr/bin/env python3
"""Compila, testa 1/2/4 threads em 3 repeticoes e gera CSVs e logs."""
import argparse, csv, os, re, subprocess, tempfile, statistics
from pathlib import Path
P=Path(__file__).resolve().parent; ROOT=P.parent
ATIVIDADES={'1 - Estatistica':('analise_inicial','analise_sections'),'2 - Particulas':('simulacao_inicial','simulacao_sections'),'3 - Contagem':('contalinhas_inicial','contalinhas_tasks')}
def fixture(base):
    texto='// linha de teste para contagem OpenMP\n'*2000
    for a in range(6):
        for b in range(8):
            p=base/f'grupo_{a:02d}'/f'sub_{b:02d}';p.mkdir(parents=True)
            for n in range(12): (p/f"arquivo_{n:02d}{'.c' if n%2==0 else '.h'}").write_text(texto)
            (p/'ignorar.txt').write_text('ignorar\n')
    return 576*2000

def extrair(s,act):
    m=re.search(r'Tempo de execucao \([^)]*\): ([0-9.]+) segundos',s)
    if not m: raise RuntimeError('Tempo ausente: '+s)
    if act.startswith('1'): keys=('Min:','Max:','Soma:','Soma dos quadrados:','Media:','Desvio padrao:')
    elif act.startswith('2'):keys=('Particula ',)
    else:keys=('Total de linhas encontradas:',)
    return float(m[1]),tuple(x for x in s.splitlines() if x.startswith(keys))

def main():
    a=argparse.ArgumentParser();a.add_argument('--threads',default='1,2,4');a.add_argument('--repeticoes',type=int,default=3);opts=a.parse_args()
    ts=[int(t) for t in opts.threads.split(',')]
    assert opts.repeticoes>=1 and min(ts)>0
    (ROOT/'dados').mkdir(exist_ok=True);(ROOT/'logs').mkdir(exist_ok=True)
    subprocess.run(['make','-C',str(P),'all'],check=True,stdout=subprocess.DEVNULL)
    resultado=[];referencia={}
    with tempfile.TemporaryDirectory(prefix='fixture_openmp_') as temp:
        origem=Path(temp);esperado=fixture(origem)
        print('Fixture: 576 arquivos fonte, 48 pastas,',esperado,'linhas',flush=True)
        for t in ts:
            for rep in range(1,opts.repeticoes+1):
                for ativ,(original,novo) in ATIVIDADES.items():
                    for versao,exe in [('inicial',original),('modificada',novo)]:
                        env=os.environ.copy();env.update(OMP_NUM_THREADS=str(t),OMP_DYNAMIC='FALSE')
                        cmd=[str(P/exe)]+([str(origem)] if ativ.startswith('3') else [])
                        p=subprocess.run(cmd,capture_output=True,text=True,check=True,env=env,timeout=180)
                        tempo,observado=extrair(p.stdout,ativ)
                        if ativ.startswith('3'): assert observado==(f'Total de linhas encontradas: {esperado}',),observado
                        if ativ in referencia: assert observado==referencia[ativ],f'{ativ}: {observado} != {referencia[ativ]}'
                        else:referencia[ativ]=observado
                        rotulo=f'a{ativ[0]}_{versao}_t{t}_r{rep}'
                        (ROOT/'logs'/f'{rotulo}.txt').write_text(f'$ OMP_NUM_THREADS={t} OMP_DYNAMIC=FALSE {" ".join(cmd)}\n'+p.stdout+p.stderr)
                        resultado.append(dict(atividade=ativ,versao=versao,threads=t,repeticao=rep,tempo_segundos=f'{tempo:.9f}',validado='SIM'))
                        print(rotulo,':',f'{tempo:.6f}s','CORRETO',flush=True)
    with (ROOT/'dados'/'tempos_individuais.csv').open('w',newline='') as fp:
        w=csv.DictWriter(fp,fieldnames=resultado[0].keys());w.writeheader();w.writerows(resultado)
    resumo=[]
    for at in ATIVIDADES:
        for ver in ('inicial','modificada'):
            for t in ts:
                vs=[float(x['tempo_segundos']) for x in resultado if x['atividade']==at and x['versao']==ver and x['threads']==t]
                resumo.append(dict(atividade=at,versao=ver,threads=t,repeticoes=len(vs),media_segundos=round(statistics.mean(vs),9),desvio_amostral_segundos=round(statistics.stdev(vs),9) if len(vs)>1 else 0,min_segundos=min(vs),max_segundos=max(vs)))
    with (ROOT/'dados'/'tempos_medios.csv').open('w',newline='') as fp:
        w=csv.DictWriter(fp,fieldnames=resumo[0].keys());w.writeheader();w.writerows(resumo)
    print('CONCLUIDO: 3 atividades validadas e CSVs salvos.',flush=True)
if __name__=='__main__':main()
