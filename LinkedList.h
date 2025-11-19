#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>

void Print(char *message){
    printf("%s\n", message);
}

typedef struct ListNode
{
    void *contents;

    struct ListNode *next;
    struct ListNode *previous;
} ListNode;

typedef struct
{
    uint8_t dataSize;
    ListNode *start;
    ListNode *end;
    uint16_t length;
} LinkedList;

LinkedList* InitializeList(int dataSize)
{
    LinkedList *output = malloc(sizeof(LinkedList));
    output->dataSize = dataSize;
    output->length = 0;
    print("List created");
    return output;
}

void AddToList(LinkedList *list, void *data, uint8_t dataSize){
    if(dataSize != list->dataSize){
        printf("Data does not match pointer type of %u bytes", list->dataSize);
        return;
    }

    if(list->length == 0){
        
        list->start = (ListNode *)malloc(sizeof(ListNode));
        print("modified start node\n");
        list->start->contents = malloc(list->dataSize);
        print("modified start size\n");
        memcpy(list->start->contents, data, list->dataSize);
        print("modified start content\n");
        list->end = (ListNode *)malloc(sizeof(ListNode));
        list->end = (list->start);
        print("modified end data\n");
    } else {
        ListNode *newNode = malloc(sizeof(ListNode));
        list->end->next = newNode;
        newNode->previous = list->end;
        list->end = newNode;
        newNode->contents = malloc(list->dataSize);
        memcpy(newNode->contents, data, list->dataSize);
    }
    list->length++;
    print("modified length\n");
}

void DeleteFromList(LinkedList *list, uint8_t index){
    if(list->length <= index){
        print("Cannot delete, out of range");
        return;
    }
    ListNode *currentNode = list->start;
    for(int i = 0; i < index; i++){
        currentNode = currentNode->next;
    }
    currentNode->previous->next = currentNode->next;
    currentNode->next->previous = currentNode->previous;
    free(currentNode);
}
