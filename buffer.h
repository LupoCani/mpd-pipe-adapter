#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#inclue "byte-utils.h"

#define VERIFY_P(a, b) if (!a) return b
#define nullptr (void*)NULL

void printn(byte * str, size_t n)
{
	write(STDOUT_FILENO, str, n);
	printf("\n");
}

typedef struct b_node{
	struct b_node *next;
	byte *data;
	size_t size;
	size_t total_size;
} node_t;

typedef node_t * buffer_t;

size_t buffer_size(buffer_t buffer)
{
	if (buffer)
		return buffer->total_size;
		
	return 0;
}

void buffer_delete(buffer_t *buffer_p)
{
	buffer_t buffer;
	buffer = *buffer_p;
	
	if (!buffer)
		return;
		
	buffer_delete(&buffer->next);
	
	free(buffer->data);
	free(buffer);
	
	*buffer_p = nullptr;
}

int buffer_write(buffer_t *buffer, byte *data, size_t len)
{
	node_t *new_node, *old_node;
	if (len == 0)
		return 0;
		
	old_node = *buffer;
	
	new_node = malloc(sizeof *new_node);
	VERIFY_P(new_node, 1);
	
	new_node->data = malloc((sizeof *data) * len);
	VERIFY_P(new_node->data, 1);
	
	new_node->size = len;
	new_node->next = old_node;
	new_node->total_size = len + buffer_size(new_node->next);
	
	memcpy(new_node->data, data, len);
	
	*buffer = new_node;
	return 0;
}

void node_cpy(node_t *node, byte *new_data, size_t len)
{
	size_t this_size	= node->size;
	size_t total_size	= node->total_size;
	size_t next_t_size	= buffer_size(node->next);
	
	assert(len == total_size);
	assert(total_size == next_t_size + this_size);
	
	memcpy(&new_data[next_t_size], node->data, this_size);
	
	if (node->next)
		node_cpy(node->next, new_data, len - this_size);
}

int buffer_cat(buffer_t buffer)
{
	byte *new_data;
	if (buffer == nullptr)
		return 0;
		
	size_t total_size = buffer->total_size;
	new_data = malloc((sizeof *new_data) * total_size);
	VERIFY_P(new_data, 1);
	
	node_cpy(buffer, new_data, total_size);
	
	buffer_delete(& buffer->next);
	free(buffer->data);
	
	buffer->data = new_data;
	buffer->size = total_size;
	
	return 0;
}

int buffer_adv(buffer_t *buffer_p, size_t steps)
{
	if (steps >= (*buffer_p)->size) {
		buffer_delete(buffer_p);
		return 1;
	}
	if ((*buffer_p)->total_size > (*buffer_p)->size)
		buffer_cat(*buffer_p);
		
	node_t *buffer;
	buffer = *buffer_p;
		
	buffer->size		-= steps;
	buffer->total_size	-= steps;
	
	byte *less_data = malloc((sizeof *less_data) * buffer->size);
	memcpy(less_data, & (buffer->data[steps]), buffer->size);
	VERIFY_P(less_data, 1);
	
	free(buffer->data);
	buffer->data = less_data;
	
	return 0;
}
