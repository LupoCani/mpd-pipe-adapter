/*
Tools for talking to the MPD API.
*/

#pragma once

#include "tcp-common.h"

#include <string.h>

typedef enum {mpd_open, mpd_idle, mpd_format, mpd_volume} MPD_DATA;

int read_mpd(int sock, MPD_DATA context, char* out, size_t n)
{
	char *w_begin, *w_end;
	
	const size_t b_size = 1024;
	char buffer[b_size];
	
	int msg_len = -1;
	short poll_res;
	bool parse_success = false;
	
	if (simple_poll(sock, POLLNOBLOCK, &poll_res, 10000))
		if (poll_res == POLLNOBLOCK)
			msg_len = read(sock, buffer, b_size);
		else
			msg_len = -2;
	
	if (msg_len > 0)
		switch (context) {
			case mpd_open:
				w_begin = buffer;
				
				w_end = strchr(buffer, ' ');
				if (!w_end) break;
				
				parse_success = true;
				break;
				
			case mpd_idle:
				w_begin = buffer;
				w_end = buffer;
				
				parse_success = true;
				break;
				
			case mpd_format:
				/*C didn't like declaring a var in the first statement*/;
				char field_name[32] = "audio:";
			case mpd_volume:
				if (context == mpd_volume)
					strcpy(field_name, "volume:");
			
				w_begin = strstr(buffer, field_name);
				if (!w_begin) break;
				
				w_begin = strchr(w_begin, ':');
				if (!w_begin) break;
				++w_begin;
				
				w_end = strchr(w_begin, '\n');
				if (!w_end) break;
				
				parse_success = true;
				break;
		}
		
	if (!parse_success)
		return -1;
	
	if (n == 0 || out == nullptr)
		return 0;
		
	size_t w_len = w_end - w_begin;
	
	--n;
	if (w_len > n)
		w_len = n;
		
	if (w_len)
		strncpy(out, w_begin, w_len);
	out[w_len] = 0;
	
	return w_len;
}

void write_MPD(int sock, MPD_DATA action)
{
	char str[32];
	switch (action) {
		case mpd_idle:
			strcpy(str, "idle mixer");
			break;
			
		case mpd_format:
		case mpd_volume:
			strcpy(str, "status");
			break;
			
		default:
			return;
	}
	
	write(sock, str, strlen(str));
	str[0] = '\n';
	write(sock, str, 1);
}

void read_mpd_format(char *comb_str, uint32_t *rate, uint32_t *cnum, byte *form, size_t form_l)
{
	char *rate_str, *cnum_str, *form_str;
	
	uint32_t cnum_inter = 0;
	char form_str_inter[32];
	
	rate_str = comb_str;
	
	form_str = strpbrk(rate_str, ":");
	if (form_str)
		*(form_str++) = 0;
		
	cnum_str = strpbrk(form_str, ":");
	if (cnum_str)
		*(cnum_str++) = 0;
	
	sscanf(rate_str, "%u", rate);
	sscanf(cnum_str, "%u", &cnum_inter);
	sscanf(form_str, "%31s", form_str_inter);
	
	*cnum = clamp_to_byte(cnum_inter);
	if (form_l > strlen(form_str_inter) + 1)
		form_l = strlen(form_str_inter) + 1;
	memcpy(form, form_str_inter, form_l);
}

void read_mpd_volume(char *data_str, uint32_t *volume)
{	
	int volume_signed = 0;
	
	sscanf(data_str, "%d", &volume_signed);
	
	if (volume_signed < 0)
		*volume = 0;
	else
		*volume = volume_signed;
		
}
