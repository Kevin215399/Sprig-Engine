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
    // printf("list passed\n");
    if (list == NULL)
    {
        printf("list is null\n");
        return;
    }
    if (list->lastElement == NULL)
    {
        // printf("createing first\n");
        list->firstElement = (GeneralListNode *)malloc(sizeof(GeneralListNode));
        // printf("createing malloced\n");
        list->lastElement = list->firstElement;
        list->lastElement->next = NULL;
        list->lastElement->previous = NULL;
        list->count = 1;
        // printf("done\n");
    }
    else
    {
        GeneralListNode *newElement = (GeneralListNode *)malloc(sizeof(GeneralListNode));
        list->lastElement->next = newElement;
        newElement->previous = list->lastElement;
        newElement->next = NULL;

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
    // printf("got content\n");

    if (list->count > 1)
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

void *PopListFirst(GeneralList *list)
{
    if (list->count == 0)
        return NULL;

    void *content = list->firstElement->content;
    // printf("got content\n");

    if (list->count > 1)
    {
        list->firstElement = list->firstElement->next;
        free(list->firstElement->previous);
        list->firstElement->previous = NULL;
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
    // printf("ggot\n");
    for (int i = 0; i < index; i++)
    {
        // printf("next\n");
        currentNode = currentNode->next;
    }
    // printf("returning");
    return currentNode->content;
}

void* DeleteListElement(GeneralList *list, int index)
{
    if (index >= list->count)
    {
        printf("out of bounds\n");
        return NULL;
    }

    GeneralListNode *currentNode = list->firstElement;
    // printf("ggot\n");
    for (int i = 0; i < index; i++)
    {
        // printf("next\n");
        currentNode = currentNode->next;
    }

    // printf("found\n");

    list->count--;

    if (currentNode->previous != NULL && currentNode->next != NULL)
    {
        // printf("prev and next\n");
        currentNode->previous->next = currentNode->next;
        currentNode->next->previous = currentNode->previous;
    }
    else if (currentNode->previous != NULL)
    {
        // printf("prev\n");
        currentNode->previous->next = NULL;
        list->lastElement = currentNode->previous;
    }
    else if (currentNode->next != NULL)
    {
        // printf("next\n");
        currentNode->next->previous = NULL;
        list->firstElement = currentNode->next;
    } else {
        list->firstElement = NULL;
        list->lastElement = NULL;
    }

    // printf("stitched list\n");

    void* content = currentNode->content;

    free(currentNode);

    // printf("freed node \n");
    return content;
}


void TransferList(GeneralList *to, GeneralList *from)
{
    while (from->count > 0)
    {
        printf("transfered list element\n");
        PushList(to, PopListFirst(from));
    }
}
void CpyList(GeneralList *to, GeneralList *from, size_t contentSize)
{
    for (int i = 0; i < from->count; i++)
    {

        void *valueToCopy = ListGetIndex(from, i);
        void *cpy = malloc(contentSize);
        memcpy(cpy, valueToCopy, contentSize);

        PushList(to, cpy);
    }
    to->count+=from->count;
}

#endif