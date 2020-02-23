#include "head/memory.hpp"

void *memory::move(const void *dest, const void *source, size_t n) {
	if(dest == NULL || source == NULL || dest == source) return NULL;
	if(dest < source) return memory::copy(dest, source, n);

	char *sourcecast = (char*) source;
	char *destcast = (char*) dest;	

	while(n) {
		*(destcast--) = *(sourcecast--);
		n--;
	}

	return (void*) dest;
}

void *memory::copy(const void *dest, const void *source, size_t n) {
	if(dest == NULL || source == NULL || dest == source) return NULL;
	char *sourcecast = (char*) source;
	char *destcast = (char*) dest;

	for(size_t i = 0; i < n; i++) {
		destcast[i] = sourcecast[i];
	}

	return (void*) dest;
}

/* void *memcpy(void* dest, const void* src, size_t count) {
	const char *sp = (char*)src;
	char *dp = (char *)dest;
	for(size_t i = count; i >= sizeof(uint64_t); i = count){
		*((uint64_t*)dp) = *((uint64_t*)sp);
		sp = sp + sizeof(uint64_t);
		dp = dp + sizeof(uint64_t);
		count -= sizeof(uint64_t);
	}
	for(size_t i = count; i >= 4; i = count){
		*((uint32_t*)dp) = *((uint32_t*)sp);
		sp = sp + 4;
		dp = dp + 4;
		count -= 4;
	}
	for (size_t i = count; i > 0; i = count){
		dp[i] = sp[i];
		count--;
	} 
	return dest;
} */