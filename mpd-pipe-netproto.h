/*
Tools to format and interpret the data sequences sent over the stream.
*/

#pragma once

#include "format-name-tools.h"
#include "mpd-pipe-codes.h"
#include "mpd-pipe-clproto.h"
#include "byte-utils.h"

#include <poll.h>
#include <stdio.h>
#include <ctype.h>

typedef enum {ctl_open, ctl_close, ctl_volume} CTL_ID;

void write_control(int sock, CTL_ID ctl, char *str)
{
	byte data[64] = {0};
	size_t data_l = 0;
	data[data_l] = CODE_CTRL;
	data_l += 1;
	
	if (ctl == ctl_open) {	
		data[data_l] = CODE_CTRL_OPEN;
		data_l += 1;
		
		uint32_t rate, channels, format_l = 32;
		byte format_str[format_l];
		
		read_mpd_format(str, &rate, &channels, format_str, format_l);
		
		uint32_to_bytes(rate, &data[data_l]);
		data_l += 4;
		
		uint32_to_bytes(channels, &data[data_l]);
		data_l += 4;
		
		mpdf_to_alsaf(format_str, format_str, format_l);
		
		memcpy(&data[data_l], format_str, format_l);
		data_l += format_l;
		
	}
	if (ctl == ctl_close) {
		data[data_l] = CODE_CTRL_CLOSE;
		data_l += 1;
	}
	if (ctl == ctl_volume) {
		data[data_l] = CODE_CTRL_VOL;
		data_l += 1;
		
		uint32_t volume;
		
		read_mpd_volume(str, &volume);
		
		uint32_to_bytes(volume, &data[data_l]);
		data_l += 4;
	}
	
	write(sock, data, data_l);
}

int read_control_format(int sock, uint32_t *rate_p, uint32_t *channels_p, char *format_p, size_t n)
{
	uint32_t rate, channels;
	size_t format_l = (32 > n) ? n : 32;
	byte format[32];
	
	byte buffer[40];
	
	if (read_exact(sock, buffer, 40, 1000) != 40)
		return -1;
		
	bytes_to_int(&rate, &buffer[0]);
	bytes_to_int(&channels, &buffer[4]);
	memcpy(format, &buffer[8], format_l);
	
	bool is_valid = true;
	
	for (size_t i = 0; i < format_l && format[i]; ++i)
		is_valid &= (isalnum(format[i]) || (format[i] == '_'));
		
	if (!is_valid)
		return -1;
		
	*rate_p = rate;
	*channels_p = channels;
	strncpy(format_p, format, format_l);
		
	return 0;
}

void write_data(int sock, byte* segment, size_t len)
{
	byte data_id;
	byte data_l[4];
	
	data_id = CODE_DATA;
	uint32_to_bytes(len, data_l);
	
	write(sock, &data_id, 1);
	write(sock, data_l, 4);
	write(sock, segment, len);
	
	//printf("Writing 1 + 4 + %u bytes of audio data.\n", len);
}

int read_data(int sock, byte* buffer, size_t max_len)
{
	byte l_bytes[4];
	uint32_t l_int = 0;
		
	int read_res = read_exact(sock, l_bytes, 4, 1000);
	
	if (read_res != 4)
		return -1;
		
	bytes_to_int(&l_int, l_bytes);
		
	if (l_int > max_len)
		return -1;
		
	//printf("Reading %u audio bytes.\n", l_int);
		
	read_res = read_exact(sock, buffer, l_int, 1000);
	
	if (read_res != l_int)
		return -1;
	
	return l_int;
}
