#include <kern/e1000.h>
#include <kern/pci.h>
static char test_packet[] = "test";
void transmit_test(){
    int r,i;
    for (i = 0;i <TX_DESC_NUM*2 + 10;i++){
        do{
        r = transmit((uint32_t)test_packet,5);
        }while(r == -E_QUEUE_FULL);
    }
}
int transmit(uint32_t addr,uint32_t length){
    if (length > 16288)
        return -E_INVAL;
    uint32_t tail = lae1000[E1000_TDT/4];
    //check tx_desc array is full
    if ((tx_desc[tail].lower.flags.cmd & E1000_TXD_CMD_RS)  && (tx_desc[tail].upper.fields.status & E1000_TXD_STAT_DD) == 0)
        return -E_QUEUE_FULL;
    tx_desc[tail].buffer_addr = addr;
    //reset lower data
    tx_desc[tail].lower.data = 0;
    tx_desc[tail].lower.flags.length = length;
    //set Report Status bit.When set, the Ethernet controller needs to report the status information
    tx_desc[tail].lower.flags.cmd |= E1000_TXD_CMD_RS;
    //reset upper data
    tx_desc[tail].upper.data = 0;
    // update TDT
    lae1000[E1000_TDT/4] = (tail + 1) % TX_DESC_NUM;
    return 0;
}
int 
e1000_init(struct pci_func *pcif){
    pci_func_enable(pcif);
    cprintf("e1000 base:%08x,size:%08x\n",pcif->reg_base[0],pcif->reg_size[0]);
    lae1000 = mmio_map_region(pcif->reg_base[0],pcif->reg_size[0]);
    cprintf("e1000 status:%08x\n",lae1000[E1000_STATUS/4]);
    //Transmit Initialization
    lae1000[E1000_TDBAL/4] = (uint32_t)tx_desc;
    //Transmit Descriptor Length 
    lae1000[E1000_TDLEN/4] = sizeof(struct e1000_tx_desc)*TX_DESC_NUM;
    //Transmit Descriptor Head
    lae1000[E1000_TDH/4] = 0;
    //Transmit Descriptor Tail
    lae1000[E1000_TDT/4] = 0;
    //transmit control
    //enable for normal operation
    lae1000[E1000_TCTL/4] |= E1000_TCTL_EN;
    //Pad Short Packets 
    lae1000[E1000_TCTL/4] |= E1000_TCTL_PSP;
    //Collision Threshold only has meaning in half duplex mode
    lae1000[E1000_TCTL/4] |= 0x10 << 4;
    // Collision Distance full duplex
    lae1000[E1000_TCTL/4] |= 0x40 << 12;
    //For the IEEE 802.3 standard IPG value of 96-bit time, the value that should be programmed into IPGT is 10
    lae1000[E1000_TIPG/4] = 10;

    return 0;
}
    

// LAB 6: Your driver code here
