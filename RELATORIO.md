# Relatório do Projeto 2: Simulador de Memória Virtual

**Disciplina:** Sistemas Operacionais
**Professor:** Lucas Figueiredo
**Data:**

## Integrantes do Grupo

- Guilherme Araujo Castro - 10427775
- Rafael de Souza Alves de Lima - 10425819
- Caio Henrique Santos Carvalho - 10425408
- Nome Completo - Matrícula

---

## 1. Instruções de Compilação e Execução

### 1.1 Compilação

Descreva EXATAMENTE como compilar seu projeto. Inclua todos os comandos necessários.

Para compilar o projeto, execute o seguinte comando:

```bash
gcc -o simulador simulador.c
```

### 1.2 Execução

Forneça exemplos completos de como executar o simulador.

Para executar o simulador, use a seguinte sintaxe:

```bash
./simulador <algoritmo> <arquivo_config> <arquivo_acessos>
```
Exemplos:

```bash
# Executar com algoritmo FIFO
./simulador fifo tests/config_1.txt tests/acessos_1.txt

# Executar com algoritmo Clock
./simulador clock tests/config_2.txt tests/acessos_2.txt

# Executar teste próprio
./simulador fifo teste_proprio_config.txt teste_proprio_acessos.txt
```

## 2. Decisões de Design

### 2.1 Estruturas de Dados

**Tabela de Páginas:**

Estrutura: Array de structs para cada processo

Informações: frame, bit de presente, bit de referência (R-bit)

Organização: Array 2D - tabelas[processo][página]

Justificativa: Acesso direto por índice é eficiente e simples de implementar

**Frames Físicos:**

Estrutura: Array de structs FrameMemoria

Informações: página, processo, R-bit, tempo de entrada

Rastreamento: Processo = -1 indica frame livre

Justificativa: Representação direta da memória física com todas as informações necessárias

**Estrutura para FIFO:**

Ordem de chegada: Cada frame armazena tempo_entrada

Identificação: Busca o frame com menor tempo_entrada

Justificativa: Simples e eficiente - não precisa de fila separada

**Estrutura para Clock:**

Ponteiro circular: Variável ponteiro_clock com módulo

R-bits: Armazenados tanto na memória quanto nas tabelas

Justificativa: Implementação padrão e mais conhecida do algoritmo com busca circular

### 2.2 Organização do Código

Arquivo único: simulador.c

Principais funções:
main() - Coordena execução e processa argumentos
processar_acesso() - Lógica principal de HIT/PAGE FAULT
tratar_page_fault() - Gerencia falta de página
substituir_fifo() - Algoritmo FIFO de substituição
substituir_clock() - Algoritmo Clock de substituição
achar_frame_livre() - Localiza frames disponíveis

### 2.3 Algoritmo FIFO

Explique **como** implementou a lógica FIFO:

Lógica FIFO:
Cada frame armazena o momento exato em que foi carregado (tempo_entrada)
Quando precisa substituir, percorre todos os frames ocupados
Seleciona o frame com o menor tempo_entrada (mais antigo)
Remove a página antiga da tabela e carrega a nova página
Atualiza o tempo_entrada para o momento atual


### 2.4 Algoritmo Clock

Explique **como** implementou a lógica Clock:

Lógica Clock:

Mantém um ponteiro circular que avança pela memória
Para cada frame na posição do ponteiro:
   Se R-bit = 0: seleciona como vítima, avança ponteiro
   Se R-bit = 1: dá "segunda chance" - zera R-bit e avança
   O R-bit é setado para 1 em TODO acesso à página
Se todas as páginas tiverem R=1, eventualmente todas serão zeradas e o algoritmo encontrará uma vítima

### 2.5 Tratamento de Page Fault

Explique como seu código distingue e trata os dois cenários:

**Cenário 1: Frame livre disponível**

Identifica frames livres verificando processo = -1
Aloca a página no primeiro frame livre encontrado
Atualiza tabela de páginas e informações do frame
Seta R-bit para 1 (acesso recente)

**Cenário 2: Memória cheia**

Identifica memória cheia quando achar_frame_livre() retorna -1
Usa o algoritmo especificado na linha de comando (FIFO ou Clock)
Remove a página vítima da tabela
Carrega a nova página no frame liberado
Mostra mensagem completa da substituição

## 3. Análise Comparativa FIFO vs Clock

### 3.1 Resultados dos Testes

Preencha a tabela abaixo com os resultados de pelo menos 3 testes diferentes:

| Descrição do Teste | Total de Acessos | Page Faults FIFO | Page Faults Clock | Diferença |
|-------------------|------------------|------------------|-------------------|-----------|
| Teste 1 - Básico  |         8         |         5         |        5           |     0      |
| Teste 2 - Memória Pequena |    10      |        10          |        10           |     0      |
| Teste 3 - Simples |         7         |         4         |          4         |      0     |
| Teste Próprio 1   |        10          |        8          |         8          |     0      |

### 3.2 Análise

Com base nos resultados acima, responda:

1. **Qual algoritmo teve melhor desempenho (menos page faults)?**
   Ambos os algoritmos tiveram o mesmo desempenho em todos os testes realizados.

2. **Por que você acha que isso aconteceu?** Considere:
   Os padrões de acesso dos testes fornecidos não exploraram situações onde o Clock poderia se beneficiar do R-bit. Em acessos sequenciais ou sem reutilização frequente de páginas, ambos os algoritmos se comportam de forma similar.

