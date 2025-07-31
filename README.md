# Utilização do sensor modular ZPHS01B para medição da qualidade do ar, com um ESP32WROOM DevKit 4, através da extensão ESPRESSIFSDK (ESP-IDF)

## Autor
**JOÃO VICTOR PERES FERERIRA PONTES**
- **Repositório do Projeto:** [https://github.com/JoaoFerreiraPontes/Sensor_QdA_ZPHS01B_Projeto](https://github.com/JoaoFerreiraPontes/Sensor_QdA_ZPHS01B_Projeto)

## Visão Geral do Projeto
Este projeto tem como objetivo a medição da qualidade do ar como um todo, focando na detecção e análise das concentrações de gases e partículas prejudiciais no ambiente. O sistema permite uma estimativa aproximada da poluição do ar, visando a tomada de ações informadas para a melhoria do bem-estar e da produtividade, diretamente impactados pela qualidade do ar.

O sistema utiliza um sensor multi-parâmetro **ZPHS01B** para coletar dados, um microcontrolador **ESP32** para processá-los e a tecnologia **Bluetooth** para enviar as informações em tempo real para um smartphone, permitindo monitoramento e análise.

### O Sensor ZPHS01B
O módulo ZPHS01B é um sensor que integra diversos componentes de detecção avançados. Ele é capaz de medir:
- **Partículas em suspensão:** PM1.0, PM2.5 e PM10 (Sensor a laser)
- **Gases:**
    - Dióxido de Carbono ($CO_2$) (Sensor infravermelho)
    - Formaldeído ($CH_2O$) (Sensor eletroquímico)
    - Ozônio ($O_3$) (Sensor eletroquímico)
    - Monóxido de Carbono ($CO$) (Sensor eletroquímico)
    - Dióxido de Nitrogênio ($NO_2$) (Sensor eletroquímico)
- **Compostos Orgânicos Voláteis (VOC)**
- **Temperatura e Umidade Relativa**

A documentação completa do sensor pode ser encontrada [neste link](https://www.winsen-sensor.com/product/zphs01b.html).

## Funcionalidades do Software (Casos de Uso)
O firmware desenvolvido para o ESP32 oferece as seguintes funcionalidades:
- **Atraso na Inicialização:** O programa aguarda 10 segundos antes de iniciar para garantir que o usuário consiga abrir o monitor serial a tempo.
- **Configuração Interativa:** Ao iniciar, o sistema solicita que o usuário defina a frequência de recebimento dos dados (em milissegundos).
- **Feedback Visual:** O usuário consegue ver os números que está digitando no terminal e pode usar a tecla *Backspace* para corrigir a entrada.
- **Timeout Inteligente:** Caso nenhum valor seja inserido em 3 minutos, o sistema adota uma frequência padrão de 5 segundos para não interromper o funcionamento.
- **Mudança de Frequência em Tempo Real:** A qualquer momento, enquanto os dados estão sendo recebidos, o usuário pode pressionar a tecla 'X' para interromper o fluxo e inserir uma nova frequência de medição.
- **Transmissão via Bluetooth:** Todos os dados formatados são enviados via Bluetooth SPP (Serial Port Profile), permitindo que qualquer aplicativo de terminal Bluetooth (como o "Serial Bluetooth Terminal" para Android) se conecte e receba as informações.

## Guia de Instalação e Configuração
Este tutorial assume que você já possui o **Visual Studio Code** instalado.

### Passo 1: Instalação do Driver USB
Para que seu computador reconheça a placa ESP32, é necessário instalar um driver de comunicação. A maioria das placas ESP32-WROOM utiliza o chip **CP2102**.

1.  Acesse o site da Silicon Labs para baixar o driver: [CP210x USB to UART Bridge VCP Drivers](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers).
2.  Baixe a versão correspondente ao seu sistema operacional (ex: Windows 10/11).
3.  Descompacte o arquivo e execute o instalador.
4.  Após a instalação, conecte a placa ESP32 ao computador. O Windows deverá reconhecê-la e atribuir uma porta COM (ex: `COM3`).

### Passo 2: Instalação do ESP-IDF (v5.5)
A maneira mais fácil de configurar o ambiente de desenvolvimento é usando o instalador oficial da Espressif.

1.  Acesse a página de releases do ESP-IDF no GitHub: [ESP-IDF Releases](https://github.com/espressif/esp-idf/releases).
2.  Procure pela **versão 5.5** e baixe o arquivo `esp-idf-tools-setup-online-*.exe`.
3.  Execute o instalador e siga os passos:
    - **Aceite os termos** da licença.
    - **Selecione a versão do ESP-IDF:** Escolha a **v5.5**.
    - **Caminhos de Instalação:** Você pode manter os caminhos padrão (geralmente `C:\Users\SeuUsuario\esp\esp-idf` para o framework e `C:\Users\SeuUsuario\.espressif` para as ferramentas).
    - **Instalação dos Componentes:** O instalador irá baixar e configurar todas as ferramentas necessárias (compilador, Python, etc.). Aguarde a conclusão, o que pode levar vários minutos.
4.  Ao final, o instalador terá criado atalhos no seu Menu Iniciar. Isso é fundamental para usar o ambiente corretamente.

### Passo 3: Configuração do Projeto no VS Code
1.  Clone este repositório para o seu computador.
2.  Abra o **VS Code**.
3.  Vá até a aba **Extensões** (ícone de quadrados na barra lateral).
4.  Pesquise por **`ESP-IDF`** e instale a extensão oficial da **Espressif Systems**.
5.  Abra a pasta do projeto clonado no VS Code (`Arquivo > Abrir Pasta...`).
6.  Agora, configure a extensão para que ela encontre suas ferramentas. Pressione `Ctrl+Shift+P` para abrir a paleta de comandos e siga estes passos:
    - Digite e selecione **`ESP-IDF: Configure ESP-IDF extension`**.
    - Selecione a opção **EXPRESS**.
    - Na barra **`Select ESP-IDF version:`**, encontre e selecione a sua instalação da **versão 5.5**.
    - O assistente irá verificar todas as ferramentas. Aguarde a conclusão e a mensagem de sucesso.

## Configuração do Hardware
Conecte o sensor ZPHS01B à placa ESP32 da seguinte forma:

| Interface ZPHS01B | Pinagem Padrão ESP32 |
| :---------------- | :------------------: |
| Tensão (Vc)       |         `5V`         |
| Receptor (RxD)    |        `GPIO16`      |
| Transmissor (TxD) |        `GPIO17`      |
| Terra (GND)       |         `GND`        |

OBS: A configuração dos pinos pode variar de acordo com o modelo da sua ESP. No caso deste projeto, foi-se utilizado o modelo ESP32WROOM DevKit 4. 
Consulte o datasheet do seu microcontrolador para definir os pinos do seu UART (Isto pode ser modificado no arquivo sdkconfig, nas funções 
CONFIG_EXAMPLE_UART_RXD e CONFIG_EXAMPLE_UART_TXD)

## Como Usar o Projeto no VS Code
Com o ambiente e o hardware configurados, o uso é simplificado pela extensão do ESP-IDF.

### Funcionalidades da Extensão (Botões)
A extensão adiciona uma barra na parte inferior do VS Code com ícones para as principais ações. A imagem que você forneceu mostra o menu de comandos, e aqui estão os mais importantes:

-   `⚙️ SDK Configuration Editor`: Abre uma interface gráfica para o `menuconfig`, permitindo alterar configurações do projeto (como os pinos do sensor).
-   `✔️ Full Clean`: Apaga todos os arquivos de compilação antigos. Útil para resolver problemas estranhos de compilação.
-   `[Build] Build Project`: Apenas compila o código para verificar se há erros.
-   `⚡️ Flash Device`: Grava o último firmware compilado na placa.
-   `🖥️ Monitor Device`: Abre o terminal para ver a saída serial da placa.
-   `🔥 ESP-IDF: Build, Flash and Monitor`: O comando mais útil. Ele executa as três ações acima em sequência: compila, grava e abre o monitor.
-   `Erase Flash`: (No menu "Advanced") Apaga completamente a memória flash do ESP32.

### Passo a Passo da Execução
1.  Com o projeto aberto no VS Code, verifique a barra inferior e selecione a porta **COM correta**.
2.  Use o comando **`ESP-IDF: Build, Flash and Monitor`** (pode ser acessado com `Ctrl+Shift+P` ou clicando no ícone de "chama" 🔥 na barra inferior).
3.  Aguarde a compilação e a gravação. O terminal do monitor serial será aberto automaticamente.
4.  **Interaja com o programa:**
    - O programa iniciará após uma contagem de 10 segundos.
    - Você verá a tela de boas-vindas e será solicitado a digitar o intervalo de tempo. Digite os números (ex: `3000`) e pressione Enter.
    - Os dados do sensor começarão a ser exibidos na frequência definida.
    - A qualquer momento, pressione a tecla `X` e depois `Enter` para voltar ao menu de configuração de frequência.

## Análise de Uso de Memória
Após compilar o projeto (`build`), o ESP-IDF exibe um sumário de como a memória do microcontrolador foi utilizada. Esta tabela é uma ferramenta poderosa para entender o tamanho do seu programa e otimizar o uso de recursos.

<pre><code>
Memory Type Usage Summary
┏━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━┳━━━━━━━━━━┳━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━┓
┃ Memory Type/Section   ┃ Used [bytes] ┃ Used [%] ┃ Remain [bytes] ┃ Total [bytes] ┃
┡━━━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━╇━━━━━━━━━━╇━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━┩
│ Flash Code            │       441866 │          │                │               │
│   .text               │       441866 │          │                │               │
│ Flash Data            │       122988 │          │                │               │
│   .rodata             │       122732 │          │                │               │
│   .appdesc            │          256 │          │                │               │
│ IRAM                  │        98943 │    75.49 │          32129 │        131072 │
│   .text               │        97915 │     74.7 │                │               │
│   .vectors            │         1028 │     0.78 │                │               │
│ DRAM                  │        43044 │    34.55 │          81536 │        124580 │
│   .bss                │        24488 │    19.66 │                │               │
│   .data               │        18556 │    14.89 │                │               │
│ RTC FAST              │           32 │     0.39 │           8160 │          8192 │
│   .force_fast         │           32 │     0.39 │                │               │
│ RTC SLOW              │           24 │     0.29 │           8168 │          8192 │
│   .rtc_slow_reserved  │           24 │     0.29 │                │               │
└───────────────────────┴──────────────┴──────────┴────────────────┴───────────────┘
</code></pre>

### O que cada tipo de memória significa?

-   **Flash (Memória Não-Volátil):** Pense nisso como o "HD" do ESP32. É aqui que o seu programa é armazenado permanentemente.
    -   `Flash Code (.text)`: Contém as instruções do seu código, ou seja, o programa compilado.
    -   `Flash Data (.rodata)`: Armazena dados que não mudam (read-only), como textos constantes (ex: a mensagem de boas-vindas).

-   **IRAM (Instruction RAM):** É uma porção da memória RAM usada para executar trechos de código que precisam de altíssima velocidade (como partes críticas do Wi-Fi ou interrupções). O ESP-IDF gerencia isso automaticamente.

-   **DRAM (Data RAM):** É a memória de trabalho principal do seu programa, similar à RAM de um computador. Ela armazena as variáveis que mudam durante a execução. É um recurso volátil e limitado, por isso é importante monitorar seu uso.
    -   `.data`: Guarda variáveis globais e estáticas que são inicializadas com um valor diferente de zero.
    -   `.bss`: Guarda variáveis globais e estáticas que são inicializadas com zero. Otimização que economiza espaço na Flash.

-   **RTC FAST/SLOW (Memória de Baixo Consumo):** É uma pequena quantidade de memória que pode permanecer ligada mesmo quando o ESP32 está em modo de sono profundo (*deep sleep*). É usada para tarefas de baixíssimo consumo de energia.

Em resumo, esta tabela mostra que o seu programa ocupa aproximadamente **565 KB na Flash** (442 KB de código + 123 KB de dados) e utiliza **34.55% da memória RAM principal (DRAM)** disponível para as variáveis e o funcionamento do sistema.

## Exemplo de Saída

A saída no monitor serial será parecida com isto:

<pre><code>
Iniciando em 10 segundos...


--- Sensor de Qualidade do Ar com ZPHS01B ---
Este projeto utiliza o sensor ZPHS01B, um modulo multi-parametros que mede
particulas (PM1.0, PM2.5, PM10), gases (CO2, CO, O3, NO2), Compostos Organicos
Volateis (VOC), formaldeido (CH2O), temperatura e umidade.
Autor do Projeto: Joao Abrantes Pontes
Repositorio do Projeto: https://github.com/JoaoFerreiraPontes/Sensor_QdA_ZPHS01B_Projeto

AVISO IMPORTANTE:
O sensor requer um periodo de calibração de 10 minutos para que as medicoes
se estabilizem. Recomenda-se utilizar os dados para tratamento real somente
apos este periodo.

I (11211) ZPHS01B_UART: Driver UART do sensor ZPHS01B inicializado.
------------------------------------------------------------------
Digite APENAS NUMEROS para definir o intervalo em milissegundos (ms).
Valores aceitos: de 1500ms a 29000ms.
Pressione Enter para confirmar ou aguarde 180 segundos para usar o tempo padrao (5000ms).
Intervalo: 
</code></pre>

## Conexão Bluetooth
Para visualizar os dados no seu smartphone:
1.  Ative o Bluetooth do seu celular.
2.  Procure por um dispositivo chamado `ESP_SPP_ACCEPTOR`.
3.  Pareie com ele.
4.  Abra um aplicativo de terminal serial Bluetooth (ex: "Serial Bluetooth Terminal" na Play Store).
5.  Conecte-se ao `ESP_SPP_ACCEPTOR` e os dados do sensor começarão a aparecer no aplicativo.