#pragma once

struct Node
{
    void (*entrypoint)(void *aspace);
    void *aspace;
    int priority;
    int deadline;
    int timeout;
};

typedef struct Node Node;

struct Heap
{
    Node *arr;
    int count;
    int capacity;
};

typedef struct Heap Heap;

extern Node createNode(void (*entrypoint)(void *aspace),
                void *aspace,
                int priority,
                int deadline,
                int timeout);

extern Heap * create_heap(int capacity, int mode);

extern void insert(Heap *h, Node node);

extern void print(Heap *h);

extern Node pop_min(Heap *h);

