#include <stdio.h>
#include <stdlib.h>
#include "heap.h"

Node createNode(void (*entrypoint)(void *aspace),
                void *aspace,
                int priority,
                int deadline,
                int timeout)
{
    Node buf;
    buf.entrypoint = entrypoint;
    buf.aspace = aspace;
    buf.priority = priority;
    buf.deadline = deadline;
    buf.timeout = timeout;
    return buf;
}

Heap *create_heap(int capacity, int mode);
void insert(Heap *h, Node node);
void print(Heap *h);
void heapify_bottom_top(Heap *h, int index);
void heapify_top_bottom(Heap *h, int parent_node);
Node pop_min(Heap *h);

int __mode; 

#define __FIFO 0
#define __PRIO 1
#define __DDLN 2
#define __TIME 3

char more(Node left, Node right)
{
    if (left.deadline >= right.deadline)
    {
        if (__mode >= __DDLN && left.deadline > right.deadline)
        {
            return 1;
        }
        else
        {
            if (left.timeout >= right.timeout)
            {
                if (__mode >= __TIME && left.timeout > right.timeout)
                {
                    return 1;
                }
                else
                {
                    if (left.priority >= right.priority)
                    {
                        if (__mode >= __PRIO && left.priority > right.priority)
                        {
                            return 1;
                        }
                        else
                        {
                            if (*((int *)left.aspace) > *((int *)right.aspace))
                            {
                                return 1;
                            }
                            else
                            {
                                return 0;
                            }
                        }
                    }
                    else
                    {
                        return 0;
                    }
                }
            }
            else
            {
                return 0;
            }
        }
    }
    else
    {
        return 0;
    }
}

char less(Node left, Node right)
{
    return more(right, left);
}

Heap *create_heap(int capacity, int mode)
{
    __mode = mode;

    Heap *h = (Heap *)malloc(sizeof(Heap)); //one is number of heap

    //check if memory allocation is fails
    if (h == NULL)
    {
        printf("Memory Error!");
        exit(1);
    }
    h->count = 0;
    h->capacity = capacity;
    h->arr = (Node *)malloc(capacity * sizeof(Node)); //size in bytes

    //check if allocation succeed
    if (h->arr == NULL)
    {
        printf("Memory Error!");
        exit(1);
    }
    return h;
}

void insert(Heap *h, Node node)
{
    if (h->count < h->capacity)
    {
        h->arr[h->count] = node;
        heapify_bottom_top(h, h->count);
        h->count++;
    }
}

void heapify_bottom_top(Heap *h, int index)
{
    Node temp;
    int parent_node = (index - 1) / 2;

    if (more(h->arr[parent_node], h->arr[index]))
    {
        //swap and recursive call
        temp = h->arr[parent_node];
        h->arr[parent_node] = h->arr[index];
        h->arr[index] = temp;
        heapify_bottom_top(h, parent_node);
    }
}

void heapify_top_bottom(Heap *h, int parent_node)
{
    int left = parent_node * 2 + 1;
    int right = parent_node * 2 + 2;
    int min;
    Node temp;

    if (left >= h->count || left < 0)
        left = -1;
    if (right >= h->count || right < 0)
        right = -1;

    if (left != -1 && less(h->arr[left], h->arr[parent_node]))
        min = left;
    else
        min = parent_node;
    if (right != -1 && less(h->arr[right], h->arr[min]))
        min = right;

    if (min != parent_node)
    {
        temp = h->arr[min];
        h->arr[min] = h->arr[parent_node];
        h->arr[parent_node] = temp;

        // recursive  call
        heapify_top_bottom(h, min);
    }
}

Node pop_min(Heap *h)
{
    Node pop;
    if (h->count == 0)
    {
        printf("\n__Heap is Empty__\n");
        exit(1);
    }
    // replace first node by last and delete last
    pop = h->arr[0];
    h->arr[0] = h->arr[h->count - 1];
    h->count--;
    heapify_top_bottom(h, 0);
    return pop;
}
void print(Heap *h)
{
    int i;
    printf("____________Print Heap_____________\n");
    for (i = 0; i < h->count; i++)
    {
        printf("-> %d ", *((int *)h->arr[i].aspace));
    }
    printf("->__/\\__\n");
}