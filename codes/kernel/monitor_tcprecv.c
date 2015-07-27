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

asmlinkage int jtcp_recvmsg(struct kiocb *iocb, struct sock *sk, struct msghdr *msg,
		size_t len, int nonblock, int flags, int *addr_len)
{
    int i, ioffs, iovlen, err;
    struct iovec *iov;
    struct sk_buff *skb;
    struct iphdr *iph;
    struct tcphdr *tcph;
    unsigned char u0,u1,u2,u3;
    unsigned short port=0;
    char temp[128], *base, *ptr;
    err = 0;
    ioffs=0;

#ifdef CONFIG_NET_DMA
    skb = skb_peek_tail(&sk->sk_receive_queue);
#endif

    if (skb) {
	if ((iph = ip_hdr(skb)) == NULL) {
	    printk(KERN_INFO "No ip header found\n");
	    goto out;
	}
	if ((tcph = tcp_hdr(skb)) == NULL) {
	    printk(KERN_INFO "No tcp header found\n");
	    goto out;
	}
    } else {
	printk(KERN_INFO "No skb found\n");
	goto out;
    }
    iovlen = msg->msg_iovlen;
    iov = msg->msg_iov;

    memset(temp, 0, sizeof(temp));
    if (iovlen > 0) {
	base = (char *)iov[0].iov_base;
    }

    for(i=0; iovlen > 0 && i<sizeof(temp)-2; i+=2) {
	if (base[i/2] >= 0x20 && base[i/2] <0x7f)
	    temp[i/2] = base[i/2];
	else
	    temp[i/2] = '.';
    }
    if (iovlen > 0) {
	err = iov[0].iov_len;
    }
    if (ntohs(tcph->source) != 80) {
	goto out;
    }

//    printk(KERN_INFO "Receive TCP data, iov[%d] size %d cap %zu, protocol %d\n",
//	   iovlen, err, len, iph->protocol);

    u0 = iph->saddr&0xff;
    u1 = (iph->saddr&0xff00) >> 8;
    u2 = (iph->saddr&0xff0000) >> 8*2;
    u3 = (iph->saddr&0xff000000) >> 8*3;
    port = ntohs(tcph->source);
//    printk("  srcaddr %d.%d.%d.%d:%d\n", u0,u1,u2,u3,port);

    u0 = iph->daddr&0xff;
    u1 = (iph->daddr&0xff00) >> 8;
    u2 = (iph->daddr&0xff0000) >> 8*2;
    u3 = (iph->daddr&0xff000000) >> 8*3;
    port = ntohs(tcph->dest);
//    printk(KERN_INFO "  dstaddr %d.%d.%d.%d:%d\n", u0,u1,u2,u3,port);
//    printk(KERN_INFO "  buffer %s\n", temp);


    if ((ptr = strstr(base, "200")) != NULL) {
	printk(KERN_INFO "Receive TCP data, iov[%d] size %d cap %zu, protocol %d\n",
	       iovlen, err, len, iph->protocol);
	printk(KERN_INFO "  buffer %s\n", temp);
	printk(KERN_INFO "!!!! change bytes\n");
	ptr[0] = '4';	ptr[1] = '0';	ptr[2] = '4';
    }

out:
    jprobe_return();

    return err;
}



static struct jprobe my_jprobe = {
    .entry = jtcp_recvmsg,
    .kp.symbol_name = "tcp_recvmsg",
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
