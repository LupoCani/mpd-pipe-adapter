#pragma once

#include <string.h>

int mpdf_to_alsaf(char *mpdf, char *alsaf, size_t n)
{
	char out_str[32];
	
	if (0 == strcmp(mpdf, "8"))
		strcpy(out_str, "S8");
		
	if (0 == strcmp(mpdf, "16"))
		strcpy(out_str, "S16_LE");
		
	if (0 == strcmp(mpdf, "24"))
		strcpy(out_str, "S24_LE");
		
	if (0 == strcmp(mpdf, "32"))
		strcpy(out_str, "S32_LE");
		
	if (0 == strcmp(mpdf, "f"))
		strcpy(out_str, "FLOAT_LE");
		
	if (0 == strcmp(mpdf, "dsd"))
		return -1;
	
	if (n > 32)
		n = 32;
	strncpy(alsaf, out_str, n);
	return 1;
		
		
}
