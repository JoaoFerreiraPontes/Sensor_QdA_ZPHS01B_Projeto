
# Utilização do sensor modular ZPHS01B para medição da qualidade do ar, através da ESPRESSIF SDK

**Autor:** [**JOÃO VICTOR PERES FERERIRA PONTES**git add]

Este projeto tem como objetivo a medição da qualidade do ar como um todo, focando na detecção e análise das concentrações de gases e partículas prejudiciais no ambiente. O sistema permite uma estimativa aproximada da poluição do ar, visando a tomada de ações informadas para a melhoria do bem-estar e da produtividade, diretamente impactados pela qualidade do ar.

O módulo ZPHS01B é um sensor multi-parâmetro que integra diversos componentes de detecção avançados. Ele possui os seguintes sensores:
* Sensor a laser para partículas (PM1.0, PM2.5, PM10)
* Sensor infravermelho de dióxido de carbono (CO2)
* Sensor eletroquímico de formaldeído (CH2O)
* Sensor eletroquímico de ozônio (O3)
* Sensor eletroquímico de monóxido de carbono (CO)
* Sensor de Compostos Orgânicos Voláteis (VOC)
* Sensor de dióxido de nitrogênio (NO2)
* Sensor de temperatura e umidade relativa

Este projeto combina a versatilidade do módulo ZPHS01B com o poder de processamento de um módulo ESP32 e a conveniência de um smartphone. O ESP32 é responsável por ler os valores brutos do módulo do sensor, processá-los e, posteriormente, enviá-los para um smartphone via Bluetooth, permitindo a visualização e análise dos dados em tempo real.

A documentação e especificações completas do módulo do sensor podem ser encontradas aqui: [https://www.winsen-sensor.com/product/zphs01b.html](https://www.winsen-sensor.com/product/zphs01b.html)

O projeto foi testado manualmente com uma placa de desenvolvimento ESP-WROOM-32 e um smartphone convencional.

## Como usar o exemplo

### Hardware Necessário

O exemplo pode ser executado em qualquer placa de desenvolvimento baseada nas séries ESP32, ESP32-S e ESP32-C que possua conectividade Bluetooth. A placa é conectada a um computador usando um único cabo USB para o processo de gravação (flashing). Alternativamente, para adicionar portabilidade ao dispositivo DIY (faça você mesmo), a conexão ao computador pode ser substituída por uma conexão a um powerbank. O objetivo principal deste exemplo é coletar dados sobre a poluição do ar utilizando o sensor ZPHS01B e transmiti-los para um dispositivo Android com o aplicativo "Serial Bluetooth Terminal" para uso e processamento futuros dos dados.

### Configuração do Hardware

Conecte a interface serial do ZPHS01B à placa ESP32 da seguinte forma:
  ---------------------------------------------------------------------
  | ZPHS01B Interface     | Opções Kconfig     | Pinagem Padrão ESP   |
  | ----------------------|--------------------|----------------------|
  | Tensão (Vc)           | n/a                | 5V                   |
  | Receptor (RxD)        | EXAMPLE_UART_TXD   | GPIO4                |
  | Transmissor D (TxD)   | EXAMPLE_UART_RXD   | GPIO5                |
  | Terra (GND)           | n/a                | GND                  |
  ---------------------------------------------------------------------

Observação: Alguns GPIOs não podem ser usados com determinados chips porque são reservados para uso interno. Consulte a documentação da UART para o destino selecionado.

### Configuração do Projeto

Utilize o comando abaixo para configurar o projeto usando o menu Kconfig, conforme mostrado na tabela acima. Os valores padrão do Kconfig podem ser alterados, como: `EXAMPLE_TASK_STACK_SIZE`, `EXAMPLE_UART_BAUD_RATE`, `EXAMPLE_UART_PORT_NUM` (Consulte o arquivo Kconfig).
```
idf.py menuconfig
```
Para detalhes adicionais sobre a configuração, você pode verificar os seguintes exemplos no ESP-IDF v5.0.7:
`uart_echo_example`, `bt_spp_acceptor`.


### Construir e Gravar (Flash)


Construa o projeto e grave-o na placa, em seguida, execute a ferramenta de monitoramento para visualizar a saída serial.

```
idf.py -p PORT flash monitor
```
Isso mostrará os logs com os dados do sensor e as informações de Bluetooth do dispositivo conectado, caso tudo esteja correto e tenha sido feito da maneira certa.

(Para sair do monitor serial, digite ``Ctrl-]``.)


## Exemplo de SAÍDA

Este é um exemplo de uma string com a contaminação do ar, conforme medida pelo ZPHS01B
`pm1.0 Low, pm2.5 Low, pm10 Low, CO2 Low, TVOC Low, CH2O Low, CO Low, O3 Low, NO2 Low, RH Med.;

pm1.0 7 ug/m3, pm2.5 9 ug/m3, pm10 9 ug/m3, CO2 526 ppm, TVOC 0 lvl, CH2O 6 ug/m3, CO 0.5 ppm, O3 20 ppb, NO2 10 ppb, 27.1 *C, 46 RH;`

