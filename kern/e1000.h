#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H
#include <kern/pci.h>
#include <kern/e1000_hw.h>
#include <kern/pmap.h>
#include <inc/error.h>
#define TX_DESC_NUM 64
int e1000_init(struct pci_func *pcif);
int transmit(uint32_t addr,uint32_t length);
void transmit_test();
volatile uint32_t *lae1000;

struct e1000_tx_desc tx_desc[TX_DESC_NUM];

#endif	// JOS_KERN_E1000_H
