/*
 * INET		An implementation of the TCP/IP protocol suite for the LINUX
 *		operating system.  INET is implemented using the  BSD Socket
 *		interface as the means of communication with the user level.
 *
 *		Definitions for the TCP protocol.
 *
 * Version:	@(#)tcp.h	1.0.2	04/28/93
 *
 * Author:	Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 */
#ifndef _LINUX_TCP_H
#define _LINUX_TCP_H


#define HEADER_SIZE	64		/* maximum header size		*/


struct tcphdr {
	__u16	source;    // 来源端口
	__u16	dest;      // 目标端口
	__u32	seq;       // 发送方的序列号
	__u32	ack_seq;   // 回复接收方 ack 的序列号
#if defined(LITTLE_ENDIAN_BITFIELD)
	__u16	res1:4,
		doff:4,
		fin:1,
		syn:1,
		rst:1,
		psh:1,
		ack:1,
		urg:1,
		res2:2;
#elif defined(BIG_ENDIAN_BITFIELD)
	__u16	doff:4,    // header的长度(需要乘以4)
		res1:4,
		res2:2,
		urg:1,    // 是否紧急数据
		ack:1,    // 是否ack包
		psh:1,    // 是否立刻处理数据
		rst:1,    // 是否reset包
		syn:1,    // 是否sync包
		fin:1;    // 是否finish包
#else
#error	"Adjust your <asm/byteorder.h> defines"
#endif
	__u16	window;  // 对端窗口还有多少空间
	__u16	check;   // 校验和
	__u16	urg_ptr; // 紧急指针
};


enum {
  TCP_ESTABLISHED = 1,
  TCP_SYN_SENT,
  TCP_SYN_RECV,
  TCP_FIN_WAIT1,
  TCP_FIN_WAIT2,
  TCP_TIME_WAIT,
  TCP_CLOSE,
  TCP_CLOSE_WAIT,
  TCP_LAST_ACK,
  TCP_LISTEN,
  TCP_CLOSING	/* now a valid state */
};

#endif	/* _LINUX_TCP_H */
