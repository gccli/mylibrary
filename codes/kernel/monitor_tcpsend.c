#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/inet.h>
#include <linux/net.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <net/sock.h>
#include <linux/skbuff.h>

asmlinkage int jtcp_sendmsg(struct kiocb *iocb, struct sock *sk, struct msghdr *msg,
			    size_t size)
{
    struct iovec *iov;
    struct tcp_sock *tp = tcp_sk(sk);
    struct sk_buff *skb;
    struct iphdr *iph;
    struct tcphdr *tcph;
    unsigned char u0,u1,u2,u3;
    unsigned short port=0;
    int iovlen, err;

    err = 0;

//    if (sk->sk_err || (sk->sk_shutdown & SEND_SHUTDOWN)) {
//	printk(KERN_INFO "sk error \n");
//	goto out;
//    }
//    if (((1 << sk->sk_state) & ~TCPF_ESTABLISHED)) {
//	printk(KERN_INFO "No established\n");
//	goto out;
//    }

    iovlen = msg->msg_iovlen;
    iov = msg->msg_iov;
    skb = skb_peek_tail(&sk->sk_receive_queue);
    if (skb) {
	if ((iph = ip_hdr(skb)) == NULL) {
	    printk(KERN_INFO "No ip header found\n");
	    goto out;
	}
	if ((tcph = tcp_hdr(skb)) == NULL) {
	    printk(KERN_INFO "No ip header found\n");
	    goto out;
	}
    } else {
	printk(KERN_INFO "No skb found\n");
	goto out;
    }

    printk(KERN_INFO "Receive TCP data, iov %d size %zu protocol %d\n", 
	   iovlen, size, iph->protocol);

    u0 = iph->saddr&0xff;
    u1 = (iph->saddr&0xff00) >> 8;
    u2 = (iph->saddr&0xff0000) >> 8*2;
    u3 = (iph->saddr&0xff000000) >> 8*3;
    port = tcph->source;
    printk("  srcaddr %d.%d.%d.%d:%d\n", u0,u1,u2,u3,port);

    u0 = iph->daddr&0xff;
    u1 = (iph->daddr&0xff00) >> 8;
    u2 = (iph->daddr&0xff0000) >> 8*2;
    u3 = (iph->daddr&0xff000000) >> 8*3;
    port = tcph->dest;
    printk(KERN_INFO "  dstaddr %d.%d.%d.%d:%d\n", u0,u1,u2,u3,port);

out:
    jprobe_return();

    return err;
}



static struct jprobe my_jprobe = {
    .entry = jtcp_sendmsg,
    .kp.symbol_name = "tcp_sendmsg",
};

static int __init jprobe_init(void)
{
    int ret;
//    my_jprobe.kp.symbol_name = "sock_common_recvmsg";

    ret = register_jprobe(&my_jprobe);
    if (ret < 0) {
	printk(KERN_INFO "register_jprobe failed, returned %d\n", ret);
	return -1;
    }
    printk(KERN_INFO "Planted jprobe at %p, handler addr %p\n",
	   my_jprobe.kp.addr, my_jprobe.entry);
    return 0;
}

static void __exit jprobe_exit(void)
{
    unregister_jprobe(&my_jprobe);
    printk(KERN_INFO "jprobe at %p unregistered\n", my_jprobe.kp.addr);
}

module_init(jprobe_init)
module_exit(jprobe_exit)
MODULE_LICENSE("GPL");
