#ifndef MALLOC_META_H
#define MALLOC_META_H

#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include "glue.h"

__attribute__((__visibility__("hidden")))
extern const uint16_t size_classes[];

#define MMAP_THRESHOLD 131052   // 超过这个大小就使用mmap

#define UNIT 16
#define IB 4

struct group {
	struct meta *meta;
	unsigned char active_idx:5;					  // 5 bits 来表示active_index	
	char pad[UNIT - sizeof(struct meta *) - 1];   // 应该是用来对齐的,对齐0x10
	unsigned char storage[];					  // 可用的空间	
};

struct meta {
	struct meta *prev, *next;						// 链表
	struct group *mem;								// 指向group，一组空闲的内存
	volatile int avail_mask, freed_mask;			// 这组内存中可用的和free掉的内存的位图，int类型，所以最多可以有32个
	uintptr_t last_idx:5;							// last_idx 最后一个块的index
	uintptr_t freeable:1;							// 是否可以被free
	uintptr_t sizeclass:6;							// group 中的块的size
	uintptr_t maplen:8*sizeof(uintptr_t)-12;		// 如果是mmap出来的，可能会用到这个字段
};

struct meta_area {									
	uint64_t check;
	struct meta_area *next;
	int nslots;
	struct meta slots[];
};

struct malloc_context {
	uint64_t secret;								// 一个类似cookie的东西
#ifndef PAGESIZE
	size_t pagesize;
#endif
	int init_done;				
	unsigned mmap_counter;
	struct meta *free_meta_head;					// 被free的meta会链入这里，采用插头法
	struct meta *avail_meta;
	size_t avail_meta_count, avail_meta_area_count, meta_alloc_shift;
	struct meta_area *meta_area_head, *meta_area_tail;
	unsigned char *avail_meta_areas;
	struct meta *active[48];						// 活动的meta，表示这个meta还没有使用完
	size_t usage_by_class[48];
	uint8_t unmap_seq[32], bounces[32];
	uint8_t seq;
	uintptr_t brk;
};

__attribute__((__visibility__("hidden")))
extern struct malloc_context ctx;					// 这个东西导出符号是__malloc_context

#ifdef PAGESIZE
#define PGSZ PAGESIZE
#else
#define PGSZ ctx.pagesize
#endif

__attribute__((__visibility__("hidden")))
struct meta *alloc_meta(void);

__attribute__((__visibility__("hidden")))
int is_allzero(void *);

static inline void queue(struct meta **phead, struct meta *m)			
{
	assert(!m->next);
	assert(!m->prev);
	if (*phead) {
		struct meta *head = *phead;
		m->next = head;
		m->prev = head->prev;
		m->next->prev = m->prev->next = m;  // 插头法
	} else {
		m->prev = m->next = m;
		*phead = m;
	}
}

static inline void dequeue(struct meta **phead, struct meta *m)  // 有点像unlink,这个函数里面是没有什么check的
{
	if (m->next != m) {
		m->prev->next = m->next;
		m->next->prev = m->prev;
		if (*phead == m) *phead = m->next;
	} else {
		*phead = 0;
	}
	m->prev = m->next = 0;
}

static inline struct meta *dequeue_head(struct meta **phead)
{
	struct meta *m = *phead;
	if (m) dequeue(phead, m);
	return m;
}

static inline void free_meta(struct meta *m)
{
	*m = (struct meta){0};				// 清空内容
	queue(&ctx.free_meta_head, m);		// 插到free list去
}

static inline uint32_t activate_group(struct meta *m)	//可能的作用是把free的slots同步到avail_mask中去
{
	assert(!m->avail_mask);		// avail_mask = 0 
	uint32_t mask, act = (2u<<m->mem->active_idx)-1;
	do mask = m->freed_mask;
	while (a_cas(&m->freed_mask, mask, mask&~act)!=mask);		// a_cas 函数没看懂，怎么会出现arm的汇编
	return m->avail_mask = mask & act;
}

static inline int get_slot_index(const unsigned char *p)		// 用来获得一个块在group中的idx
{
	return p[-3] & 31;
}

static inline struct meta *get_meta(const unsigned char *p)  // 用来获得一个块所在的group的meta结构体
{
	assert(!((uintptr_t)p & 15));							// 检查 p 是0x10对齐的
	int offset = *(const uint16_t *)(p - 2);				// 获取对group结构体的offset
	int index = get_slot_index(p);							// 获取在这块mem中的idx
	if (p[-4]) {											// 这个位置是一个8bit的不知道啥东西
		assert(!offset);
		offset = *(uint32_t *)(p - 8);
		assert(offset > 0xffff);
	}
	const struct group *base = (const void *)(p - UNIT*offset - UNIT);		// 获取group结构体
	const struct meta *meta = base->meta;									// 获取meta
	assert(meta->mem == base);												// 后面基本全是check
	assert(index <= meta->last_idx);
	assert(!(meta->avail_mask & (1u<<index)));
	assert(!(meta->freed_mask & (1u<<index)));						
	const struct meta_area *area = (void *)((uintptr_t)meta & -4096);
	assert(area->check == ctx.secret);
	if (meta->sizeclass < 48) {
		assert(offset >= size_classes[meta->sizeclass]*index);
		assert(offset < size_classes[meta->sizeclass]*(index+1));
	} else {
		assert(meta->sizeclass == 63);
	}
	if (meta->maplen) {
		assert(offset <= meta->maplen*4096UL/UNIT - 1);
	}
	return (struct meta *)meta;
}

