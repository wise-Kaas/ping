#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

#include "ping.h"
#include "lib.h"

#include <stdio.h>

void	run_requests(t_info *info);
void	sig_handler(int signo);
void	prepare_info(char *input, t_info *info);
void	prepare_msg_object(t_msg_in *msg);

/*
** global variable control the flow of program
*/
volatile sig_atomic_t g_v = SEND_PACKET;

// TODO switch info.address_info to addrinfo to support both IPv4 and IPv6
int		main(int argv, char *args[])
{
	t_info info;

	ft_memset(&info, 0, sizeof(info));
	options(argv, args, &info.options);
	prepare_info(args[argv - 1], &info);
	//TODO decide between malloc and in-place
	if (signal(SIGALRM, sig_handler) == SIG_ERR)
		exit_with_error(SIGNAL_ERROR);
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		exit_with_error(SIGNAL_ERROR);
	print_execution_intro(args[argv - 1], info.dst_char, info.options.icmp_data_size);
	//TODO choose between normal return and use of exit
	run_requests(&info);
	return (0);
}

void 	sig_handler(int signo)
{
	if (signo == SIGALRM)
		g_v = SEND_PACKET;
	else if (signo == SIGINT)
		g_v = EXIT;
}

/*
** getting sockets for sending and receiving packets
** checking options passed by user
** getting destination address
** preparing non-changing parts of icmp packet
** preparing struct for gathering rt stats
*/

void	prepare_info(char *input, t_info *info)
{
	info->pid = getpid();
	info->sfd_out = get_socket_out(&info->options);
	info->sfd_in = get_socket_in();
	if (!(info->options.options & T_FLAG))
		info->options.ttl = DEFAULT_TTL;
	printf("flags - %d\n", info->options.options);
	if (!(info->options.options & S_FLAG))
		info->options.icmp_data_size = DEFAULT_ICMP_DATA_SIZE;
	info->icmp_size = (int) sizeof(t_icmp_hdr) + info->options.icmp_data_size;
	//TODO check if input already an address
	info->address_info = get_address(input);
	inet_ntop(AF_INET, &(info->address_info.sin_addr), info->dst_char, sizeof(info->dst_char));
	//TODO decide between malloc and in-place
	info->icmp_packet = get_icmp_packet(info);
	info->rt_stats = (t_rt_stats *)malloc(sizeof(*info->rt_stats));
	ft_memset(info->rt_stats, 0, sizeof(*info->rt_stats));
	if (gettimeofday(&info->rt_stats->tv_start, NULL) != 0)
		exit_with_error(GETTIMEOFDAY_ERROR);
	info->rt_stats->min = DEFAULT_TIMEOUT * 1000000; // max waiting time
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
/*
** recvmsg is set to non-blocking mode
** and will exit with errno set to EINTR
** in case of being interrupted by signal
** and to EAGAIN when did not recieve any
** message by the end of timer
*/
void	run_requests(t_info *info)
{
	t_msg_in	msg;

	prepare_msg_object(&msg);
	if (info->options.options & L_FLAG) {
		while (info->options.preload-- > 0) {
			update_icmp_packet(info->rt_stats->pkg_sent + 1, info->icmp_size, info->icmp_packet);
			info->rt_stats->pkg_sent += send_packet(info->sfd_out, info->icmp_size, info->icmp_packet, &info->address_info);
		}
	}
	// TODO think about changing the size of icmp packet
	while (1)
	{
		if (g_v == SEND_PACKET)
		{
			update_icmp_packet(info->rt_stats->pkg_sent + 1, info->icmp_size, info->icmp_packet);
			info->rt_stats->pkg_sent += send_packet(info->sfd_out, info->icmp_size, info->icmp_packet, &info->address_info);
			g_v = DO_NOTHING;
			alarm(1);
		}
		if (recvmsg(info->sfd_in, &msg.msghdr, 0) < 0)
		{
			if (errno != EINTR && errno != EAGAIN)
				exit_with_error(RECVMSG_ERROR);
		}
		else
			verify_received_packet(info->pid, info->icmp_size, info->rt_stats, &msg);
		if (g_v == EXIT)
			exit_program(info);
		if (info->options.options & C_FLAG && info->rt_stats->pkg_sent == info->options.count)
			exit_program(info);
	}
}
#pragma clang diagnostic pop

void prepare_msg_object(t_msg_in *msg)
{
	ft_memset(msg, 0, sizeof(*msg));
	msg->io.iov_base = msg->buf;
	msg->io.iov_len = 256; //TODO macroS
	msg->msghdr.msg_name = &msg->rec_addr;
	msg->msghdr.msg_namelen = sizeof(msg->rec_addr);
	msg->msghdr.msg_control = msg->buf + 256;
	msg->msghdr.msg_controllen = 256;
	msg->msghdr.msg_iov = &msg->io;
	msg->msghdr.msg_iovlen = 1;
}
