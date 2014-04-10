
#include "config.h"
#if USE_ETHERNET
#include "drv_emac.h"
#endif

void init_comms(void)
{
#if USE_ETHERNET
    init_emac();
#endif
}

unsigned getline(char *buf, int max_len, int *count)
{
#if USE_ETHERNET
    return (emac_getline(buf, max_len, count));
#endif
}

unsigned sendline_crlf(char *buf)
{
#if USE_ETHERNET
    return (emac_sendline_crlf(buf));
#endif
}

void comm_handshake(void)
{

#if USE_ETHERNET
    emac_handshake();
#endif

}
