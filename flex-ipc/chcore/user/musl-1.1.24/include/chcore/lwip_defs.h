#pragma once

#ifdef LWIP_SRC
#include <lwip/sockets.h>
#else /* in musl-libc */
#include <bits/errno.h>
#include <string.h>
#include <sys/socket.h>
#endif

#include <chcore/defs.h>
#include <chcore/syscall.h>
#include <stdio.h>

#define ARGS_NUM 8
#define LWIP_ARG_LEN  (sizeof(enum LWIP_REQ) + sizeof(unsigned long) * ARGS_NUM)
#define LWIP_DATA_LEN 2048

enum LWIP_REQ {
	LWIP_CREATE_SOCKET, //	0
	LWIP_SOCKET_SOPT,	// 1
	LWIP_SOCKET_GOPT,	// 2
	LWIP_SOCKET_NAME,	// 3
	LWIP_SOCKET_PEER,	// 4
	LWIP_SOCKET_BIND,	// 5
	LWIP_SOCKET_RECV,	// 6
	LWIP_SOCKET_RMSG,	// 7
	LWIP_SOCKET_SEND,	// 8
	LWIP_SOCKET_SMSG,	// 9
	LWIP_SOCKET_LIST,	// 10
	LWIP_SOCKET_CONN,	// 11
	LWIP_SOCKET_ACPT,	// 12
	LWIP_SOCKET_CLSE,	// 13
	LWIP_SOCKET_STDW,	// 14
	LWIP_SOCKET_READ,	// 15
	LWIP_SOCKET_WRITE,	// 16
	LWIP_REQ_SOCKET_POLL,	// 17
	LWIP_SOCKET_IOCTL,	// 18
};

enum LWIP_REQ_STATE
{
	LWIP_REQ_INIT,
	LWIP_REQ_SUBMITTED,
	LWIP_REQ_BUSY,
	LWIP_REQ_DONE
};

struct lwip_request {
	enum LWIP_REQ req;
	enum LWIP_REQ_STATE req_state;
	unsigned long args[ARGS_NUM];
	char data[LWIP_DATA_LEN];
	u64 ret;
};

typedef struct flex_lwip_handler_arg {
	u64 server_shm_addr;
	u64 client_badge;
} flex_lwip_handler_arg_t;

/* Calculate the total length of msghdr data structure */
static inline int get_msghdr_size(struct msghdr *msg)
{
	struct iovec *msg_iov;
	int total_len = 0;
	int i = 0;

	total_len = sizeof(struct msghdr);
	if (msg->msg_namelen && !msg->msg_name)
		return -EINVAL;

	total_len += msg->msg_namelen;

	msg_iov = msg->msg_iov;
	if (msg->msg_iovlen && !msg_iov)
		return -EINVAL;
	total_len += msg->msg_iovlen * sizeof(struct iovec);

	for (i = 0; i < msg->msg_iovlen; i++) {
		total_len += msg_iov[i].iov_len;
	}

	if (msg->msg_controllen && !msg->msg_control)
		return -EINVAL;
	total_len += msg->msg_controllen;
	return total_len;
}

/* Internal memory copy for pack_message
 * when pmo_cap > 0:
 * 	memcpy to pmo, dst is the offset
 * else
 * 	memcpy to dst
 */
static inline void __pack_mem_cpy(void *dst, void *src, ssize_t len,
				  int pmo_cap)
{
	if (len == 0)
		return; 
	if (pmo_cap < 0) {
		memcpy(dst, src, len);
	} else {
		usys_write_pmo(pmo_cap, (u64)dst, src, len);
	}
}

