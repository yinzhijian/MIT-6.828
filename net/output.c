#include "ns.h"

#define debug 0
extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	uint32_t req, whom;
	int perm, r;

	while (1) {
		perm = 0;
		req = ipc_recv((int32_t *) &whom, (void *)&nsipcbuf.pkt, &perm);
		if (debug)
			cprintf("ns output req %d from %08x [page %08x: %s]\n", req, whom, uvpt[PGNUM(&nsipcbuf)], nsipcbuf);

		// All requests must contain an argument page
		if (!(perm & PTE_P)) {
			cprintf("Invalid request from %08x: no argument page\n",
				whom);
			continue; // just leave it hanging...
		}

		if (req == NSREQ_OUTPUT) {
            do{
			    r = sys_transmit(nsipcbuf.pkt.jp_data,nsipcbuf.pkt.jp_len);
            }while(r == -E_QUEUE_FULL);
            if (r < 0)
                panic("output error:%e\n",r);
		} else {
			cprintf("Invalid request code %d from %08x\n", req, whom);
			r = -E_INVAL;
		}
		//ipc_send(whom, r, pg, perm);
		//sys_page_unmap(0, &nsipcbuf);
	}
}
