/*
Anirudh S Chakravarthy - 2017A7PS1195P
Hariharan Srikrishnan  - 2017A7PS0134P
Honnesh Rohmetra	   - 2016B2A70770P
Praveen Ravirathinam   - 2017A7PS1174P
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "symboltableDef.h"
#include "symboltable.h"


/* Identifier Linked List - helper function */

// create an empty ID Linked list
idLinkedList createIdLinkedList() {
    idLinkedList list;
    list.head = NULL;
    list.length = 0;
    return list;
}


// delete each node from linked list
idLinkedList deleteIdLinkedList(idLinkedList list) {
    idNode* tmp = list.head;
    for(int i = 0; i < list.length - 1; i++) {
        idNode* prev = tmp;
        tmp = tmp->next;
        free(prev);
    }
    list.head = NULL;
    list.length = 0;
    return list;
}


// insert into the list
idLinkedList insertIdList(idLinkedList list, symbolTableIdEntry entry) {
    
    if (list.head == NULL) {
        idNode* newNode = (idNode*) malloc(sizeof(idNode));
        newNode->entry = entry;
        newNode->next = NULL;
        list.head = newNode;
        list.length = 1;
        return list;
    }

    idNode* tmp = list.head;
    while(tmp->next != NULL) {
        if(strcmp(tmp->entry.name, entry.name) == 0)
            return list;
        tmp = tmp->next;
    }

    idNode* newNode = (idNode*) malloc(sizeof(idNode));
    newNode->entry = entry;
    newNode->next = NULL;
    tmp->next = newNode;
    list.length++;
    return list;
}


// search for identifier in the linked list
symbolTableIdEntry* searchIdList(idLinkedList list, char* key) {
    idNode* tmp = list.head;
    while(tmp) {
        if(strcmp(tmp->entry.name, key) == 0) {
            return &(tmp->entry);
        }
        tmp = tmp->next;
    }
    return NULL;
}

// remove an identifier from the linked list
idLinkedList removeFromIdList(idLinkedList list, char* key) {
    idNode* tmp = list.head;

    if (tmp == NULL)
        return list;

    // delete first element
    if (strcmp(tmp->entry.name, key) == 0) {
        list.head = tmp->next;
        list.length--;
        free(tmp);
        return list;
    }

    idNode* prev = tmp;
    tmp = tmp->next;
    while(tmp) {

        if (strcmp(tmp->entry.name, key) == 0) {
            list.length--;
            prev->next = tmp->next;
            free(tmp);
            return list;
        }

        else {
            tmp = tmp->next;
            prev = prev->next;
        }
    }
    return list;
}



/* Functions Linked List - helper function */

// create an empty function linked list
funcLinkedList createFuncLinkedList() {
    funcLinkedList list;
    list.head = NULL;
    list.length = 0;
    return list;
}


// delete each node from linked list
funcLinkedList deleteFuncLinkedList(funcLinkedList list) {
    funcNode* tmp = list.head;
    for(int i = 0; i < list.length - 1; i++) {
        funcNode* prev = tmp;
        tmp = tmp->next;
        free(prev);
    }
    list.head = NULL;
    list.length = 0;
    return list;
}


// insert into the list
funcLinkedList insertFuncList(funcLinkedList list, symbolTableFuncEntry entry) {

    if (list.head == NULL) {
        funcNode* newNode = (funcNode*) malloc(sizeof(funcNode));
        newNode->entry = entry;
        newNode->next = NULL;
        list.head = newNode;
        list.length = 1;
        return list;
    }

    funcNode* tmp = list.head;
    while(tmp->next != NULL) {
        if(strcmp(tmp->entry.name, entry.name) == 0) 
            return list;
        tmp = tmp->next;
    }

    funcNode* newNode = (funcNode*) malloc(sizeof(funcNode));
    newNode->entry = entry;
    newNode->next = NULL;
    tmp->next = newNode;
    list.length++;
    return list;
}


// search for function in the linked list
symbolTableFuncEntry* searchFuncList(funcLinkedList list, char* key) {
    funcNode* tmp = list.head;
    while(tmp) {
        if(strcmp(tmp->entry.name, key) == 0) {
            return &(tmp->entry);
        }
        tmp = tmp->next;
    }
    return NULL;
}


// int main() {
//     funcLinkedList newList = createFuncLinkedList();
//     symbolTableFuncEntry entry;
//     entry.name = (char*) malloc(sizeof(char) * 25);
//     strcpy(entry.name, "A");
//     newList = insertFuncList(newList, entry);
//     newList = insertFuncList(newList, entry);
//     newList = insertFuncList(newList, entry);
//     printf("%d\n", newList.length);
//     symbolTableFuncEntry* temp = searchFuncList(newList, "A");
//     printf("%s\n", temp->name);
//     newList = deleteFuncLinkedList(newList);
//     printf("%d\n", newList.length);
// }