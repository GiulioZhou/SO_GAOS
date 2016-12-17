#ifndef _CLIST_H
#define _CLIST_H

#include "types.h"


#define container_of(ptr, type, member) ({                                     \
            const typeof( ((type *)0)->member ) *__mptr = (ptr);               \
            (type *)( (char *)__mptr - offsetof(type,member) );})

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/*
 * Constant used to initialize an empty list
 */
#define CLIST_INIT (struct clist) {NULL}

/*
 * Add the structure pointed to by elem as the last element of a circular list
 * clistp is the address of the tail pointer (struct clist *)
 * member is the field of *elem used to link this list
 */
#define clist_enqueue(elem, clistp, member) ({                                 \
    if ( clist_empty(*(clistp)) ){                                             \
        (clistp)->next = &(elem)->member;                                      \
        (elem)->member.next = &(elem)->member;                                 \
    } else {                                                                   \
        (elem)->member.next = (clistp)->next->next;                            \
        (clistp)->next->next = &(elem)->member;                                \
        (clistp)->next = &((elem)->member);                                    \
    }                                                                          \
})

/*
 * Add the structure pointed to by elem as the first element of a circular list
 * clistp is the address of the tail pointer (struct clist *)
 * member is the field of *elem used to link this list
 */
#define clist_push(elem, clistp, member) ({                                    \
    if ( clist_empty(*(clistp)) ){                                             \
        (clistp)->next = &((elem)->member);                                    \
        (elem)->member.next = &((elem)->member);                               \
    } else {                                                                   \
        (elem)->member.next = (clistp)->next->next;                            \
        (clistp)->next->next = &((elem)->member);                              \
    }                                                                          \
})

/*
 * clist_empty returns true in the circular list is empty, false otherwise
 * clistx is a struct clist
 */
#define clist_empty(clistx) (((clistx).next)==NULL)

/*
 * Return the pointer of the first element of the circular queue.
 * elem is also an argument to retrieve the type of the element
 * member is the field of *elem used to link this list
 */
#define clist_head(elem, clistx, member)                                       \
    (elem = clist_empty(clistx)                                                \
        ? NULL                                                                 \
        : container_of((clistx).next->next, typeof(*(elem)), member))

/*
 * Return the pointer of the last element of the circular queue.
 * elem is also an argument to retrieve the type of the element
 * member is the field of *elem used to link this list
 */
#define clist_tail(elem, clistx, member)                                       \
    (elem = clist_empty(clistx)                                                \
        ? NULL                                                                 \
        : container_of((clistx).next, typeof(*(elem)), member))

/*
 * Delete the first element of the list (this macro does not return any value)
 * clistp is the address of the tail pointer (struct clist *)
 */
#define clist_dequeue(clistp) ({                                               \
    if ( !clist_empty(*(clistp)) ) {                                           \
        if ((clistp)->next == (clistp)->next->next)                            \
            *(clistp) = CLIST_INIT;                                            \
        else                                                                   \
            (clistp)->next->next = (clistp)->next->next->next;                 \
    }                                                                          \
})

/*
 * clist_pop removes the head from the list and set elem as the old head,
 * useful for allocations
 */
#define clist_pop(clistp, elem, member) ({                                     \
    clist_head(elem, *(clistp), member);                                       \
    clist_dequeue(clistp);                                                     \
})

/*
 * Delete from a circular list the element whose pointer is elem
 * clistp is the address of the tail pointer (struct clist *)
 * member is the field of *elem used to link this list
 */
#define clist_delete(elem, clistp, member)                                     \
({                                                                             \
    char __deleted = 0;                                                        \
    if ( !clist_empty(*(clistp)) ) {                                           \
        struct clist *__nptr = (clistp)->next;                                 \
        if (__nptr == __nptr->next) {                                          \
            /* The list contains just one element */                           \
            if (container_of(__nptr, typeof(*(elem)), member) == (elem)) {     \
                *(clistp) = CLIST_INIT;                                        \
                __deleted = 1;                                                 \
            }                                                                  \
        } else do {                                                            \
            /* Delete the element */                                           \
            if (container_of(__nptr->next, typeof(*(elem)), member) == elem) { \
                if (__nptr->next == (clistp)->next) {                          \
                    (clistp)->next = __nptr;                                   \
                }                                                              \
                __nptr->next = __nptr->next->next;                             \
                __deleted = 1;                                                 \
            }                                                                  \
            /* Move on */                                                      \
            __nptr = __nptr->next;                                             \
        } while (!__deleted && __nptr != (clistp)->next);                      \
    }                                                                          \
    !__deleted;                                                                \
})

/*
 * this macro has been designed to be used as a for instruction, the instruction
 * (or block) following clist_foreach will be repeated for each element
 * of the circular list. elem will be assigned to each element
 * clistp is the address of the tail pointer (struct clist *)
 * member is the field of *elem used to link this list
 * tmp is a void * temporary variable
 */
#define clist_foreach(scan, clistp, member, tmp)                               \
    for (tmp = (clistp)->next; /* NULL if the list is empty */                 \
        tmp && /* tmp is set to NULL if list empty or after the last cycle */  \
        (scan = container_of((((struct  clist *)tmp)->next), typeof(*(scan)),  \
                member)); /* Get cycle element */                              \
        tmp = ((struct  clist *)tmp)->next,                                    \
        (tmp == (clistp)->next && (tmp = NULL)))                               \

/*
 * this macro should be used after the end of a clist_foreach cycle
 * using the same args. it returns false if the cycle terminated by a break,
 * true if it scanned all the elements
 */
#define clist_foreach_all(scan, clistp, member, tmp)                           \
    (clist_empty(*(clistp)) || !tmp)

/*
 * this macro should be used *inside* a clist_foreach loop to delete the
 * current element
 */
#define clist_foreach_delete(scan, clistp, member, tmp) ({                     \
    clist_delete(scan, clistp, member);                                        \
})

/*
 * this macro should be used *inside* a clist_foreach loop to add an element
 * before the current one
 */
#define clist_foreach_add(elem, scan, clistp, member, tmp) ({                  \
    (elem)->member.next = ((struct  clist *)(tmp))->next;                      \
    ((struct  clist *)(tmp))->next = &(elem)->member;                          \
    tmp = ((struct  clist *)(tmp))->next;                                      \
})

/*
 * This macro adds an element in the list, sorted by the field order, ascending
 */
#define clist_insert_asc(elem, clistp, member, order) ({                       \
    typeof(*(elem)) *scan;                                                     \
    void *tmp;                                                                 \
    clist_foreach(scan, clistp, member, tmp)                                   \
        if (scan->order > (elem)->order) {                                     \
            clist_foreach_add(elem, scan, clistp, member, tmp);                \
            break;                                                             \
        }                                                                      \
    /* Enqueue if not already inserted */                                      \
    if (clist_foreach_all(scan, clistp, member, tmp))                          \
        clist_enqueue(elem, clistp, member);                                   \
})


#endif
