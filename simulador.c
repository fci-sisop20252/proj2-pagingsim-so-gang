#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========== ESTRUTURAS DE DADOS ==========
// Cada processo tem uma tabela dessas
typedef struct {
    int frame;          // Onde está na memória física
    int presente;       // 1 se está na memória, 0 se não
    int referenciada;   // Foi acessada recentemente?
} EntradaTabela;

// Cada espaço na memória física é um desses
typedef struct {
    int pagina;         // Qual página está aqui
    int processo;       // De qual processo
    int referenciada;   // Foi acessada recentemente?
    int tempo_entrada;  // Quando entrou (para FIFO)
} FrameMemoria;

// ========== VARIÁVEIS GLOBAIS ==========
EntradaTabela **tabelas;    // Tabelas de todos os processos
FrameMemoria *memoria;      // Memória física
int num_frames;             // Quantos frames na memória
int tam_pagina;             // Tamanho de cada página
int num_processos;          // Quantos processos
int relogio = 0;            // Contador de tempo
int acessos_totais = 0;     // Total de acessos
int faults_totais = 0;      // Total de page faults
int ponteiro_clock = 0;     // Para algoritmo Clock

// ========== FUNÇÕES ==========

// Procura um frame vazio na memória
int achar_frame_livre() {
    for (int i = 0; i < num_frames; i++) {
        if (memoria[i].processo == -1) {
            return i;  // Achou frame livre!
        }
    }
    return -1;  // Memória cheia
}

// Algoritmo FIFO: remove o que está há mais tempo
int substituir_fifo() {
    int mais_antigo = relogio + 1;
    int frame_escolhido = -1;
    
    for (int i = 0; i < num_frames; i++) {
        if (memoria[i].tempo_entrada < mais_antigo) {
            mais_antigo = memoria[i].tempo_entrada;
            frame_escolhido = i;
        }
    }
    return frame_escolhido;
}

// Algoritmo Clock: dá segunda chance
int substituir_clock() {
    while (1) {
        if (memoria[ponteiro_clock].referenciada == 0) {
            // Achou vítima! Não foi referenciada recentemente
            int frame = ponteiro_clock;
            ponteiro_clock = (ponteiro_clock + 1) % num_frames;
            return frame;
        } else {
            // Dá segunda chance: zera a flag e continua
            memoria[ponteiro_clock].referenciada = 0;
            ponteiro_clock = (ponteiro_clock + 1) % num_frames;
        }
    }
}

// Trata quando a página não está na memória
void tratar_page_fault(int processo, int pagina, char *algoritmo) {
    faults_totais++;
    
    // Tenta achar frame livre
    int frame = achar_frame_livre();
    int substituiu = 0;
    int proc_antigo = -1, pag_antiga = -1;
    
    if (frame == -1) {
        // Memória cheia! Precisa substituir
        substituiu = 1;
        
        if (strcmp(algoritmo, "fifo") == 0) {
            frame = substituir_fifo();
        } else {
            frame = substituir_clock();
        }
        
        // Guarda info da página que vai sair
        proc_antigo = memoria[frame].processo;
        pag_antiga = memoria[frame].pagina;
        
        // Tira a página antiga da tabela
        tabelas[proc_antigo][pag_antiga].presente = 0;
        tabelas[proc_antigo][pag_antiga].frame = -1;
    }
    
    // Coloca a página nova na memória
    tabelas[processo][pagina].presente = 1;
    tabelas[processo][pagina].frame = frame;
    tabelas[processo][pagina].referenciada = 1;  // R-BIT = 1
    
    memoria[frame].processo = processo;
    memoria[frame].pagina = pagina;
    memoria[frame].referenciada = 1;             // R-BIT = 1
    memoria[frame].tempo_entrada = relogio;
    
    // Imprime resultado
    if (substituiu) {
        printf("Memória cheia. Página %d (PID %d) (Frame %d) será desalocada. -> ", 
               pag_antiga, proc_antigo, frame);
    }
    printf("Página %d (PID %d) alocada no Frame %d\n", pagina, processo, frame);
}

// Processa um acesso à memória
void processar_acesso(int processo, int endereco, char *algoritmo) {
    acessos_totais++;
    
    // Calcula página e deslocamento
    int pagina = endereco / tam_pagina;
    int offset = endereco % tam_pagina;
    
    printf("Acesso: PID %d, Endereço %d (Página %d, Deslocamento %d) -> ", 
           processo, endereco, pagina, offset);
    
    if (tabelas[processo][pagina].presente) {
        // HIT - Página já está na memória
        int frame = tabelas[processo][pagina].frame;
        
        // ⭐⭐ R-BIT: Marca como referenciada em TODO acesso
        tabelas[processo][pagina].referenciada = 1;
        memoria[frame].referenciada = 1;
        
        printf("HIT: Página %d (PID %d) já está no Frame %d\n", 
               pagina, processo, frame);
    } else {
        // PAGE FAULT - Página não está na memória
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
    
    // ========== LER CONFIGURAÇÃO ==========
    FILE *f = fopen(arquivo_config, "r");
    fscanf(f, "%d", &num_frames);
    fscanf(f, "%d", &tam_pagina);
    fscanf(f, "%d", &num_processos);
    
    // Alocar memória para as tabelas
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
    
    // ========== INICIALIZAR MEMÓRIA FÍSICA ==========
    memoria = malloc(num_frames * sizeof(FrameMemoria));
    for (int i = 0; i < num_frames; i++) {
        memoria[i].pagina = -1;
        memoria[i].processo = -1;
        memoria[i].referenciada = 0;
        memoria[i].tempo_entrada = -1;
    }
    
    // ========== PROCESSAR ACESSOS ==========
    f = fopen(arquivo_acessos, "r");
    int processo, endereco;
    while (fscanf(f, "%d %d", &processo, &endereco) == 2) {
        processar_acesso(processo, endereco, algoritmo);
    }
    fclose(f);
    
    // ========== ESTATÍSTICAS FINAIS ==========
    printf("--- Simulação Finalizada (Algoritmo: %s)\n", algoritmo);
    printf("Total de Acessos: %d\n", acessos_totais);
    printf("Total de Page Faults: %d\n", faults_totais);
    
    return 0;
}