static inline size_t get_nominal_size(const unsigned char *p, const unsigned char *end) // 获得用户需要的名义上的size，就是去掉维护的结构的size
{
	size_t reserved = p[-3] >> 5;
	if (reserved >= 5) {
		assert(reserved == 5);
		reserved = *(const uint32_t *)(end-4);
		assert(reserved >= 5);
		assert(!end[-5]);
	}
	assert(reserved <= end-p);
	assert(!*(end-reserved));
	// also check the slot's overflow byte
	assert(!*end);
	return end-reserved-p;
}

static inline size_t get_stride(const struct meta *g)  // 返回一个块的大小
{
	if (!g->last_idx && g->maplen) {
		return g->maplen*4096UL - UNIT;
	} else {
		return UNIT*size_classes[g->sizeclass];
	}
}

static inline void set_size(unsigned char *p, unsigned char *end, size_t n)  // 这个块的大小信息
{
	int reserved = end-p-n;
	if (reserved) end[-reserved] = 0;
	if (reserved >= 5) {
		*(uint32_t *)(end-4) = reserved;
		end[-5] = 0;
		reserved = 5;	// 如果reserved>=5，那就把reserved放在end-4这个地方，然后改成5
	}
	p[-3] = (p[-3]&31) + (reserved<<5); // 0x31 = 0x11111, 低五位是flag
}

static inline void *enframe(struct meta *g, int idx, size_t n, int ctr)
{
	size_t stride = get_stride(g);		// 这个group中存放的大小
	size_t slack = (stride-IB-n)/UNIT;   // 留余
	unsigned char *p = g->mem->storage + stride*idx;		// 找到对应idx的块
	unsigned char *end = p+stride-IB;						// 块的结束
	// cycle offset within slot to increase interval to address
	// reuse, facilitate trapping double-free.
	int off = (p[-3] ? *(uint16_t *)(p-2) + 1 : ctr) & 255;
	assert(!p[-4]);
	if (off > slack) {
		size_t m = slack;
		m |= m>>1; m |= m>>2; m |= m>>4;
		off &= m;
		if (off > slack) off -= slack+1;
		assert(off <= slack);
	}
	if (off) {
		// store offset in unused header at offset zero
		// if enframing at non-zero offset.
		*(uint16_t *)(p-2) = off;
		p[-3] = 7<<5;
		p += UNIT*off;
		// for nonzero offset there is no permanent check
		// byte, so make one.
		p[-4] = 0;
	}
	*(uint16_t *)(p-2) = (size_t)(p-g->mem->storage)/UNIT;
	p[-3] = idx;
	set_size(p, end, n);
	return p;
}

static inline int size_to_class(size_t n)   // 从size获得size_classes
{
	n = (n+IB-1)>>4;			// 这个地方应该是为了取一个刚好符合的class
	if (n<10) return n;			// < 10 直接返回，就是size< 0x9c
	n++;						
	int i = (28-a_clz_32(n))*4 + 8;
	if (n>size_classes[i+1]) i+=2;
	if (n>size_classes[i]) i++;
	return i;
}

static inline int size_overflows(size_t n)		// 判断是不是溢出了
{
	if (n >= SIZE_MAX/2 - 4096) {
		errno = ENOMEM;
		return 1;
	}
	return 0;
}

static inline void step_seq(void)
{
	if (ctx.seq==255) {
		for (int i=0; i<32; i++) ctx.unmap_seq[i] = 0;
		ctx.seq = 1;
	} else {
		ctx.seq++;
	}
}

static inline void record_seq(int sc)
{
	if (sc-7U < 32) ctx.unmap_seq[sc-7] = ctx.seq;
}

static inline void account_bounce(int sc)
{
	if (sc-7U < 32) {
		int seq = ctx.unmap_seq[sc-7];
		if (seq && ctx.seq-seq < 10) {
			if (ctx.bounces[sc-7]+1 < 100)
				ctx.bounces[sc-7]++;
			else
				ctx.bounces[sc-7] = 150;
		}
	}
}

static inline void decay_bounces(int sc)
{
	if (sc-7U < 32 && ctx.bounces[sc-7])
		ctx.bounces[sc-7]--;
}

static inline int is_bouncing(int sc)
{
	return (sc-7U < 32 && ctx.bounces[sc-7] >= 100);
}

#endif
