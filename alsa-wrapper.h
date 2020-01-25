/*
Tools for abstracting away the ALSA interaction,
which is currently done through aplay.

Should probably migrate to use the actual ALSA library.
*/

#include "popen-bidir.h"

#include <string.h>
#include <stdint.h>

pipe_bd alsa_open(uint32_t rate, uint32_t channels, char * format)
{
		char exstr_rate[32], exstr_cnum[32], exstr_form[32];
		
		snprintf(exstr_rate, 32, "%u", rate);
		snprintf(exstr_cnum, 32, "%u", channels);
		
		strncpy(exstr_form, format, 32);
		exstr_form[31] = 0;
		
		char *exec_arr[] = {
			"/usr/bin/aplay",
			"-D", "converter",
			"-r", exstr_rate,
			"-c", exstr_cnum,
			"-f", exstr_form,
			NULL
		};
		
		return popen2(exec_arr, 0);
}

void alsa_setvol(uint32_t volume)
{
	if (volume > 100)
		volume = 100;
		
	char volume_str[32];
	snprintf(volume_str, 32, "%u%%", volume);
		
	char *exec_arr[] = {
		"/usr/bin/amixer",
		"sset", "Master",
		volume_str,
		NULL
	};
	
	pclose2(popen2(exec_arr, 1));
}

pipe_bd alsa_reopen(pipe_bd handle, uint32_t rate, uint32_t channels, char * format)
{
	pclose2(handle);
	
	return alsa_open(rate, channels, format);
}
