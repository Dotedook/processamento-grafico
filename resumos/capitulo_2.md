# O Pipeline de Renderização Gráfica

O capítulo descreve o pipeline de renderização gráfica, essencial para gerar imagens 2D a partir de cenas 3D em tempo real. Dividido em quatro estágios principais, o processo envolve:

## 1. Estágio de Aplicação

**Função:** Controlado pelo desenvolvedor na CPU, executa tarefas como:

- Detecção de colisões, simulação física, entrada do usuário.
- Algoritmos de otimização (ex: culling para descartar objetos fora da vista).
- Preparação de primitivos (pontos, linhas, triângulos) para processamento.

**Saída:** Geometria (objetos 3D) e configurações (câmera, luzes) enviadas ao próximo estágio.

## 2. Processamento Geométrico (GPU)

Transforma vértices e prepara a geometria para rasterização:

### Transformações:

- **Modelagem:** Posiciona objetos no espaço 3D (coordenadas de mundo).
- **Visualização:** Ajusta a cena para a perspectiva da câmera (espaço de visualização).
- **Projeção:** Converte para coordenadas de recorte (volume de visualização em cubo unitário).
- **Recorte:** Remove primitivos fora do volume de visualização ou os ajusta.
- **Mapeamento para Tela:** Converte coordenadas normalizadas para pixels na janela de exibição.

### Estágios Opcionais:

- **Tesselação:** Divide superfícies em triângulos dinamicamente.
- **Geometria Shader:** Modifica primitivos ou gera novos (ex: partículas).
- **Stream Output:** Envia dados processados para uso posterior (CPU/GPU).

## 3. Rasterização

Converte primitivos geométricos em fragmentos (pixels potenciais):

- **Configuração do Triângulo:** Calcula bordas e dados para interpolação.
- **Travessia do Triângulo:** Identifica pixels cobertos pelo primitivo, usando amostragem (ponto central ou múltiplos pontos para antialiasing).

## 4. Processamento de Pixels

Define cores e resolve visibilidade:

### Pixel Shading:

- Calcula cores usando sombreamento, texturas e luzes (programável na GPU).
- Exemplo: Aplicação de texturas como logotipos em superfícies.

### Mesclagem (Merging):

- **Z-Buffer:** Teste de profundidade para determinar visibilidade.
- **Blending:** Combina cores (ex: transparência).
- **Stencil Buffer:** Máscaras para efeitos específicos.
- **Double Buffering:** Evita flicker exibindo a imagem apenas quando pronta.

## Exemplo Prático (Aplicação CAD)

- **Aplicação:** O usuário move a tampa de uma máquina de waffle; a CPU atualiza matrizes de transformação.
- **Geometria:** Vértices são transformados, projetados e recortados para a visão da câmera.
- **Rasterização:** Triângulos da tampa são convertidos em pixels.
- **Pixels:** Texturas são aplicadas, e o Z-buffer garante que apenas superfícies visíveis sejam renderizadas.

## Conclusão

O pipeline moderno é programável, permitindo flexibilidade em GPUs. Técnicas como ray tracing são alternativas para renderização offline, mas o pipeline descrito é otimizado para velocidade em tempo real. A evolução das APIs e hardware permitiu estágios customizáveis, essenciais para gráficos realistas e interativos.
