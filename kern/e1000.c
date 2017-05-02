#include <kern/e1000.h>
#include <kern/pci.h>
void transmit_test(){
    int r,i;
    for (i = 0;i <TX_DESC_NUM + 10;i++){
        do{
        r = transmit((void *)"hello",5);
        }while(r == -E_QUEUE_FULL);
    }
}
int transmit(void * addr,uint32_t length){
    if (length > MAX_TX_BUF)
        return -E_INVAL;
    uint32_t tail = lae1000[E1000_TDT/4];
    //check tx_desc array is full
    if ((tx_desc[tail].lower.data & E1000_TXD_CMD_RS)  && (tx_desc[tail].upper.data & E1000_TXD_STAT_DD) == 0)
        return -E_QUEUE_FULL;
    memcpy((void *)tx_buf[tail].buf,addr,length);
    tx_desc[tail].buffer_addr = PADDR((void *)tx_buf[tail].buf);
    //set Report Status bit.When set, the Ethernet controller needs to report the status information
    tx_desc[tail].lower.data = E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
    tx_desc[tail].lower.flags.length = length;
    //reset upper data
    tx_desc[tail].upper.data = 0;
    // update TDT
    lae1000[E1000_TDT/4] = (tail + 1) % TX_DESC_NUM;
    return 0;
}
int 
receive(void *addr){
    static uint32_t next = 0;
    int len = 0;
    //cprintf("receive tail:%08x,head:%08x\n",lae1000[E1000_RDT/4],lae1000[E1000_RDH/4]);
    if ((uint32_t)addr + MAX_RX_BUF >= UTOP)
        return -E_INVAL;
    //check rx_desc array is empty
    if (!(rx_desc[next].status&E1000_RXD_STAT_DD))
        return -E_RECEIVE_RETRY;

    assert(rx_desc[next].status & E1000_RXD_STAT_EOP);
    memcpy(addr,(void *)rx_buf[next].buf,rx_desc[next].length);
    len = rx_desc[next].length;
    //reset receive status
    rx_desc[next].status = 0;
    rx_desc[next].length = 0;
    rx_desc[next].csum = 0;
    rx_desc[next].errors = 0;
    rx_desc[next].special = 0;
    // update RDT
    lae1000[E1000_RDT/4] = next;
    next = (next + 1)%RX_DESC_NUM;
    return len;
}

int 
e1000_init(struct pci_func *pcif){
    int i = 0;
    pci_func_enable(pcif);
    cprintf("e1000 base:%08x,size:%08x\n",pcif->reg_base[0],pcif->reg_size[0]);
    lae1000 = mmio_map_region(pcif->reg_base[0],pcif->reg_size[0]);
    if (lae1000[E1000_STATUS/4] != 0x80080783)
        panic("E1000_STATUS != 0x80080783");
    //Transmit Initialization
    static_assert((uint32_t)tx_desc % 16 == 0);
    lae1000[E1000_TDBAL/4] = (uint32_t)(PADDR((void *)tx_desc));
    //Transmit Descriptor Length 
    static_assert(sizeof(tx_desc) % 128 == 0);
    lae1000[E1000_TDLEN/4] = sizeof(tx_desc);
    //Transmit Descriptor Head
    lae1000[E1000_TDH/4] = 0;
    //Transmit Descriptor Tail
    lae1000[E1000_TDT/4] = 0;
    //transmit control
    //Pad Short Packets 
    lae1000[E1000_TCTL/4] |= E1000_TCTL_PSP;
    //Collision Threshold only has meaning in half duplex mode
    lae1000[E1000_TCTL/4] |= 0x10 << 4;
    // Collision Distance full duplex
    lae1000[E1000_TCTL/4] |= 0x40 << 12;
    //For the IEEE 802.3 standard IPG value of 96-bit time, the value that should be programmed into IPGT is 10
    lae1000[E1000_TIPG/4] = 10;
    //enable for normal operation
    lae1000[E1000_TCTL/4] |= E1000_TCTL_EN;

    //Receive Initialization
    // set Receive Address to 52:54:00:12:34:56
    lae1000[E1000_RA] = 0x12004552;
    lae1000[E1000_RA+1] = 0x5634;
    // set Receive descriptor valid
    lae1000[E1000_RA/4+1] |= E1000_RAH_AV;

    // set Multicast Table Array to 0
    for (i = 0;i<128;i++)
        lae1000[E1000_MTA/4 + i] = 0;
    static_assert((uint32_t)rx_desc % 16 == 0);
    lae1000[E1000_RDBAL/4] = (uint32_t)(PADDR((void *)rx_desc));
    lae1000[E1000_RDBAH/4] = 0;
    static_assert(sizeof(rx_desc) % 128 == 0);
    lae1000[E1000_RDLEN/4] = sizeof(rx_desc);

    // init rx queue's buf addr
    memset(rx_desc, 0, sizeof(rx_desc));

    for (i = 0; i < RX_DESC_NUM-1; ++i) {
        rx_desc[i].buffer_addr = PADDR(&rx_buf[i]);
    }
    //Receive Descriptor Head
    lae1000[E1000_RDH/4] = 0;
    //Receive Descriptor Tail
    lae1000[E1000_RDT/4] = RX_DESC_NUM-1;

    // reset receiver control reg
    lae1000[E1000_RCTL/4] = 0;
    // Long Packet Disable
    //lae1000[E1000_RCTL/4] &= ~E1000_RCTL_LPE;
    // Loopback Mode (RCTL.LBM) should be set to 00b for normal operation
    lae1000[E1000_RCTL/4] |= E1000_RCTL_LBM_NO;
    // the Receive Buffer Size
    lae1000[E1000_RCTL/4] |= E1000_RCTL_SZ_2048;
    // Strip Ethernet CRC 
    lae1000[E1000_RCTL/4] |= E1000_RCTL_SECRC;
    // receiver Enable
    lae1000[E1000_RCTL/4] |= E1000_RCTL_EN;

    return 0;
}
    

// LAB 6: Your driver code here
