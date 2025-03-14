#include "fmpi_async.h"

static sw_uart_t u, u_async;
static int _rank, _size, _root;


/* async message passing */
void FMPI_Init_async(int rank, int size, int root) {
    u_async = sw_uart_init(TX_ASYNC, RX_ASYNC, BAUD_RATE);   
    _rank = rank;
    _size = size;
    _root = root;
}

void send_async(void *buffer, int count) {
    for (int i = 0; i < count; i++) {
        sw_uart_put8(&u_async, ((data_type *)buffer)[i]);
    }
}

void recv_async(void *buffer, int count) {
    for (int i = 0; i < count; i++) {
        int val = sw_uart_get8(&u_async);
        ((data_type *)buffer)[i] = val;
    }
}

void send_signal_async(uint8_t signal) {
    sw_uart_put8(&u_async, signal);
}

void wait_signal_async(uint8_t signal) {
    while (sw_uart_get8(&u_async) != signal)
        ;
}

int wait_signal_timeout_async(uint8_t signal, uint32_t msec) {
    return sw_uart_get8_timeout(&u_async, 1000*msec);
}

void sync_me_last() {
    printk("_size: %d\n", _size);
    for (int i = 0; i < _size; i++) {
        if (i == _rank) continue;
        do {
            send_signal_async(SYNC_SIGNAL);
        } while (wait_signal_timeout_async(SYNC_SIGNAL, TIMEOUT_MS) != SYNC_SIGNAL);
    }
}

void sync_receiver() {
    int get_signal = wait_signal_timeout_async(SYNC_SIGNAL, 1000);
    if (get_signal == -1) return;
    delay_ms(DELAY_MS);
    send_signal_async(SYNC_SIGNAL);
}

void FMPI_PUT(void *buffer, int count) {
    sync_me_last();
    // printk("sync: %d after sync_me_last!\n", _rank);
    send_async(buffer, count);
}