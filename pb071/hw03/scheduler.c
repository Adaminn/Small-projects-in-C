#include "scheduler.h"
#include <assert.h>

#ifdef CONFIG_ENABLE_DEBUG
#include <stdio.h>
#define DEBUG(STATEMENT) \
    do {                 \
        STATEMENT;       \
    } while (0)
#else
#define DEBUG(STATEMENT)
#endif /* CONFIG_ENABLE_DEBUG */

unsigned cpu_count(unsigned mask)
{
    unsigned count = 0;
    unsigned i = 1;
    unsigned int max = (unsigned int) ~0 >> 1;

    for (i = 1; i < max; i *= 2) {
        if (i & mask) {
            count++;
        }
    }
    return count;
}

bool is_inconsistent(process_type p1, process_type p2)
{
    return p1.context == p2.context && p1.callback == p2.callback;
}
bool is_prioritized(process_type p1, process_type p2)
{
    if (p1.remaining_time * p1.niceness < p2.remaining_time * p2.niceness) {
        return false;
    }
    if (p1.remaining_time * p1.niceness == p2.remaining_time * p2.niceness) {
        unsigned p1_cpus = cpu_count(p1.cpu_mask);
        unsigned p2_cpus = cpu_count(p2.cpu_mask);

        if (p1_cpus < p2_cpus) {
            return false;
        }
    }
    return true;
}

bool insert(struct priority_queue *queue, process_type to_insert, priority_queue_item *in_front)
{
    priority_queue_item *new = malloc(sizeof(priority_queue_item));
    priority_queue_item *prev_dest = NULL;
    if (new == NULL) {
        return false;
    }
    if (in_front != NULL) { // is not last
        prev_dest = in_front->prev;
        in_front->prev = new;
    } else {
        prev_dest = queue->bottom;
        queue->bottom = new;
    }

    if (prev_dest != NULL) { // is not first
        prev_dest->next = new;
    } else {
        queue->top = new;
    }

    new->next = in_front;
    new->prev = prev_dest;

    new->process = to_insert;
    queue->size++;
    return true;
}

priority_queue create_queue(void)
{
    priority_queue queue;

    queue.top = NULL;
    queue.bottom = NULL;
    queue.size = 0;
    return queue;
}

void clear_queue(priority_queue *queue)
{
    assert(queue != NULL);

    priority_queue_item *current = queue->top;
    priority_queue_item *next = NULL;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    queue->size = 0;
    queue->bottom = NULL;
    queue->top = NULL;
}

bool copy_queue(priority_queue *dest, const priority_queue *source)
{
    assert(dest != NULL && source != NULL);

    priority_queue dest_temp = create_queue();
    if (source->size == 0) {
        *dest = create_queue();
        return true;
    }

    priority_queue_item *current_source = source->top;
    while (current_source != NULL) {
        if (!insert(&dest_temp, current_source->process, NULL)) {
            clear_queue(&dest_temp);
            return false;
        }

        current_source = current_source->next;
    }
    *dest = create_queue();
    dest->top = dest_temp.top;
    dest->bottom = dest_temp.bottom;
    dest->size = dest_temp.size;
    return true;
}

enum push_result push_to_queue(priority_queue *queue, process_type process)
{
    assert(queue != NULL);
    assert(process.niceness < 50 && process.niceness >= 10);
    priority_queue_item *top = queue->top;
    priority_queue_item *current = top;

    while (current != NULL) { // check for duplicates
        if (is_inconsistent(current->process, process)) {
            if (current->process.remaining_time == process.remaining_time &&
                    current->process.niceness == process.niceness &&
                    current->process.cpu_mask == process.cpu_mask) {
                return push_duplicate;
            }
            return push_inconsistent;
        }

        current = current->next;
    }
    current = top;

    while (current != NULL) {
        if (is_prioritized(current->process, process)) {
            break;
        }
        current = current->next;
    }

    if (insert(queue, process, current)) {
        return push_success;
    }
    return push_error;
}
priority_queue_item *get_top_node(const priority_queue *queue, uint16_t cpu_mask)
{
    assert(queue != NULL);
    priority_queue_item *current = queue->top;

    while (current != NULL) {
        if (current->process.cpu_mask & cpu_mask) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

process_type *get_top(const priority_queue *queue, uint16_t cpu_mask)
{
    assert(queue != NULL);
    priority_queue_item *current = get_top_node(queue, cpu_mask);
    if (current != NULL) {
        return &current->process;
    }
    return NULL;
}

void del_node(priority_queue *queue, priority_queue_item *to_delete)
{
    assert(queue != NULL);
    if (to_delete->prev != NULL) { // if not first
        to_delete->prev->next = to_delete->next;
    } else {
        queue->top = to_delete->next;
    }

    if (to_delete->next != NULL) { // if not last
        to_delete->next->prev = to_delete->prev;
    } else {
        queue->bottom = to_delete->prev;
    }
    free(to_delete);
    queue->size--;
}

bool pop_top(priority_queue *queue, uint16_t cpu_mask, process_type *out)
{
    assert(queue != NULL);
    priority_queue_item *current = queue->top;

    while (current != NULL) {
        if (current->process.cpu_mask & cpu_mask) {
            if (out != NULL) {
                *out = current->process;
            }

            del_node(queue, current);
            return true;
        }
        current = current->next;
    }
    return false;
}
void re_insert(priority_queue *queue, priority_queue_item *to_insert)
{
    process_type temp = to_insert->process;
    del_node(queue, to_insert);
    push_to_queue(queue, temp);
}

unsigned int run_top(priority_queue *queue, uint16_t cpu_mask, unsigned int run_time)
{
    assert(queue != NULL);
    if (queue->size == 0) {
        return 0;
    }
    priority_queue_item *node = get_top_node(queue, cpu_mask);
    if (node == NULL) {
        return 0;
    }
    if (node->process.callback == NULL) {
        return 0;
    }
    unsigned int cb_ret = (*(node->process.callback))(run_time, node->process.context);
    if (cb_ret == 0) {
        del_node(queue, node);
        return 0;
    }
    unsigned result;
    if (run_time > node->process.remaining_time) {
        result = cb_ret;
    } else {
        result = node->process.remaining_time - run_time + cb_ret;
    }
    node->process.remaining_time = result;
    re_insert(queue, node);
    return result;
}

bool renice(priority_queue *queue, cb_type callback, void *context, unsigned int niceness)
{
    assert(queue != NULL);

    priority_queue_item *top = queue->top;
    priority_queue_item *current = top;
    process_type target_process = { .callback = callback, .context = context, .niceness = niceness };
    while (current != NULL) { // find the item
        if (is_inconsistent(current->process, target_process)) {
            current->process.niceness = target_process.niceness;
            re_insert(queue, current);
            return true;
        }

        current = current->next;
    }
    return false;
}
