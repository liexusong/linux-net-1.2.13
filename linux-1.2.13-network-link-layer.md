# Linux-1.2.13 网络原理

## 接收数据包过程

当内核从网卡处接收到数据包时，会触发以下过程：

```c
networkcard_driver(): <skb>
         \_> netif_rx(skb)
                  \__> skb_queue_tail(&backlog, skb)
                   \_> mark_bh(NET_BH)

```

* 网卡接收到数据包时，会触发中断处理程序，中断处理程序从网卡中读取到数据后封装成一个 `sk_buff` 数据包对象，然后调用 `netif_rx(skb)` 函数把数据包对象发送给内核。
* `netif_rx()` 会调用 `skb_queue_tail(&backlog, skb)` 函数把数据包对象添加到 `backlog` 队列中。
* `netif_rx()` 最后会调用 `mark_bh(NET_BH)` 来触发 `net_bh()` 中断下半部处理函数。

所以，对于处理接收到的数据包是通过 `net_bh()` 中断下半部处理函数来进行的。

## 网络中断下半部处理

我们暂时只分析接收数据的过程，我们一步步来分析：

```c
void net_bh(void *tmp)
{
    struct sk_buff *skb;
    ...
    while ((skb = skb_dequeue(&backlog)) != NULL)
    {
        ...
    }
    ...
}
```

首先，`net_bh()` 网络中断下半部处理例程通过调用 `skb_dequeue(&backlog)` 函数从 `backlog` 队列中获取一个数据包对象。接着我们继续分析：

```c
void net_bh(void *tmp)
{
    struct sk_buff *skb;
    ...
    while ((skb = skb_dequeue(&backlog)) != NULL)
    {
        backlog_size--;

        sti();

        // 去掉链路层头部
        skb->h.raw = skb->data + skb->dev->hard_header_len;
        skb->len -= skb->dev->hard_header_len;

        type = skb->dev->type_trans(skb, skb->dev); // 解析出网络层协议类型
        ...
    }
    ...
}
```

接着，去掉链路层头部（如以太网头部），然后调用接收设备绑定的 `type_trans()` 方法来解析出网络层协议类型，如以太网的 `type_trans()` 方法对应的是 `eth_type_trans()` 函数。

`eth_type_trans()` 函数处理分析出网络层协议类型外，还会判断数据包是否广播包、多播包或者发送给其他机器的包，代码如下：

```c
unsigned short eth_type_trans(struct sk_buff *skb, struct device *dev)
{
    struct ethhdr *eth = (struct ethhdr *) skb->data;
    unsigned char *rawp;

    if (*eth->h_dest&1)
    {
        if (memcmp(eth->h_dest,dev->broadcast, ETH_ALEN)==0)
            skb->pkt_type = PACKET_BROADCAST; // 广播数据
        else
            skb->pkt_type = PACKET_MULTICAST; // 多播数据
    }

    if (dev->flags&IFF_PROMISC)
    {
        if (memcmp(eth->h_dest,dev->dev_addr, ETH_ALEN))
            skb->pkt_type = PACKET_OTHERHOST; // 不是给本机的数据
    }

    if (ntohs(eth->h_proto) >= 1536)
        return eth->h_proto; // 返回网络层协议类型
    ...
}
```

一般来说，网络层协议有 `IP协议`、`ARP协议`、`IPX协议` 等，我们接着分析 `net_bh()` 的处理过程：

```c
void net_bh(void *tmp)
{
    struct sk_buff *skb;
    ...
    while((skb = skb_dequeue(&backlog)) != NULL)
    {
        ...
        pt_prev = NULL;
        for (ptype = ptype_base; ptype != NULL; ptype = ptype->next)
        {
            if ((ptype->type == type || ptype->type == htons(ETH_P_ALL))
                && (!ptype->dev || ptype->dev == skb->dev))
            {
                if(pt_prev)
                {
                    struct sk_buff *skb2;

                    skb2 = skb_clone(skb, GFP_ATOMIC);
                    if(skb2)
                        pt_prev->func(skb2, skb->dev, pt_prev);
                }

                pt_prev=ptype;
            }
        }

        if(pt_prev)
            pt_prev->func(skb, skb->dev, pt_prev);
        ...
    }
    ...
}
```

获取到网络层协议类型后，通过遍历 `ptype_base` 列表（这个列表保存了所有网络层协议对应的处理方法），找到网络层协议类型对应的处理方法，然后调用其处理方法处理数据包。如 `IP协议` 对应的处理方法就是 `ip_rcv()`。

当把数据包交给网络层处理方法后，数据包就由网络层继续处理。
