#include <string.h>
#include <unistd.h>

#include "buffer.h"

int main()
{
	buffer_t buffer = nullptr;
	
	for (;;) {
		
		char str[1024];
		char a = 0;
		int n = 0;
		printf("Enter command: ");
		scanf("%c", &a);
		
		switch (a) {
			case 'W':
				scanf("%1023s", str);
				buffer_write(&buffer, str, strlen(str));
				printf("Wrote '%s' to buffer.\n", str);
				break;
				
			case 'C':
				buffer_cat(buffer);
				printf("Buffer concatenated.\n");
				break;
				
			case 'P':
				if (buffer) {
					printn(buffer->data, buffer->size);
				}
				else
					printf("Buffer is null.\n");
				break;
				
			case 'A':
				if (buffer) {
					printn(buffer->data, buffer_size(buffer));
				}
				else
					printf("Buffer is null.\n");
				break;
				
			case 'S':
				printf("Size: %i \n", buffer_size(buffer));
				break;
				
			case 'D':
				buffer_delete(buffer);
				buffer = nullptr;
		}
	}
}
