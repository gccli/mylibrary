#ifndef RTLAYER_H_
#define RTLAYER_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

struct rtlhdr {
	uint32_t         sid;
	uint32_t         seq; /* sequence */ 
	uint32_t         ts;  /* timestamp */
};


struct rtlrtt {
  float		rtt_rtt;	/* most recent measured RTT, in seconds */
  float		rtt_srtt;	/* smoothed RTT estimator, in seconds */
  float		rtt_rttvar;	/* smoothed mean deviation, in seconds */
  float		rtt_rto;	/* current RTO to use, in seconds */
  int		rtt_nrexmt;	/* # times retransmitted: 0, 1, 2, ... */
  uint32_t	rtt_base;	/* # sec since 1/1/1970 at start */
};

#define	RTT_RXTMIN      2	/* min retransmit timeout value, in seconds */
#define	RTT_RXTMAX     60	/* max retransmit timeout value, in seconds */
#define	RTT_MAXNREXMT 	3	/* max # times to retransmit */


typedef struct RTL_SESSION
{
	uint32_t         sid;
	uint32_t         seq; /* sequence */ 
	uint32_t         ts;  /* timestamp */

	uint32_t         remote_sid;
	struct sockaddr *remote_addr;
	socklen_t        remote_addrlen;

	struct rtlrtt rtt;
	unsigned char   rtt_init;
	timer_t         timer;

	int sock;
	unsigned char *msg;
	int msglen;
} Session;


inline bool operator == (const Session& s1, const Session& s2)
{
	return s1.sid == s2.sid;
}

/* function prototypes */
void	 rtt_debug(struct rtlrtt *);
void	 rtt_init(struct rtlrtt *);
void	 rtt_newpack(struct rtlrtt *);
int		 rtt_start(struct rtlrtt *);
void	 rtt_stop(struct rtlrtt *, uint32_t);
int		 rtt_timeout(struct rtlrtt *);
uint32_t rtt_ts(struct rtlrtt *);

extern int	rtt_d_flag;	/* can be set to nonzero for addl info */





/*      // old
extern int errno;

enum {
  MSGTYPE_DATA = 0x00,
  MSGTYPE_ACK  = 0x01
};

// reliable transport control block
struct RTCB
{
	int        sockfd;
	uint32_t   seqno;
	uint32_t   ack;
	uint32_t   ack_rcv;
};

#pragma pack(1)
struct RTHDR
{
	uint16_t   reserve:4;
	uint16_t   type:4;
	uint16_t   flags:8;
	uint32_t   seqno;
	uint32_t   ack;
	uint16_t   win;
};
#pragma pack(0)


#define SEQ_LT(x,y)          ((int)((x) - (y)) < 0) 
#define SEQ_LET(x,y)         ((int)((x) - (y)) <= 0)
#define SEQ_GT(x,y)          ((int)((x) - (y)) > 0)
#define SEQ_GET(x,y)         ((int)((x) - (y)) >= 0)
*/

#endif // RTLAYER_H_

