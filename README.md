# Utilização do sensor modular ZPHS01B para medição da qualidade do ar

### Medição da qualidade do ar com ESP32-WROOM DevKit 4, através da extensão ESPRESSIF SDK (ESP-IDF)

**Autor:** JOÃO VICTOR PERES FERERIRA PONTES
**Repositório do Projeto:** [https://github.com/JoaoFerreiraPontes/Sensor_QdA_ZPHS01B_Projeto](https://github.com/JoaoFerreiraPontes/Sensor_QdA_ZPHS01B_Projeto)

## Visão Geral do Projeto

Este projeto tem como objetivo a medição da qualidade do ar como um todo, focando na detecção e análise das concentrações de gases e partículas prejudiciais no ambiente. O sistema permite uma estimativa aproximada da poluição do ar, visando a tomada de ações informadas para a melhoria do bem-estar e da produtividade, diretamente impactados pela qualidade do ar.

O sistema utiliza um sensor multi-parâmetro **ZPHS01B** para coletar dados, um microcontrolador **ESP32** para processá-los e a tecnologia **Bluetooth** para enviar as informações em tempo real para um smartphone, permitindo monitoramento e análise.

## O Sensor ZPHS01B

O módulo ZPHS01B é um sensor que integra diversos componentes de detecção avançados. Ele é capaz de medir:

* **Partículas em suspensão:** PM1.0, PM2.5 e PM10 (Sensor a laser)
* **Gases:**
    * Dióxido de Carbono (CO2) (Sensor infravermelho)
    * Formaldeído (CH2O) (Sensor eletroquímico)
    * Ozônio (O3) (Sensor eletroquímico)
    * Monóxido de Carbono (CO) (Sensor eletroquímico)
    * Dióxido de Nitrogênio (NO2) (Sensor eletroquímico)
    * Compostos Orgânicos Voláteis (VOC)
* **Temperatura e Umidade Relativa**

A documentação completa do sensor pode ser encontrada [neste link](https://www.winsen-sensor.com/d/files/PDF/zphs01b-multi-in-one-module-v1_1.pdf).

## Funcionalidades do Sistema (Análise de Casos de Uso)

Esta seção detalha o comportamento do software sob a perspectiva da engenharia de requisitos, descrevendo as interações do usuário e as respostas do sistema.

### Casos de Uso Detalhados

#### UC01: Configurar o Intervalo de Medição

* **Ator:** Usuário Final
* **Resumo:** O usuário define a frequência com que os dados do sensor são lidos e transmitidos pelo sistema.
* **Pré-condições:**
    1.  O dispositivo ESP32 deve estar ligado.
    2.  O Usuário Final deve ter um terminal serial (via USB ou Bluetooth) conectado e visível.
* **Fluxo Principal (Caminho Feliz):**
    1.  O sistema é iniciado ou reiniciado.
    2.  O sistema exibe no terminal a mensagem de boas-vindas e o prompt para inserir a frequência em milissegundos.
    3.  O usuário digita um valor numérico (ex: `5000`). O sistema exibe cada dígito no terminal conforme é digitado.
    4.  O usuário pressiona a tecla `Enter` para confirmar.
    5.  O sistema valida a entrada, confirma a nova frequência e informa que as medições começarão.
    6.  O caso de uso é encerrado com sucesso e o sistema transiciona para o monitoramento contínuo (UC02/UC03).
* **Fluxos Alternativos:**
    * **A1 (Timeout na Configuração):** Se o usuário não inserir um valor e pressionar `Enter` dentro de 180 segundos (3 minutos), o sistema cancela o prompt, adota a frequência padrão de 5 segundos e inicia o monitoramento automaticamente.
    * **A2 (Correção de Entrada):** Durante o passo 3, se o usuário pressionar a tecla `Backspace`, o sistema apaga o último dígito inserido, permitindo a correção.
    * **A3 (Reconfiguração em Tempo Real):** Se o sistema já estiver no modo de monitoramento (UC02/UC03), o usuário pode pressionar a tecla `X` e `Enter`. O sistema interrompe o monitoramento e reinicia este caso de uso (UC01) a partir do passo 2.
* **Pós-condição:**
    * A frequência de medição do sistema é definida com o valor inserido pelo usuário ou com o valor padrão.

#### UC02: Monitorar a Qualidade do Ar via Conexão Serial

* **Ator:** Usuário Final
* **Resumo:** O usuário visualiza o fluxo de dados do sensor em tempo real através de um terminal conectado via USB.
* **Pré-condições:**
    1.  O dispositivo deve estar conectado ao computador via cabo USB.
    2.  O software do monitor serial (ex: no VS Code, Arduino IDE) deve estar aberto e conectado à porta COM correta.
    3.  O Caso de Uso UC01 (Configurar o Intervalo de Medição) deve ter sido concluído com sucesso.
* **Fluxo Principal (Caminho Feliz):**
    1.  No intervalo de tempo definido, o sistema solicita e recebe os dados brutos do sensor ZPHS01B.
    2.  O sistema processa e formata os dados em uma string legível, com identificação para cada parâmetro (PM2.5, CO2, Temp, etc.).
    3.  O sistema envia a string formatada para a saída serial (UART).
    4.  O usuário visualiza os novos dados no monitor serial.
    5.  O processo se repete a partir do passo 1, indefinidamente.
* **Fluxo Alternativo:**
    * **A1 (Interrupção para Reconfigurar):** A qualquer momento, o usuário pode acionar o fluxo alternativo A3 do UC01 para interromper este fluxo e reconfigurar a frequência.
* **Pós-condição:**
    * O usuário se mantém continuamente informado sobre as condições da qualidade do ar no ambiente.

#### UC03: Monitorar a Qualidade do Ar via Bluetooth

* **Ator:** Usuário Final
* **Resumo:** O usuário visualiza os dados do sensor em um dispositivo móvel (smartphone/tablet) sem a necessidade de fios.
* **Pré-condições:**
    1.  O dispositivo ESP32 está ligado.
    2.  O Bluetooth do smartphone do usuário está ativado.
    3.  O usuário possui um aplicativo de terminal serial Bluetooth instalado.
    4.  O Caso de Uso UC01 foi concluído.
* **Fluxo Principal (Caminho Feliz):**
    1.  O sistema inicializa e anuncia seu serviço Bluetooth SPP com o nome `ESP_SPP_ACCEPTOR`.
    2.  O usuário, em seu smartphone, busca por dispositivos Bluetooth e encontra o `ESP_SPP_ACCEPTOR`.
    3.  O usuário solicita o pareamento com o dispositivo.
    4.  O usuário abre o aplicativo de terminal Bluetooth e se conecta ao dispositivo pareado.
    5.  Assim que a conexão é estabelecida, o sistema começa a enviar o mesmo fluxo de dados formatados (gerado no UC02) através da conexão Bluetooth.
    6.  O usuário visualiza os dados chegando em tempo real no aplicativo.
* **Fluxo Alternativo:**
    * **A1 (Perda de Conexão):** Se a conexão Bluetooth for perdida, o sistema simplesmente para de enviar os dados via Bluetooth, mas continua a operação interna (e a transmissão via serial, se conectada). Ele permanece pronto para aceitar uma nova conexão a qualquer momento.
* **Pós-condição:**
    * O usuário pode monitorar a qualidade do ar de forma remota e portátil.

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
    * Aceite os termos da licença.
    * Selecione a versão do ESP-IDF: Escolha a **v5.5**.
    * Caminhos de Instalação: Você pode manter os caminhos padrão (geralmente `C:\Users\SeuUsuario\esp\esp-idf` para o framework e `C:\Users\SeuUsuario\.espressif` para as ferramentas).
    * Instalação dos Componentes: O instalador irá baixar e configurar todas as ferramentas necessárias (compilador, Python, etc.). Aguarde a conclusão, o que pode levar vários minutos.
4.  Ao final, o instalador terá criado atalhos no seu Menu Iniciar. Isso é fundamental para usar o ambiente corretamente.

### Passo 3: Configuração do Projeto no VS Code

1.  Clone este repositório para o seu computador.
2.  Abra o **VS Code**.
3.  Vá até a aba **Extensões** (ícone de quadrados na barra lateral).
4.  Pesquise por **ESP-IDF** e instale a extensão oficial da **Espressif Systems**.
5.  Abra a pasta do projeto clonado no VS Code (`Arquivo > Abrir Pasta...`).
6.  Agora, configure a extensão para que ela encontre suas ferramentas. Pressione `Ctrl+Shift+P` para abrir a paleta de comandos e siga estes passos:
    * Digite e selecione **ESP-IDF: Configure ESP-IDF extension**.
    * Selecione a opção **EXPRESS**.
    * Na barra **Select ESP-IDF version:**, encontre e selecione a sua instalação da **versão 5.5**.
    * O assistente irá verificar todas as ferramentas. Aguarde a conclusão e a mensagem de sucesso.

## Configuração do Hardware

Conecte o sensor ZPHS01B à placa ESP32 da seguinte forma:

| Interface ZPHS01B | Pinagem Padrão ESP32 |
| :---------------- | :------------------- |
| **Tensão (Vc)** | 5V                   |
| **Receptor (RxD)**| GPIO16               |
| **Transmissor (TxD)**| GPIO17               |
| **Terra (GND)** | GND                  |

**OBS:** A configuração dos pinos pode variar de acordo com o modelo da sua ESP. No caso deste projeto, foi-se utilizado o modelo ESP32WROOM DevKit 4. Consulte o datasheet do seu microcontrolador para definir os pinos do seu UART (Isto pode ser modificado no arquivo `sdkconfig`, nas funções `CONFIG_EXAMPLE_UART_RXD` e `CONFIG_EXAMPLE_UART_TXD`).

## Como Usar o Projeto no VS Code

Com o ambiente e o hardware configurados, o uso é simplificado pela extensão do ESP-IDF.

### Funcionalidades da Extensão (Botões)

A extensão adiciona uma barra na parte inferior do VS Code com ícones para as principais ações:

* **⚙️ SDK Configuration Editor:** Abre uma interface gráfica para o `menuconfig`, permitindo alterar configurações do projeto (como os pinos do sensor).
* **✔️ Full Clean:** Apaga todos os arquivos de compilação antigos. Útil para resolver problemas estranhos de compilação.
* **[Build] Build Project:** Apenas compila o código para verificar se há erros.
* **⚡️ Flash Device:** Grava o último firmware compilado na placa.
* **🖥️ Monitor Device:** Abre o terminal para ver a saída serial da placa.
* **🔥 ESP-IDF: Build, Flash and Monitor:** O comando mais útil. Ele executa as três ações acima em sequência: compila, grava e abre o monitor.
* **Erase Flash:** (No menu "Advanced") Apaga completamente a memória flash do ESP32.

### Passo a Passo da Execução

1.  Com o projeto aberto no VS Code, verifique a barra inferior e selecione a porta **COM correta**.
2.  Use o comando **ESP-IDF: Build, Flash and Monitor** (pode ser acessado com `Ctrl+Shift+P` ou clicando no ícone de "chama" 🔥 na barra inferior).
3.  Aguarde a compilação e a gravação. O terminal do monitor serial será aberto automaticamente.
4.  Interaja com o programa:
    * O programa iniciará após uma contagem de 10 segundos.
    * Você verá a tela de boas-vindas e será solicitado a digitar o intervalo de tempo. Digite os números (ex: `3000`) e pressione Enter.
    * Os dados do sensor começarão a ser exibidos na frequência definida.
    * A qualquer momento, pressione a tecla `X` e depois `Enter` para voltar ao menu de configuração de frequência.

## Conexão Bluetooth

Para visualizar os dados no seu smartphone:

1.  Ative o Bluetooth do seu celular.
2.  Procure por um dispositivo chamado **ESP_SPP_ACCEPTOR**.
3.  Pareie com ele.
4.  Abra um aplicativo de terminal serial Bluetooth (ex: "Serial Bluetooth Terminal" na Play Store).
5.  Conecte-se ao **ESP_SPP_ACCEPTOR** e os dados do sensor começarão a aparecer no aplicativo.

## Análise de Uso de Memória

Após compilar o projeto (`build`), o ESP-IDF exibe um sumário de como a memória do microcontrolador foi utilizada. Esta tabela é uma ferramenta poderosa para entender o tamanho do seu programa e otimizar o uso de recursos.

**O que cada tipo de memória significa?**

* **Flash (Memória Não-Volátil):** Pense nisso como o "HD" do ESP32. É aqui que o seu programa é armazenado permanentemente.
    * **Flash Code (.text):** Contém as instruções do seu código, ou seja, o programa compilado.
    * **Flash Data (.rodata):** Armazena dados que não mudam (read-only), como textos constantes (ex: a mensagem de boas-vindas).
* **IRAM (Instruction RAM):** É uma porção da memória RAM usada para executar trechos de código que precisam de altíssima velocidade (como partes críticas do Wi-Fi ou interrupções).
* **DRAM (Data RAM):** É a memória de trabalho principal do seu programa, similar à RAM de um computador. Ela armazena as variáveis que mudam durante a execução.
    * **.data:** Guarda variáveis globais e estáticas que são inicializadas com um valor diferente de zero.
    * **.bss:** Guarda variáveis globais e estáticas que são inicializadas com zero (uma otimização para economizar espaço na Flash).
* **RTC FAST/SLOW (Memória de Baixo Consumo):** É uma pequena quantidade de memória que pode permanecer ligada mesmo quando o ESP32 está em modo de sono profundo (deep sleep).

Em resumo, a tabela de memória mostra o espaço que seu programa ocupa e quanta memória RAM está sendo utilizada para as variáveis e o funcionamento do sistema em tempo de execução.

## Exemplo de Saída

```
Iniciando em 10 segundos...


--- Sensor de Qualidade do Ar com ZPHS01B ---
Este projeto utiliza o sensor ZPHS01B, um modulo multi-parametros que mede
particulas (PM1.0, PM2.5, PM10), gases (CO2, CO, O3, NO2), Compostos Organicos
Volateis (VOC), formaldeido (CH2O), temperatura e umidade.
Autor do Projeto: Joao Abrantes Pontes
Repositorio do Projeto: [https://github.com/JoaoFerreiraPontes/Sensor_QdA_ZPHS01B_Projeto](https://github.com/JoaoFerreiraPontes/Sensor_QdA_ZPHS01B_Projeto)

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
```
