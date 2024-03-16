#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


static inline int ele_compare(const element_t *a, const element_t *b)
{
    return strcmp(a->value, b->value);
}
static inline bool ele_compare_less(const element_t *a, const element_t *b)
{
    return ele_compare(a, b) < 0;
}
static inline bool ele_compare_greater(const element_t *a, const element_t *b)
{
    return ele_compare(a, b) > 0;
}

int q_merge_two_queues(struct list_head *dest,
                       struct list_head *src,
                       bool is_descend)
{
    // Preconditions:
    // 1. Both dest and src are non-empty.
    // 2. Both queues are sorted according to the order specified by is_descend.
    // 3. Postcondition: src is emptied and its elements are merged into dest.

    typedef bool (*comp_func_t)(const element_t *, const element_t *);
    comp_func_t comp_func = is_descend ? ele_compare_greater : ele_compare_less;

    int total_size = q_size(dest) + q_size(src);

    // Iterate through src, inserting each element into the appropriate position
    // in dest. The sorted nature of dest and src allows for optimized insertion
    // by maintaining a reference to the current insertion point in dest.
    struct list_head *cur_src = NULL, *safe = NULL, *dest_next = dest->next;
    list_for_each_safe (cur_src, safe, src) {
        element_t *cur = list_entry(cur_src, element_t, list);
        bool inserted = false;

        // Traverse dest to find the correct insertion point for the current src
        // element.
        while (dest_next != dest) {
            element_t *dest_ele = list_entry(dest_next, element_t, list);
            if (comp_func(cur, dest_ele)) {
                // Insert before the current destination element if the
                // comparison condition is met.
                list_move_tail(cur_src, dest_next);
                inserted = true;
                break;
            }
            dest_next = dest_next->next;
        }

        // If the element from src was not inserted, it's either the largest
        // (ascending order) or the smallest (descending order) compared to all
        // elements in dest. Thus, append it to the end of dest.
        if (!inserted) {
            list_move_tail(cur_src, dest);
            // Optimizing for subsequent insertions by updating the insertion
            // point.
            dest_next = dest->prev->next;
        }
    }

    return total_size;
}


/* create new element with @value */
element_t *e_new(char *value)
{
    element_t *ret = (element_t *) malloc(sizeof(element_t));
    if (ret == NULL)
        return NULL;  // no mem
    ret->value = strdup(value);
    if (ret->value == NULL) {
        free(ret);
        return NULL;  // no mem
    }
    return ret;
}

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *ret =
        (struct list_head *) malloc(sizeof(struct list_head));
    if (ret)
        INIT_LIST_HEAD(ret);
    return ret;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (head == NULL)
        return;  // nothing to do
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        q_release_element(entry);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;
    element_t *ele = e_new(s);
    if (ele == NULL)
        return false;  // error out
    list_add(&ele->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (head == NULL)
        return false;
    element_t *new_ele = e_new(s);
    if (new_ele == NULL)
        return false;
    list_add_tail(&new_ele->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    element_t *entry = list_first_entry(head, element_t, list);
    strncpy(sp, entry->value, bufsize);
    list_del(&entry->list);
    return entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (head == NULL || list_empty(head))
        return NULL;
    element_t *entry = list_last_entry(head, element_t, list);
    strncpy(sp, entry->value, bufsize);
    list_del(&entry->list);
    return entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (head == NULL)
        return -1;
    if (list_empty(head))
        return 0;
    int ret = 0;
    struct list_head *node;
    list_for_each (node, head) {
        ret++;
    }
    return ret;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (head == NULL || list_empty(head))
        return false;
    int len = q_size(head);
    int target = len / 2;
    int idx = 0;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (idx == target) {
            list_del(&entry->list);
            q_release_element(entry);
            return true;
        } else {
            idx++;
        }
    }
    return true;
}

static inline int min(const int a, const int b)
{
    return (a > b) ? b : a;
}

// static void print_list(struct list_head *head)
// {
//     element_t *entry;
//     printf("DEBUG : list contain ");
//     list_for_each_entry (entry, head, list) {
//         printf("-> %s", entry->value);
//     }
//     printf("\n");
// }

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    // assume the list is sorted
    if (!head)
        return false;
    if (list_empty(head))  // empty is already dedup
        return true;

    struct list_head *node = NULL, *safe = NULL;
    bool delete_next = false;
    list_for_each_safe (node, safe, head) {
        element_t *cur, *next;
        cur = list_entry(node, element_t, list);
        next = list_entry(node->next, element_t, list);
        bool match = (node->next != head);
        if (match)
            match &= (strcmp(cur->value, next->value) == 0);
        if (match || delete_next) {
            list_del(node);
            q_release_element(cur);
        }
        delete_next = match;
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    struct list_head *prev, *now, *next;
    list_for_each (now, head) {
        if (now->next == head) {
            return;
        }
        next = now->next;
        prev = now->prev;
        // prev <-> now
        list_del(next);
        // prev <-> next <-> now
        now->prev = next;
        next->prev = prev;
        prev->next = next;
        next->next = now;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (head == NULL || list_empty(head))
        return;
    struct list_head *cur, *next;
    cur = head;
    for (;;) {
        next = cur->next;
        cur->next = cur->prev;
        cur->prev = next;
        cur = next;
        if (cur == head)
            break;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;  // nothing to do

    if (k >= q_size(head)) {
        q_reverse(head);  // normal reverse
        return;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *fast = head->next, *slow = head;
    for (; fast != head && fast->next != head; fast = fast->next->next)
        slow = slow->next;

    struct list_head right;
    list_cut_position(&right, head, slow);

    q_sort(head, descend);
    q_sort(&right, descend);

    q_merge_two_queues(head, &right, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}



/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (head == NULL || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return q_size(list_first_entry(head, queue_contex_t, chain)->q);

    int cnt = 0;
    queue_contex_t *first = list_first_entry(head, queue_contex_t, chain);
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        queue_contex_t *cur_ctx = list_entry(node, queue_contex_t, chain);
        if (cur_ctx == first)
            continue;
        cnt = q_merge_two_queues(first->q, cur_ctx->q, descend);
    }
    return cnt;
}
