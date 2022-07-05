#define _GNU_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <chcore/syscall.h>
#include <chcore/coroutine-jmp.h>

#include "tests.h"

#define THREAD_NUM	32
#define PLAT_CPU_NUM	4
#define TEST_NUM	1
static int thread_num = 1;
static int test_num = 1;
// static double test_time = 0;
static int read_count = 0;
static double read_time = 0;

void *client_routine(void *arg)
{
	int client_fd = 0;
	int ret = 0;
	char data_buf[NET_BUFSIZE];
	int i = 0;
	struct sockaddr_in target;
	struct msghdr msgrecv;
	struct iovec iovrecv[1];
	cpu_set_t mask;
	char server_ip[] = "127.0.0.1";
	int server_port = NET_SERVERPORT;
	int tid = (int)(long)arg;
	int offset = 0;
	int remain = 0;

	/* set aff */
	CPU_ZERO(&mask);
	CPU_SET(tid % PLAT_CPU_NUM, &mask);
	ret = sched_setaffinity(0, sizeof(mask), &mask);
	chcore_assert(ret == 0, "Set affinity failed!");
	sched_yield();

	for (i = 0; i < test_num; i++) {
		offset = 0;
		remain = NET_BUFSIZE;
		memset(data_buf, 0, sizeof(data_buf));

		do {
			client_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (client_fd >= 0) /* succ */
				break;
			printf("create socket failed! errno = %d\n", errno);
			sleep(1);
		} while (1);

		ret = setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1},
				 sizeof(int));
		if (ret < 0) {
			info("[client] set socketopt return %d\n", ret);
			return (void *)-1;
		}

		/* Client do not need to bind */

		memset(&target, 0, sizeof(struct sockaddr_in));
		target.sin_family = AF_INET;
		target.sin_addr.s_addr = inet_addr(server_ip);
		target.sin_port = htons(server_port);
		ret = connect(client_fd, (struct sockaddr *)&target,
			      sizeof(struct sockaddr_in));
		if (ret < 0) {
			info("[client] cannot connect %d!\n", ret);
			return (void *)-1;
		}

		int mode = 0;
		ret = ioctl(client_fd, FIONBIO, &mode);
		chcore_assert(ret == 0, "[client] ioctl error!");

		if (i % 3 == 0) {
			while (remain > 0) {
				ret = recv(client_fd, data_buf + offset,
					   sizeof(data_buf) - offset, 0);
				chcore_assert(
				    ret >= 0,
				    "[client] receive message error!");
				remain -= ret;
				offset += ret;
			}
		} 
		else if (i % 3 == 1) {
			while (remain > 0) {
				ret = read(client_fd, data_buf + offset,
					   sizeof(data_buf) - offset);
				chcore_assert(
				    ret >= 0,
				    "[client] receive message error!");
				remain -= ret;
				offset += ret;
			}
		} 
		else {
			memset(&msgrecv, 0, sizeof(struct msghdr));
			msgrecv.msg_name = 0;
			msgrecv.msg_namelen = 0;
			msgrecv.msg_iov = iovrecv;
			msgrecv.msg_iovlen = 1;
			while (remain > 0) {
				iovrecv[0].iov_base = data_buf + offset;
				iovrecv[0].iov_len = sizeof(data_buf) - offset;
				/* currently we do not support large buf */
				msgrecv.msg_flags = 0;
				ret = recvmsg(client_fd, &msgrecv, 0);
				chcore_assert(
				    ret >= 0,
				    "[client] receive message error!");
				remain -= ret;
				offset += ret;
			}
		}

		for (int j = 0; j < 4096; j++) {
			chcore_assert(data_buf[j] == 'a' + j % 26,
				      "[client] receive message error!");
		}

		ret = shutdown(client_fd, SHUT_WR);
		chcore_assert(ret >= 0, "[client] shutdown failed!");

		ret = close(client_fd);
		chcore_assert(ret >= 0, "[client] close failed!");
	}

	return 0;
}

void run_test_thread()
{
	pthread_t thread_id[THREAD_NUM];
	int i = 0;
	void *ret = 0;

	info("LWIP network test: %d threads\n", thread_num);

	for (i = 0; i < thread_num; i++)
		pthread_create(&thread_id[i], NULL, client_routine,
			       (void *)(unsigned long)i);
	for (i = 0; i < thread_num; i++) {
		pthread_join(thread_id[i], &(ret));
		chcore_assert((unsigned long)ret == 0,
			      "Return value check failed!");
	}
}

