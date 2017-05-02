#include "ns.h"

extern union Nsipc nsipcbuf;
#define MAX_RX_BUF 2048

typedef struct {
        char buf[MAX_RX_BUF];
} rx_packet_t;
rx_packet_t rx_local_buf;
void
sleep(int msec)
{
    unsigned now = sys_time_msec();
    unsigned end = now + msec;

    if ((int)now < 0 && (int)now > -MAXERROR)
        panic("sys_time_msec: %e", (int)now);
    if (end < now)
        panic("sleep: wrap");

    while (sys_time_msec() < end)
        sys_yield();
}
void
input(envid_t ns_envid)
{
	binaryname = "ns_input";
	int  r =0;
	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	while (1) {
        while((r = sys_receive(rx_local_buf.buf)) == -E_RECEIVE_RETRY){
            continue;
        }
        if (r < 0)
            panic("input error:%e",r);
        nsipcbuf.pkt.jp_len= r;
        memcpy(nsipcbuf.pkt.jp_data ,rx_local_buf.buf,nsipcbuf.pkt.jp_len);
        ipc_send(ns_envid, NSREQ_INPUT, (void *)&nsipcbuf, PTE_P|PTE_W|PTE_U);
        sleep(50);
		//sys_page_unmap(0, &nsipcbuf);
    }
}