static inline int pack_message(struct msghdr *msg, char *dst, int dst_len,
			       int copy_data, int *shared_pmo_ptr)
{
	char *data_ptr = 0;
	struct iovec *msg_iov;
	int copy_len = 0;
	int shared_pmo = 0;
	int i = 0;
	int total_len = dst_len;

	if (total_len < 0) /* ERROR */
		return -EINVAL;

	if (total_len > LWIP_DATA_LEN) {
		shared_pmo = usys_create_pmo(total_len, PMO_DATA);
		*shared_pmo_ptr = shared_pmo;
		if (shared_pmo < 0)
			return -ENOMEM;
		data_ptr = 0; /* offset */
	} else {
		shared_pmo = -1;
		data_ptr = dst; /* dst */
	}

	/* Copy message to the data */
	/* Copy msg_hdr struct */
	copy_len = sizeof(struct msghdr);
	__pack_mem_cpy(data_ptr, msg, copy_len, shared_pmo);
	// printf("msg_hdr %lx data %lx\n", data_ptr, copy_len);
	data_ptr += copy_len;


	/* Copy msg_name */
	copy_len = msg->msg_namelen;
	/* Only copy the msg_name when needed */
	if (copy_data)
		__pack_mem_cpy(data_ptr, msg->msg_name, copy_len, shared_pmo);
	// printf("msg_hdr %lx data %lx\n", data_ptr, copy_len);
	/* Update the pointer */
	data_ptr += copy_len;

	/* Copy msg_iov */
	msg_iov = msg->msg_iov;
	/* Copy msg_iov struct */
	copy_len = msg->msg_iovlen * sizeof(struct iovec);
	__pack_mem_cpy(data_ptr, msg_iov, copy_len, shared_pmo);
	/* Update the pointer */
	// printf("msg_iov %lx data %lx\n", data_ptr, copy_len);
	data_ptr += copy_len;

	for (i = 0; i < msg->msg_iovlen; i++) {
		copy_len = msg_iov[i].iov_len;
		if (copy_data)
			__pack_mem_cpy(data_ptr, msg_iov[i].iov_base, copy_len,
				       shared_pmo);
		/* Update the pointer */
		// printf("msg_iov[%d] %lx data %lx\n", i, data_ptr,
		// copy_len);
		data_ptr += copy_len;
	}

	/* Copy the msg_control */
	copy_len = msg->msg_controllen;
	/* Only copy the msg_control when needed */
	if (copy_data)
		__pack_mem_cpy(data_ptr, msg->msg_control, copy_len,
			       shared_pmo);
	/* Update the pointer */
	data_ptr += copy_len;
	return 0;
}

static inline int unpack_message(struct msghdr *msg, char *src)
{
	char *data_ptr = src;
	struct iovec *msg_iov;
	int copy_len = 0;
	int i = 0;

	/* Skip the metadata */
	copy_len = sizeof(struct msghdr);
	data_ptr += copy_len;

	copy_len = msg->msg_namelen;
	/* " If the socket is connected, the msg_name and msg_namelen
	 * members shall be ignored." */
	/* update the pointer */
	data_ptr += copy_len;

	/* Skip msg_iov */
	msg_iov = msg->msg_iov;
	/* Copy msg_iov struct */
	copy_len = msg->msg_iovlen * sizeof(struct iovec);
	data_ptr += copy_len;

	for (i = 0; i < msg->msg_iovlen; i++) {
		copy_len = msg_iov[i].iov_len;
		/* Only copy the msg_iov when needed */
		memcpy(msg_iov[i].iov_base, data_ptr, copy_len);
		/* update the pointer */
		// printf("msg_iov[%d] %lx data %lx\n", i, data_ptr, copy_len);
		data_ptr += copy_len;
	}

	copy_len = msg->msg_controllen;
	memcpy(msg->msg_control, data_ptr, copy_len);
	/* update the pointer */
	// printf("msg_control %lx data %lx\n", data_ptr, copy_len);
	data_ptr += copy_len;
	return 0;
}

static inline int update_msg_ptr(struct msghdr *msg)
{
	char *data_ptr = (char *)msg;
	int copy_len = 0;
	int i = 0;

	/* msg_hdr struct */
	copy_len = sizeof(struct msghdr);
	// printf("msg_hdr %lx data %lx\n", data_ptr, copy_len);
	data_ptr += copy_len;

	/* msg_name */
	copy_len = msg->msg_namelen;
	msg->msg_name = data_ptr;
	// printf("msg_name %lx data %lx\n", data_ptr, copy_len);
	/* update the pointer */
	data_ptr += copy_len;

	/* Copy msg_iov struct */
	copy_len = msg->msg_iovlen * sizeof(struct iovec);
	msg->msg_iov = (struct iovec *)data_ptr;
	// printf("msg_iov %lx data %lx\n", data_ptr,  copy_len);
	/* update the pointer */
	data_ptr += copy_len;

	for (i = 0; i < msg->msg_iovlen; i++) {
		copy_len = msg->msg_iov[i].iov_len;
		/* update the pointer */
		msg->msg_iov[i].iov_base = data_ptr;
		// printf("msg_iov[%d] %lx data %lx\n", i, data_ptr, copy_len);
		data_ptr += copy_len;
	}

	/* msg_control */
	copy_len = msg->msg_controllen;
	/* update the pointer */
	msg->msg_control = data_ptr;
	// printf("msg_control %lx data %lx\n", data_ptr, copy_len);
	data_ptr += copy_len;
	return 0;
}
