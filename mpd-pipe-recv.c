#include "mpd-pipe-codes.h"
#include "tcp-common.h"
#include "byte-utils.h"
#include "popen-bidir.h"
#include "mpd-pipe-netproto.h"
#include "alsa-wrapper.h"

#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>

int main()
{
	//Begin setup
	struct sockaddr_in servaddr, cli;
	int sockfd, connfd, enable = 1; 
	unsigned int len;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	setup_ip_sockaddr(&servaddr, DIR_IN, 6601, "");
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof enable);
	
	printf("Socket configured.\n");
	
	int bind_res = bind(sockfd, (SA*)&servaddr, sizeof(servaddr));
	int list_res = listen(sockfd, 1);
	
	struct pollfd polls[1];
	byte next_bytes[32] = {0};
	int read_res = 0;
	bool soft_close = false;
	
	printf("Port listening setup %d %d.\n", bind_res, list_res);
	signal(SIGPIPE, SIG_IGN);
	
	for (;;) {
		printf("Waiting for connection.\n");
		soft_close = false;
		connfd = accept(sockfd, (SA*) &cli, &len);
		
		printf("Accepting connection.\n");
		
		read_res = read_exact(connfd, next_bytes, 2, 1000);
		if (read_res != 2 ||
			next_bytes[0] != CODE_CTRL ||
			next_bytes[1] != CODE_CTRL_OPEN)
			goto cleanup_loop;
		
		uint32_t rate, channels;
		char format[32];
		
		if (read_control_format(connfd, &rate, &channels, format, 32) < 0)
			goto cleanup_loop;
			
		printf("Parameters valid, opening aplay.\n (-f %s -r %u -c %u) \n",
			format, rate, channels);
			
		pipe_bd alsa_out2 = alsa_open(rate, channels, format);
		
		polls[0].fd = connfd;
		polls[0].events = POLLNOBLOCK;
		
		while (poll(polls, 1, 2000))
		{
			if (! (polls[0].revents & POLLNOBLOCK)) {
				printf("Timeout/broken pipe.\n");
				break;}
			
			read_res = read_exact(connfd, next_bytes, 1, 5000);
			
			if (read_res != 1) {
				printf("Could not get sequence ID byte.");
				break;}
			
			if (next_bytes[0] == CODE_CTRL)
			{
				read_res = read_exact(connfd, next_bytes, 1, 1000);
				if (read_res != 1){
					printf("Could not get control ID byte.");
					break;
				}
				if (next_bytes[0] == CODE_CTRL_CLOSE) {
					printf("End of data, exiting.\n");
					soft_close = true;
					break;
				}
				if (next_bytes[0] == CODE_CTRL_VOL) {
					byte volume_b[4] = {0};
					uint32_t volume_i = 0;
					read_exact(connfd, volume_b, 4, 1000);
					bytes_to_int(&volume_i, volume_b);
					printf("Volume changed to %u.\n", volume_i);
				}
			}
			else if (next_bytes[0] == CODE_DATA)
			{
				byte audio_data[24000];
				int bytes_rec = read_data(connfd, audio_data, 24000);
				
				if (bytes_rec > 0) {
					while (write(alsa_out2.in, audio_data, bytes_rec) < 0) {
						alsa_out2 = alsa_reopen(alsa_out2, rate, channels, format);
					}
				} else {
					printf("Incorrect audio data.\n");
					break;
				}
			}
			else {
				printf("Invalid sequence id (%u).\n", next_bytes[0]);
				break;
			}
		}
		
		cleanup_loop:
		
		printf("Beginning cleanup.\n");
		//pclose(alsa_out);
		pclose2(alsa_out2);
		close(connfd);
		
	}
	return 0;
}
