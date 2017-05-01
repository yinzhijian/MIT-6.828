#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H
#include <kern/pci.h>
#include <kern/e1000_hw.h>
#include <kern/pmap.h>
#include <inc/error.h>
#include <inc/string.h>
#define TX_DESC_NUM 64
#define MAX_TX_BUF 1518
#define MAX_RX_BUF 2048 /* required by E1000_RCTL_SZ_2048 */
int e1000_init(struct pci_func *pcif);
int transmit(void *addr,uint32_t length);
void transmit_test();
volatile uint32_t *lae1000;

struct e1000_tx_desc tx_desc[TX_DESC_NUM];
typedef struct {
        uint8_t buf[MAX_TX_BUF];
} tx_packet_t;

typedef struct {
        uint8_t buf[MAX_RX_BUF];
} rx_packet_t;

tx_packet_t tx_buf[TX_DESC_NUM];
//rx_packet_t rx_buf[MAXRXD];

#endif	// JOS_KERN_E1000_H
