#include <stdio.h>
#include <stdlib.h>
#include "common.h"

/* typedef struct node {
    char* val;
    struct node *next;
} node_t; */

void print_list(node_t *head) {
    node_t *current = head;
    printf("%s", "[");
    while (current != NULL) {
        printf("%s%s", current->val, current->next == NULL ? "]" : ", ");
        current = current->next;
    }
}

// add to the end of the list
void push(node_t *head, char *val) {
    node_t *current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    // add the new variable
    current->next = malloc(sizeof(node_t));
    current->next->val = val;
    current->next->next = NULL;
}

// add to the beginning of the list
void push_first(node_t **head, char *val) {
    node_t * new_node;
    new_node = malloc(sizeof(node_t));

    new_node->val = val;
    new_node->next = *head;
    *head = new_node;
}

// remove the first item (and return its value)
char *pop(node_t ** head) {
    char *retval = "-1";
    node_t * next_node = NULL;

    if (*head == NULL) {
        return retval;
    }

    next_node = (*head)->next;
    retval = (*head)->val;
    free(*head);
    *head = next_node;

    return retval;
}

// remove the first item (and return its value)
char *remove_last(node_t * head) {
    char *retval = "0";
    /* if there is only one item in the list, remove it */
    if (head->next == NULL) {
        retval = head->val;
        free(head);
        return retval;
    }

    /* get to the second to last node in the list */
    node_t * current = head;
    while (current->next->next != NULL) {
        current = current->next;
    }

    /* now current points to the second to last item of the list, so let's remove current->next */
    retval = current->next->val;
    free(current->next);
    current->next = NULL;
    return retval;
}

// remove item at index n
char *remove_by_index(node_t ** head, int n) {
    int i = 0;
    char *retval = "-1";
    node_t * current = *head;
    node_t * temp_node = NULL;

    if (n == 0) {
        return pop(head);
    }

    for (i = 0; i < n-1; i++) {
        if (current->next == NULL) {
            return retval;
        }
        current = current->next;
    }

    temp_node = current->next;
    retval = temp_node->val;
    current->next = temp_node->next;
    free(temp_node);

    return retval;
}

// create new list with val as the value for the first item
node_t *new_linked_list(char *val) {
    node_t *head = NULL;
    head = malloc(sizeof(node_t));

    if (head == NULL)
        return NULL;

    head->val = val;
    head->next = NULL;

    return head;
}

/* int main() {
    node_t *llist = new_linked_list("0");
    int i;

    for (i=1; i<10; i++) {
        char val[2];
        sprintf(val, "%d", i);
        push(llist, strdup(val));
    }
    
    print_list(llist);
    puts("Pop");
    pop(&llist);
    print_list(llist);
    puts("Push 6 to beginning");
    push_first(&llist, "6");
    print_list(llist);
    puts("Remove last");
    remove_last(llist);
    print_list(llist);
    puts("Remove 4th item");
    remove_by_index(&llist, 3);
    print_list(llist);
} */