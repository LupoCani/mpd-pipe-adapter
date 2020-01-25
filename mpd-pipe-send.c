#include "tcp-common.h"
#include "byte-utils.h"
#include "mpd-pipe-clproto.h"
#include "mpd-pipe-netproto.h"

#include <signal.h>
#include <string.h>
#include <poll.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	setbuf(stdout, NULL);
	byte one_byte;
	printf("Starting MPD pipe adapter.\n");
	
	//Create MPD TCP socket
	SA_IN addr_MPD, addr_out;
	uint32_t port_mpd, port_out;
	
	if (argc != 4)
		return -1;
		
	sscanf(argv[1], "%u", &port_mpd);
	sscanf(argv[3], "%u", &port_out);
	char * remote_ip = argv[2];
	
	printf("MPD port: %u \nRemote: %s:%u \n",
		port_mpd, remote_ip, port_out);
	//printf("Parameters are f: %s, c: %u, r: %u", );
	
	int sock_MPD, sock_out;
	int res_set_mpd, res_set_out;
	int res_con_mpd, res_con_out;
	
	sock_MPD = socket(AF_INET, SOCK_STREAM, 0);
	sock_out = socket(AF_INET, SOCK_STREAM, 0);
	
	if (!sock_MPD || !sock_out) {
		printf("Failed to aqcuire sockets.\n");
		return 1; }
	
	res_set_mpd = setup_ip_sockaddr(&addr_MPD, DIR_OUT, 6600, 0);
	res_set_out = setup_ip_sockaddr(&addr_out, DIR_OUT, 6601, remote_ip);
	
	if (!res_set_mpd || !res_set_out) {
		printf("Failed to set addresses.\n");
		return 2;}
	
	res_con_mpd = connect(sock_MPD, (SA*) &addr_MPD, sizeof(addr_MPD));
	res_con_out = connect(sock_out, (SA*) &addr_out, sizeof(addr_out));
	
	if (res_con_mpd || res_con_out) {
		printf("Failed to connect to mpd or remote.\n");
		printf("(%d %d)\n", res_con_mpd, res_con_out);
		return 3; }
	
	//Query MPD format
	//Query MPD volume
	
	read_mpd(sock_MPD, mpd_open, nullptr, 0);
	
	char format[32];
	char volume[32];
	
	write_MPD(sock_MPD, mpd_format);
	if (read_mpd(sock_MPD, mpd_format, format, 32) < 1) {
		printf("Failed to read format.\n");
		return 4;}
	
	write_MPD(sock_MPD, mpd_volume);
	if (read_mpd(sock_MPD, mpd_volume, volume, 32) < 1) {
		printf("Failed to read volume.\n");
		return 5;}
	
	write_MPD(sock_MPD, mpd_idle);
	
	write_control(sock_out, ctl_open, format);
	write_control(sock_out, ctl_volume,  volume);
	
	struct pollfd polls[2];
	
	polls[0].fd = STDIN_FILENO;
	polls[1].fd = sock_MPD;
	
	polls[0].events = POLLNOBLOCK;
	polls[1].events = POLLNOBLOCK;
	
	bool soft_exit = false;
	
	signal(SIGPIPE, SIG_IGN);
	
	for (;;) {

		bool did_something = false;
		int pipes = poll(polls, 2, 2000);
		
		
		if (! (pipes > 0)) {
			printf("Poll result: %i", pipes);
			break;}
		
		if (polls[0].revents & (POLLNOBLOCK | POLLHUP)) {
			
			did_something = true;
			byte segment[2400];
			size_t len = read(STDIN_FILENO, segment, 2400);
			
			if (len > 0)
				write_data(sock_out, segment, len);
			else if (len == 0) {
				soft_exit = true;
				break;}
		}
		else print_bitmask(polls[0].revents);
		
		if (polls[1].revents & POLLNOBLOCK) {
			did_something = true;
			read_mpd (sock_MPD, mpd_idle, volume, 0);
			write_MPD(sock_MPD, mpd_volume);
			read_mpd (sock_MPD, mpd_volume, volume, 32);
			write_MPD(sock_MPD, mpd_idle);
			
			write_control(sock_out, ctl_volume, volume);
			printf("Detected volume change to %s.\n", volume);
		}
		
		if (!did_something) {
			printf("Unpecified read error.\n");
			break;}
	}
	write_control(sock_out, ctl_close, 0);
	read(sock_out, &one_byte, 1);
	close(sock_MPD);
	close(sock_out);
	
	if (soft_exit)
		printf("No more audio, exiting.\n");
	else
		printf("Exiting unexpectedly.\n");
	
	return 0;
}
