#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "elist.h"
#include "logger.h"

#define DEFAULT_INIT_SZ 10
#define RESIZE_MULTIPLIER 2

struct elist {
    size_t capacity;         /*!< Storage space allocated for list items */
    size_t size;             /*!< The actual number of items in the list */
    size_t item_sz;          /*!< Size of the items stored in the list */
    void *element_storage;   /*!< Pointer to the beginning of the array */
};

bool idx_is_valid(struct elist *list, size_t idx);

struct elist *elist_create(size_t list_sz, size_t item_sz)
{
    // we need to create enough space to hold whatever items the user wants
    // to store here

    struct elist *list = malloc(sizeof(struct elist));
    if (list == NULL) {
        return NULL;
    }
    
    // if list_sz is 0, then we'll just use default
    if (list_sz == 0) {
        list_sz = DEFAULT_INIT_SZ;
    }

    list->capacity = list_sz;
    list->item_sz = item_sz;
    list->size = 0;

    size_t storage_bytes = list->capacity * list->item_sz;
    
    LOG("Initializing new elist: capacity = [%zu], item_sz=[%zu], bytes[%zu]\n",
            list->capacity,
            list->item_sz,
            storage_bytes);

    list->element_storage = malloc(list->capacity * item_sz);

    if (list->element_storage == NULL) {
        perror("malloc");
        free(list);
        return NULL;
    }

    return list;
}

void elist_destroy(struct elist *list)
{
    free(list->element_storage);
    free(list);
}

int elist_set_capacity(struct elist *list, size_t capacity)
{
    if (capacity == 0) {
        capacity = 1;
        elist_clear(list);
    }

    list->element_storage = realloc(list->element_storage, (capacity) * list->item_sz);

    if (list->element_storage == NULL) {
        return -1;
    }

    if (capacity < list->size) {
        list->size = capacity;
    }

    list->capacity = capacity;
    
    LOG("Setting capacity: list->capacity = [%zu], capacity = [%zu], size = [%zu], item_sz = [%zu]\n",
            list->capacity,
            capacity,
            list->size,
            list->item_sz);

    return 0;
}

size_t elist_capacity(struct elist *list)
{
    return list->capacity;
}

ssize_t elist_add(struct elist *list, void *item)
{
    if (list->capacity == 0) {
        list->capacity = DEFAULT_INIT_SZ;
    }

    if (list->size >= list->capacity) {
        //TODO resize it 
        list->capacity = list->capacity * RESIZE_MULTIPLIER;
        elist_set_capacity(list, list->capacity);
    }

    if (list->element_storage == NULL) {
        return -1;
    }

    size_t idx = list->size;
    list->size++;
    
    //LOG("elist_add: capacity = [%zu], size = [%zu] item_sz = [%zu]\n",
    //        list->capacity,
    //        list->size,
    //        list->item_sz);

    void *item_ptr = list->element_storage + idx * list->item_sz;
    memcpy(item_ptr, item, list->item_sz);
    
    return idx;
}

void *elist_add_new(struct elist *list)
{
    if (list->size >= list->capacity) {
        //TODO resize it
        if (elist_set_capacity(list, list->capacity * RESIZE_MULTIPLIER) == -1) {
            return NULL;
        }
    }

    size_t idx = list->size;
    list->size++;
    void *item_ptr = list->element_storage + idx * list->item_sz; 
    
    LOG("index = %p\n", item_ptr);

    return item_ptr;
}

int elist_set(struct elist *list, size_t idx, void *item)
{
    if (idx >= list->size) {
        return -1;
    }

    void *item_ptr = list->element_storage + idx * list->item_sz;
    memcpy(item_ptr, item, list->item_sz);
    return 0;
}

void *elist_get(struct elist *list, size_t idx)
{
    if (idx >= list->size) {
        return NULL;
    }

    void *item_ptr = list->element_storage + idx * list->item_sz;
    return item_ptr;
}

size_t elist_size(struct elist *list)
{
    LOG("%zu\n", list->size);
    return list->size;
}

int elist_remove(struct elist *list, size_t idx)
{
    if (idx >= list->size) {
        return -1;
    }
    
    int i;
    for (i = idx; i < list->size; i++) {
        void *item_ptr = list->element_storage + i * list->item_sz;
        memcpy(item_ptr, item_ptr + list->item_sz, list->item_sz);
    }
    
    list->size--;

    return 0;
}

void elist_clear(struct elist *list)
{
    list->size = 0; 
}

void elist_clear_mem(struct elist *list)
{
    memset(list->element_storage, 0, list->capacity * list->item_sz);
    list->size = 0;
}

ssize_t elist_index_of(struct elist *list, void *item)
{
    int i = 0;
    while (i <= list->size) {
        if (memcmp(list->element_storage + i * list->item_sz, item, list->item_sz) == 0) {
            return i;
        }
        i++;
    }

    return -1;
}

void elist_sort(struct elist *list, int (*comparator)(const void *, const void *))
{
    qsort(list->element_storage, list->size, list->item_sz, comparator);
}

bool idx_is_valid(struct elist *list, size_t idx)
{
    if (idx < list->size) {
        return true;
    }
    return false;
}
