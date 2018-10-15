#include "stdint.h"
#include "stddef.h"


struct header_t
{
	size_t size;
	unsigned free;
	struct header_t * next;
};

struct header_t *first = NULL, *last = NULL;


extern volatile uint8_t __heap_start;	/*Heap start address*/
extern volatile uint8_t __heap_end;		/*Heap end address*/

/*BRK Pointer*/
void * brk = 0;

/*Function that allows to decrease and increase the brk pointer*/
void * sbrk(size_t size)
{
	if(size == 0)
		return brk;
	brk = (void *)(((unsigned int) brk) + size);
	return brk;
}

struct header_t * get_first_free_block(size_t size)
{
	struct header_t * aux = NULL;

	aux = first;

	while (aux != NULL)
	{
		if(aux->free && aux->size >= size)
			return aux;
		aux = aux->next;
	}
	return NULL;
}

/*Is not necessary to protect the alloc and free actions with a mutex because there will be only one process*/

void * malloc(size_t size)
{
	size_t total_size;
	void * block;
	struct header_t * header;

	if(!size)
		return NULL;

	header = get_first_free_block(size);

	/*There is a free block with the appropiate size*/
	if(header != NULL)
	{
		header->free = 0;
		return (void *) (header + 1);
	}

	total_size = sizeof(struct header_t) + size;

	/*Alloc memory*/
	block = (void *)sbrk(total_size);

	/*Allocation error*/
	if(block == (void *)-1)
	{
		return NULL;
	}
	
	header = block;
	/*Setting header values*/
	header->free = 0;
	header->size = size;
	header->next = NULL;

	/*Adding new block to the block chain*/
	last->next = header;
	last = header;

	return (void *) (header + 1);
}


void free(void * block)
{
	struct header_t * header = NULL, * aux = NULL;

	void * brk_pointer;

	if(block == NULL)
		return;

	/*Get the header of the block*/
	header = (struct header_t *)block - 1;

	/*Get brk pointer*/
	brk_pointer = (void *)sbrk(0);

	/*Freeing latest block allocated*/
	if(brk_pointer == (void *) (((unsigned int) (block)) + header->size) )
	{
		/*There is only one block allocated*/
		if(first == last)
			first = last = NULL;
		/*There are more than one block allocated*/
		else
		{
			aux = first;
			while(aux->next != header)
				aux = aux->next;

			/*Remove the block from the chain*/
			aux->next = NULL;
			last = aux;
		}
		sbrk(-(sizeof(struct header_t) + header->size));
		return;
	}
	header->free = 1;

	return;
}