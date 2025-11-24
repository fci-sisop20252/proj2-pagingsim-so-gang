#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int frame;
    int presente;
    int referenciada;
} EntradaTabela;

typedef struct {
    int pagina;
    int processo;
    int referenciada;
    int tempo_entrada;
} FrameMemoria;

EntradaTabela **tabelas;
FrameMemoria *memoria;
int num_frames;
int tam_pagina;
int num_processos;
int relogio = 0;
int acessos_totais = 0;
int faults_totais = 0;
int ponteiro_clock = 0;

int achar_frame_livre() {
    for (int i = 0; i < num_frames; i++) {
        if (memoria[i].processo == -1)
            return i;
    }
    return -1;
}

int substituir_fifo() {
    int frame_escolhido = -1;
    int mais_antigo = 99999999;

    for (int i = 0; i < num_frames; i++) {
        if (memoria[i].processo != -1 && memoria[i].tempo_entrada < mais_antigo) {
            mais_antigo = memoria[i].tempo_entrada;
            frame_escolhido = i;
        }
    }
    return frame_escolhido;
}

int substituir_clock() {
    while (1) {
        if (memoria[ponteiro_clock].referenciada == 0) {
            int frame = ponteiro_clock;
            ponteiro_clock = (ponteiro_clock + 1) % num_frames;
            return frame;
        } else {
            memoria[ponteiro_clock].referenciada = 0;
            ponteiro_clock = (ponteiro_clock + 1) % num_frames;
        }
    }
}

void tratar_page_fault(int processo, int pagina, char *algoritmo) {
    faults_totais++;

    int frame = achar_frame_livre();
    int substituiu = 0;
    int proc_antigo = -1, pag_antiga = -1;

    if (frame == -1) {
        substituiu = 1;
        if (strcmp(algoritmo, "fifo") == 0) {
            frame = substituir_fifo();
        } else {
            frame = substituir_clock();
        }
        proc_antigo = memoria[frame].processo;
        pag_antiga = memoria[frame].pagina;
        tabelas[proc_antigo][pag_antiga].presente = 0;
    }

    tabelas[processo][pagina].presente = 1;
    tabelas[processo][pagina].frame = frame;
    tabelas[processo][pagina].referenciada = 1;

    memoria[frame].processo = processo;
    memoria[frame].pagina = pagina;
    memoria[frame].referenciada = 1;
    memoria[frame].tempo_entrada = relogio;

    if (substituiu) {
        printf("Memória cheia. Página %d (PID %d) (Frame %d) será desalocada. -> Página %d (PID %d) alocada no Frame %d\n", pag_antiga, proc_antigo, frame, pagina, processo, frame);
    } else {
        printf("Página %d (PID %d) alocada no Frame livre %d\n", pagina, processo, frame);
    }
}

void processar_acesso(int processo, int endereco, char *algoritmo) {
    acessos_totais++;
    int pagina = endereco / tam_pagina;
    int offset = endereco % tam_pagina;

    printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> ", processo, endereco, pagina, offset);

    if (tabelas[processo][pagina].presente) {
        int frame = tabelas[processo][pagina].frame;
        tabelas[processo][pagina].referenciada = 1;
        memoria[frame].referenciada = 1;
        printf("HIT: Página %d (PID %d) já está no Frame %d\n", pagina, processo, frame);
    } else {
        printf("PAGE FAULT -> ");
        tratar_page_fault(processo, pagina, algoritmo);
    }
    relogio++;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: ./simulador fifo config.txt acessos.txt\n");
        return 1;
    }

    char *algoritmo = argv[1];
    char *arquivo_config = argv[2];
    char *arquivo_acessos = argv[3];

    FILE *f = fopen(arquivo_config, "r");
    fscanf(f, "%d", &num_frames);
    fscanf(f, "%d", &tam_pagina);
    fscanf(f, "%d", &num_processos);

    tabelas = malloc(num_processos * sizeof(EntradaTabela*));
    for (int i = 0; i < num_processos; i++) {
        int pid, tamanho;
        fscanf(f, "%d %d", &pid, &tamanho);
        int num_paginas = (tamanho + tam_pagina - 1) / tam_pagina;
        tabelas[pid] = malloc(num_paginas * sizeof(EntradaTabela));
        for (int j = 0; j < num_paginas; j++) {
            tabelas[pid][j].frame = -1;
            tabelas[pid][j].presente = 0;
            tabelas[pid][j].referenciada = 0;
        }
    }
    fclose(f);

    memoria = malloc(num_frames * sizeof(FrameMemoria));
    for (int i = 0; i < num_frames; i++) {
        memoria[i].pagina = -1;
        memoria[i].processo = -1;
        memoria[i].referenciada = 0;
        memoria[i].tempo_entrada = -1;
    }

    f = fopen(arquivo_acessos, "r");
    int processo, endereco;
    while (fscanf(f, "%d %d", &processo, &endereco) == 2) {
        processar_acesso(processo, endereco, algoritmo);
    }
    fclose(f);

    printf("--- Simulação Finalizada (Algoritmo: %s)\n", algoritmo);
    printf("Total de Acessos: %d\n", acessos_totais);
    printf("Total de Page Faults: %d\n", faults_totais);

    return 0;
}
