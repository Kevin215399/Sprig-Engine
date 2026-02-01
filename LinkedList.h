#ifndef LINKED_LIST
#define LINKED_LIST

#include <stdio.h>
#include <stdlib.h>

typedef struct GeneralListNode
{
    struct GeneralListNode *next;
    struct GeneralListNode *previous;
    void *content;
} GeneralListNode;

typedef struct GeneralList
{
    GeneralListNode *firstElement;
    GeneralListNode *lastElement;
    int count;
} GeneralList;

void InitializeList(GeneralList *list)
{
    list->count = 0;
    list->firstElement = NULL;
    list->lastElement = NULL;
}

void PushList(GeneralList *list, void *content)
{
    //printf("list passed\n");
    if (list == NULL)
    {
        printf("list is null\n");
        return;
    }
    if (list->lastElement == NULL)
    {
       // printf("createing first\n");
        list->firstElement = (GeneralListNode *)malloc(sizeof(GeneralListNode));
        //printf("createing malloced\n");
        list->lastElement = list->firstElement;
        list->count = 1;
        //printf("done\n");
    }
    else
    {
        GeneralListNode *newElement = (GeneralListNode *)malloc(sizeof(GeneralListNode));
        list->lastElement->next = newElement;
        newElement->previous = list->lastElement;

        list->lastElement = newElement;
        list->count++;
    }

    list->lastElement->content = content;
}

void *PopList(GeneralList *list)
{
    if (list->count == 0)
        return NULL;

    void *content = list->lastElement->content;
    //printf("got content\n");

    if (list->count >1)
    {
        list->lastElement = list->lastElement->previous;
        free(list->lastElement->next);
        list->lastElement->next = NULL;
    }
    else
    {
        free(list->firstElement);
        list->firstElement = NULL;
        list->lastElement = NULL;
    }

    list->count--;

    return content;
}

void *ListGetIndex(GeneralList *list, int index)
{
    if (index >= list->count)
    {
        printf("out of bounds\n");
        return NULL;
    }

    GeneralListNode *currentNode = list->firstElement;
    //printf("ggot\n");
    for (int i = 0; i < index; i++)
    {
        //printf("next\n");
        currentNode = currentNode->next;
    }
    //printf("returning");
    return currentNode->content;
}

#endif