#include <mem/alloc.hpp>

void *operator new(size_t size) {
    return kmalloc(size);
}

void *operator new[](size_t size) {
    return kmalloc(size);
}

void operator delete(void *p) {
    kfree(p);
}

void operator delete[](void *p) {
    kfree(p);
}

void operator delete(void *p, size_t size) {
    ::operator delete(p);
}

void operator delete[](void *p, size_t size) {
    kfree(p);
}
