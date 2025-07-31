#ifndef ZPHS01B_H
#define ZPHS01B_H

#include <stdint.h>

/**
 * @brief Inicializa o driver da UART para comunicação com o sensor.
 * Esta função deve ser chamada apenas uma vez no início do programa.
 */
void zphs01b_uart_init(void);

/**
 * @brief Cria e inicia a tarefa do sensor com um intervalo de tempo específico.
 * Se uma tarefa antiga existir, ela é deletada primeiro.
 * @param delay_ms Intervalo de tempo entre as leituras.
 */
void init_and_run_zphs01b(uint32_t delay_ms);

/**
 * @brief Para e deleta a tarefa do sensor.
 */
void stop_zphs01b_task(void);

#endif /* ZPHS01B_H */