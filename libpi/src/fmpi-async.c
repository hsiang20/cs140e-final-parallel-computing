#include "fmpi-async.h"

static sw_uart_t u_async[MAX_RPIS];
static int _rank, _size, _root;


/* async message passing */
void FMPI_Init_async(int rank, int size, int root) {
    assert(size <= MAX_RPIS);
    _rank = rank;
    _size = size;
    _root = root;
    if (_rank == _root) {
        int idx = 0;
        for (int i = 0; i < _size; i++) {
            if (i == _root) continue;
            u_async[i] = sw_uart_init(TX_ASYNC, RX_ASYNC, BAUD_RATE);
            idx++;
        }
    } else {
        u_async[_root] = sw_uart_init(TX_ASYNC, RX_ASYNC, BAUD_RATE); 
    }
}

void send_async(void *buffer, int recv_pi, int count, int data_size) {
    for (int i = 0; i < count * data_size; i++) {
        sw_uart_put8(&u_async[recv_pi], ((uint8_t *)buffer)[i]);
        // printk("SEND_ASYNC: sent %x\n", ((uint8_t *)buffer)[i]);
    }
}

void recv_async(uint8_t *buffer, int send_pi, int count, int data_size) {
    for (int i = 0; i < count * data_size; i++) {
        buffer[i] = sw_uart_get8(&u_async[send_pi]);
    }
    // printk("RECV_ASYNC finished! got data %x %x %x %x\n", buffer[5], buffer[6], buffer[7], buffer[8]);
    
}

void send_signal_async(int recv_pi, uint8_t signal) {
    sw_uart_put8(&u_async[recv_pi], signal);
}

void wait_signal_async(int send_pi, uint8_t signal) {
    while (sw_uart_get8(&u_async[send_pi]) != signal)
        ;
}

int wait_signal_timeout_async(int recv_pi, uint8_t signal, uint32_t msec) {
    return sw_uart_get8_timeout(&u_async[recv_pi], 1000*msec);
}

void sync_me_last(int recv_pi) {
    for (int i = 0; i < _size; i++) {
        if (i == _rank) continue;
        do {
            send_signal_async(recv_pi, SYNC_SIGNAL);
        } while (wait_signal_timeout_async(recv_pi, SYNC_SIGNAL, TIMEOUT_MS) != SYNC_SIGNAL);
    }
}

int sync_receiver(int send_pi) {
    int get_signal = wait_signal_timeout_async(send_pi, SYNC_SIGNAL, 1000);
    if (get_signal == -1) return 0;
    delay_ms(DELAY_MS);
    send_signal_async(send_pi, SYNC_SIGNAL);
    return 1;
}

void FMPI_PUT(int recv_pi, uint32_t *buffer, uint32_t *address) {
    Packet p;
    p.data = *buffer;
    p.address = *address;
    p.command = 1;
    
    int data_size = 9;
    // gpio_set_on(TX_ASYNC);
    gpio_set_off(TX_ASYNC);
    sync_me_last(recv_pi);
    send_async((uint8_t *)&p, recv_pi, 1, data_size);
}

uint32_t FMPI_GET(int recv_pi, uint32_t *address) {
    Packet p;
    p.data = 0;
    p.address = *address;
    p.command = 2;
    
    int data_size = 9;
    // gpio_set_on(TX_ASYNC);
    gpio_set_off(TX_ASYNC);
    sync_me_last(recv_pi);
    send_async((uint8_t *)&p, recv_pi, 1, data_size);

    uint8_t recv_p[4];
    recv_async(recv_p, recv_pi, 4, 1);
    return *(uint32_t *)recv_p;
}