3. **Em que situações Clock é melhor que FIFO?**
   Clock seria melhor em situações com:
   Padrões de acesso com páginas "quentes" (frequentemente reutilizadas)
   Acessos cíclicos onde algumas páginas são acessadas repetidamente
   Cenários onde o FIFO removeria páginas ainda em uso ativo
   
4. **Houve casos onde FIFO e Clock tiveram o mesmo resultado?**
  Sim, em todos os testes realizados. Isso aconteceu porque os padrões de acesso não criaram situações onde a "segunda chance" do Clock faria diferença.

5. **Qual algoritmo você escolheria para um sistema real e por quê?**
   Escolheria o algoritmo Clock por ser uma melhoria inteligente sobre o FIFO. Embora nos testes tenham sido equivalentes, em cenários reais com padrões de acesso mais complexos, o Clock tende a performar melhor ao preservar páginas       frequentemente usadas, com um custo computacional muito similar ao FIFO.
   
## 4. Desafios e Aprendizados

### 4.1 Maior Desafio Técnico

Descreva o maior desafio técnico que seu grupo enfrentou durante a implementação:

O maior desafio técnico foi implementar corretamente o algoritmo Clock e entender o funcionamento do R-bit.

Inicialmente, não estava claro quando e onde o R-bit deveria ser setado. Nos primeiros testes, o algoritmo Clock não estava funcionando como esperado porque o R-bit não estava sendo atualizado consistentemente em todos os acessos.
Os testes com diff mostravam diferenças nas saídas, especialmente quando comparávamos com os resultados esperados. Percebemos que em alguns casos o Clock deveria ter menos page faults que o FIFO, mas isso não estava acontecendo.
Através da discussão em grupo, chegamos a conclussão que o R-bit deve ser setado para 1 em TODO acesso à página, tanto em HITs quanto após carregar uma página em PAGE FAULTs. Implementamos essa lógica de forma consistente:
   Em HITs: setar R-bit na tabela e na memória
   Em PAGE FAULTs: setar R-bit após carregar a nova página
   No algoritmo Clock: zerar R-bit ao dar segunda chance


Aprendemos que pequenos detalhes de implementação podem ter grande impacto no comportamento dos algoritmos. A consistência na atualização dos bits de controle é crucial para o funcionamento correto dos algoritmos de substituição.

### 4.2 Principal Aprendizado

Descreva o principal aprendizado sobre gerenciamento de memória que vocês tiveram com este projeto:

O principal aprendizado foi entender na prática como a memória virtual permite que programas usem mais memória do que a fisicamente disponível.

**O que não entendiam bem antes e agora entendem?**
Antes, a tradução de endereços virtuais para físicos parecia algo confuso. Agora entendemos como o sistema operacional usa tabelas de páginas para mapear endereços virtuais em frames físicos, e o que acontece quando uma página não está na memória.

**Como este projeto mudou sua compreensão de memória virtual?**
Antes viamos mais a memória virtual como um conceito teórico. Agora entendo que é um mecanismo prático que isola processos uns dos outro, permite oversubscription de memória e requer algoritmos inteligentes para decidir quais páginas manter na memória física.

**Que conceito das aulas ficou mais claro após a implementação?**
O conceito de page fault ficou muito mais claro. Implementar o tratamento de page faults mostrou na prática o custo envolvido quando uma página precisa ser trazida do disco, e por que algoritmos como Clock que reduzem page faults são importantes para o desempenho do sistema. A implementação também deixou claro a diferença entre páginas virtuais e frames físicos, e como a tabela de páginas conecta esses dois.
---

## 5. Vídeo de Demonstração

**Link do vídeo:** https://youtu.be/CvU5WW3Zh9Q

### Conteúdo do vídeo:

Confirme que o vídeo contém:

- [x] Demonstração da compilação do projeto
- [x] Execução do simulador com algoritmo FIFO
- [x] Execução do simulador com algoritmo Clock
- [x] Explicação da saída produzida
- [x] Comparação dos resultados FIFO vs Clock
- [x] Breve explicação de uma decisão de design importante

---

## Checklist de Entrega

Antes de submeter, verifique:

- [x] Código compila sem erros conforme instruções da seção 1.1
- [x] Simulador funciona corretamente com FIFO
- [x] Simulador funciona corretamente com Clock
- [x] Formato de saída segue EXATAMENTE a especificação do ENUNCIADO.md
- [x] Testamos com os casos fornecidos em tests/
- [x] Todas as seções deste relatório foram preenchidas
- [x] Análise comparativa foi realizada com dados reais
- [x] Vídeo de demonstração foi gravado e link está funcionando
- [x] Todos os integrantes participaram e concordam com a submissão

---
## Referências
Materiais da Disciplina
Aulas 11 e 12: Slides e materiais sobre gerenciamento de memória virtual, paginação e algoritmos de substituição
ENUNCIADO.md: Especificação completa do projeto com formato de entrada/saída
CONCEITOS.md: Explicação teórica de paginação, page fault e algoritmos de substituição
FAQ.md: Perguntas frequentes e dicas de implementação fornecidas pelo professor

Consultas Técnicas
OpenAI ChatGPT: Consultas para esclarecimento de conceitos sobre implementação de algoritmos FIFO e Clock, estruturas de dados apropriadas e resolução de problemas específicos de codificação

Discussões sobre implementação do R-bit e sua correta atualização
Esclarecimentos sobre o funcionamento do ponteiro circular no algoritmo Clock

---

## Comentários Finais

Use este espaço para quaisquer observações adicionais que julguem relevantes (opcional).

---
