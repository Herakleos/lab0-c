#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *node = malloc(sizeof(struct list_head));
    if (!node)
        return NULL;
    INIT_LIST_HEAD(node);
    return node;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    struct list_head *li, *tmp;
    list_for_each_safe (li, tmp, l) {
        element_t *delete = list_entry(li, element_t, list);
        list_del(li);
        q_release_element(delete);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_head = malloc(sizeof(element_t));
    if (!new_head)
        return false;
    INIT_LIST_HEAD(&new_head->list);
    size_t size = sizeof(char) * (strlen(s) + 1);  // for NULL terminator
    new_head->value = malloc(size);
    if (!new_head->value) {
        free(new_head);
        return false;
    }
    strncpy(new_head->value, s, size);
    list_add(&new_head->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;
    size_t size = sizeof(char) * (strlen(s) + 1);  // for NULL terminator
    node->value = malloc(size);
    if (!node->value) {
        free(node);
        return false;
    }
    strncpy(node->value, s, size);
    list_add_tail(&node->list, head);
    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *remove = list_first_entry(head, element_t, list);
    list_del_init(head->next);

    if (sp) {
        memset(sp, '\0', bufsize);                  // plus a null terminator
        strncpy(sp, remove->value, (bufsize - 1));  // maximum of bufsize-1
    }
    return remove;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *remove = list_last_entry(head, element_t, list);
    list_del(head->prev);

    if (sp) {
        memset(sp, '\0', bufsize);                  // plus a null terminator
        strncpy(sp, remove->value, (bufsize - 1));  // maximum of bufsize-1
    }
    return remove;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

struct list_head *find_mid(struct list_head *head)
{
    struct list_head *forward = head->next, *backward = head->prev;
    while (forward != backward) {
        forward = forward->next;
        if (forward == backward)
            break;
        backward = backward->prev;
    }
    return forward;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return NULL if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return NULL;

    struct list_head *forward = find_mid(head);
    element_t *delete = list_entry(forward, element_t, list);
    list_del(forward);
    q_release_element(delete);
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    int delete = 0;
    struct list_head *li, *tmp;
    list_for_each_safe (li, tmp, head) {
        element_t *cur = list_entry(li, element_t, list);
        element_t *nxt = list_entry(li->next, element_t, list);
        if ((li->next != head) && !strcmp(cur->value, nxt->value)) {
            delete = 1;
            list_del(li);
            q_release_element(cur);
        } else if (delete) {
            list_del(li);
            q_release_element(cur);
            delete = 0;
        }
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    if (!head)
        return;

    struct list_head *cur = head->next;
    while ((cur != head) && (cur->next != head)) {
        struct list_head *tmp = cur->next;
        list_del(cur);
        list_add(cur, tmp);
        cur = cur->next;
    }
}

static inline void swap(char **x, char **y)
{
    char *tmp = *x;
    *x = *y;
    *y = tmp;
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *forward = head->next, *backward = head->prev;
    while (forward != backward) {
        element_t *fwd = list_entry(forward, element_t, list);
        element_t *bwd = list_entry(backward, element_t, list);
        swap(&fwd->value, &bwd->value);
        forward = forward->next;
        if (forward == backward)
            break;
        backward = backward->prev;
    }
}

struct list_head *merge(struct list_head *a, struct list_head *b)
{
    struct list_head *head = NULL, **tail = &head;

    for (;;) {
        if (strcmp(list_entry(a, element_t, list)->value,
                   list_entry(b, element_t, list)->value) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

struct list_head *merge_sort(struct list_head *head)
{
    if (!head || !head->next)
        return head;

    // find mid
    struct list_head *fast = head->next, *slow = head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    fast = slow->next;
    slow->next = NULL;

    struct list_head *left = merge_sort(head);
    struct list_head *right = merge_sort(fast);

    return merge(left, right);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    // to singly linked list
    head->prev->next = NULL;

    head->next = merge_sort(head->next);

    // to doubly linked list
    struct list_head *tail = head;
    while (tail->next) {
        tail->next->prev = tail;
        tail = tail->next;
    }
    tail->next = head;
    head->prev = tail;
}