void *client_coroutine(void *arg)
{
	int client_fd = 0;
	int ret = 0;
	char data_buf[NET_BUFSIZE];
	int i = 0;
	struct sockaddr_in target;
	struct msghdr msgrecv;
	struct iovec iovrecv[1];
	char server_ip[] = "127.0.0.1";
	int server_port = NET_SERVERPORT;
	int offset = 0;
	int remain = 0;

	for (i = 0; i < test_num; i++) {
		offset = 0;
		remain = NET_BUFSIZE;
		memset(data_buf, 0, sizeof(data_buf));

		do {
			client_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (client_fd >= 0) /* succ */
				break;
			printf("create socket failed! errno = %d\n", errno);
			sleep(1);
		} while (1);
	
		ret = setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1},
				 sizeof(int));
		if (ret < 0) {
			info("[client] set socketopt return %d\n", ret);
			return (void *)-1;
		}

		/* Client do not need to bind */

		memset(&target, 0, sizeof(struct sockaddr_in));
		target.sin_family = AF_INET;
		target.sin_addr.s_addr = inet_addr(server_ip);
		target.sin_port = htons(server_port);
		ret = connect(client_fd, (struct sockaddr *)&target,
			      sizeof(struct sockaddr_in));
		if (ret < 0) {
			info("[client] cannot connect %d!\n", ret);
			return (void *)-1;
		}

		int mode = 0;
		ret = ioctl(client_fd, FIONBIO, &mode);
		chcore_assert(ret == 0, "[client] ioctl error!");

		if (i % 3 == 0) {
			while (remain > 0) {
				struct timeval start, end;
				gettimeofday(&start, 0);
				ret = recv(client_fd, data_buf + offset,
					   sizeof(data_buf) - offset, 0);
				gettimeofday(&end, 0);
				read_time += end.tv_sec - start.tv_sec + ((double) (end.tv_usec - start.tv_usec)) / 1000000;
				++read_count;
				chcore_assert(
				    ret >= 0,
				    "[client] receive message error!");
				remain -= ret;
				offset += ret;
			}
		} 
		else if (i % 3 == 1) {
			while (remain > 0) {
				struct timeval start, end;
				gettimeofday(&start, 0);
				ret = read(client_fd, data_buf + offset,
					   sizeof(data_buf) - offset);
				gettimeofday(&end, 0);
				read_time += end.tv_sec - start.tv_sec + ((double) (end.tv_usec - start.tv_usec)) / 1000000;
				++read_count;
				chcore_assert(
				    ret >= 0,
				    "[client] receive message error!");
				remain -= ret;
				offset += ret;
			}
		} 
		else {
			memset(&msgrecv, 0, sizeof(struct msghdr));
			msgrecv.msg_name = 0;
			msgrecv.msg_namelen = 0;
			msgrecv.msg_iov = iovrecv;
			msgrecv.msg_iovlen = 1;
			while (remain > 0) {
				iovrecv[0].iov_base = data_buf + offset;
				iovrecv[0].iov_len = sizeof(data_buf) - offset;
				/* currently we do not support large buf */
				msgrecv.msg_flags = 0;
				struct timeval start, end;
				gettimeofday(&start, 0);
				ret = recvmsg(client_fd, &msgrecv, 0);
				gettimeofday(&end, 0);
				read_time += end.tv_sec - start.tv_sec + ((double) (end.tv_usec - start.tv_usec)) / 1000000;
				++read_count;
				chcore_assert(
				    ret >= 0,
				    "[client] receive message error!");
				remain -= ret;
				offset += ret;
			}
		}

		for (int j = 0; j < 4096; j++) {
			chcore_assert(data_buf[j] == 'a' + j % 26,
				      "[client] receive message error!");
		}

		ret = shutdown(client_fd, SHUT_WR);
		chcore_assert(ret >= 0, "[client] shutdown failed!");

		ret = close(client_fd);
		chcore_assert(ret >= 0, "[client] close failed!");
	}

	return 0;
}

void run_test_coroutine()
{
	info("LWIP network test: %d coroutines\n", thread_num);
	create_scheduler();
	for(int i = 0; i < thread_num; ++i){
		create_coroutine(client_coroutine, (void *) NULL);
	}
	// clock_t start = clock();
	run_scheduler();
	// clock_t end = clock();
	// test_time = ((double) (end - start)) / CLOCKS_PER_SEC;
	close_scheduler();
}

int main(int argc, char *argv[])
{
	if (argc == 2)
		thread_num = atoi(argv[1]);
	
	if (argc == 3){
		thread_num = atoi(argv[1]);
		test_num = atoi(argv[2]);
	}

	run_test_coroutine();
	// run_test_thread();

	info("LWIP network test finished\n");
	printf("total read count: %d, read time: %f\n", read_count, read_time);
	// info("LWIP network test finished in %f seconds!\n", test_time);
	return 0;
}
