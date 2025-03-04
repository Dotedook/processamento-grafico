# A Unidade de Processamento Gráfico (GPU)

O capítulo aborda a arquitetura e o funcionamento da GPU, focando em sua evolução de pipelines fixos para sistemas programáveis e altamente paralelos. Aqui estão os pontos principais:

## 1. Arquitetura Paralela de Dados

### Paralelismo Massivo

- A GPU utiliza milhares de núcleos (_shader cores_) para processar vértices, fragmentos e outros dados simultaneamente.

### Warps/Wavefronts

- Grupos de threads (32 threads no NVIDIA, chamados de _warps_) executados em paralelo.
- O escalonamento rápido entre _warps_ minimiza a latência durante operações de memória.

### Ocultação de Latência

- Quando uma operação (ex.: acesso à textura) causa espera, a GPU alterna para outro _warp_, mantendo os núcleos ocupados.

### Divergência de Threads

- "Ifs" e loops podem reduzir a eficiência, pois threads em um mesmo _warp_ devem seguir o mesmo fluxo de instruções.

## 2. Pipeline Gráfico na GPU

O pipeline é dividido em estágios programáveis (verde), configuráveis (amarelo) e fixos (azul):

- **Estágios Programáveis:** _Vertex Shader, Tessellation, Geometry Shader, Pixel Shader._
- **Estágios Configuráveis:** _Screen Maping, Merger._
- **Estágios Fixos:** _Clipping, Triangle Setup_

## 3. Shaders Programáveis

### Modelo Unificado

- Todos os shaders (_vertex, pixel, etc._) compartilham a mesma arquitetura de instruções (ISA) e núcleos de processamento.

### Linguagens

- HLSL (DirectX) e GLSL (OpenGL) são usadas para programar shaders, com suporte a tipos de dados (_float, int_), vetores, matrizes e fluxo de controle.

### Recursos

- Registradores de entrada/saída, buffers constantes, texturas e acesso a memória compartilhada.

## 4. Evolução Histórica

### De Pipelines Fixos a Programáveis

- NVIDIA GeForce 256 (1999) introduziu transformações por hardware.
- DirectX 8 (2001) trouxe _vertex/pixel shaders_ programáveis.

### APIs Modernas

- DirectX 12, Vulkan e Metal focam em reduzir _overhead_ do driver, permitindo controle direto do hardware para maior eficiência.

### Mobile

- OpenGL ES e WebGL simplificam o pipeline para dispositivos móveis, com suporte a shaders programáveis desde 2007 (OpenGL ES 2.0).

## 5. Principais Estágios do Pipeline

### Vertex Shader

- Transforma vértices do espaço do modelo para o espaço de tela.
- **Aplicações:** Animação (ex.: _skinning_), deformações procedurais (ex.: bandeiras ondulantes).

### Tessellation Stage (Opcional)

- Divide superfícies curvas em triângulos dinamicamente.

#### Componentes:

- **Hull Shader:** Define nível de detalhe (LOD) e ajusta _control points_.
- **Tessellator (fixo):** Gera malha de triângulos.
- **Domain Shader:** Calcula posições finais dos vértices.

#### Vantagens:

- Eficiência em memória e adaptação à distância da câmera.

### Geometry Shader (Opcional)

- Modifica ou gera primitivas (ex.: converter pontos em partículas, criar _wireframes_).
- Limitado por desempenho, pois a ordenação de saída pode causar _overhead_.

### Stream Output

- Permite armazenar vértices processados em buffers para reutilização (ex.: simulação de fluidos).

### Pixel Shader (_Fragment Shader_)

- Calcula cor e atributos por fragmento, com interpolação de dados dos vértices.

#### MRT (_Multiple Render Targets_)

- Saída para múltiplos buffers (ex.: cor, normais, profundidade).

#### Acesso a Buffers

- UAVs (DirectX) e SSBOs (OpenGL) permitem escrita/leitura em memória compartilhada.

### Merging Stage

- **Testes de profundidade (_early-z_):** Descartam fragmentos ocultos antecipadamente.
- **Blending:** Combina cores (ex.: transparência) e aplica operações lógicas.
- **ROVs (_Rasterizer Order Views_):** Garantem ordem de execução para evitar conflitos em escrita.

## 6. Compute Shader

### Uso Geral (_GPGPU_)

- Processamento paralelo fora do pipeline gráfico (ex.: simulações físicas, pós-processamento).

### Thread Groups

- Grupos de threads com memória compartilhada para cálculos cooperativos.

### Aplicações

- Filtros de imagem, cálculos de luminância, simulação de partículas.

## Conclusão

A GPU moderna é uma arquitetura altamente paralela e flexível, evoluindo de pipelines fixos para shaders programáveis e computação de propósito geral. APIs como DirectX 12 e Vulkan oferecem controle refinado, enquanto estágios como _tessellation_ e _compute shader_ expandem as possibilidades de renderização e processamento em tempo real.
