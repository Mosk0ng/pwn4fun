```c
struct meta_area {
	uint64_t check;
	struct meta_area *next;
	int nslots;
	struct meta slots[];
};

struct meta {
	struct meta *prev, *next;   // 双链表
	struct group *mem;			// 指向group，group中还有一个指针指回来
	volatile int avail_mask, freed_mask;  // 位图,free掉不一定可用
	uintptr_t last_idx:5;
	uintptr_t freeable:1;
	uintptr_t sizeclass:6;
	uintptr_t maplen:8*sizeof(uintptr_t)-12;
};

$11 = {
  prev = 0x55555555a1a8,
  next = 0x55555555a1a8,
  mem = 0x7ffff7ffed10,
  avail_mask = 1016,			// 位表
  freed_mask = 0,
  last_idx = 9,				   // 最后一个块的index
  freeable = 1,				   // 能否free
  sizeclass = 2,               // size class = 2 代表一类chunk的大小
  maplen = 0					// 不知道是干啥的
}


struct group {
	struct meta *meta;
	unsigned char active_idx:5;
	char pad[UNIT - sizeof(struct meta *) - 1]; // 7
	unsigned char storage[];
};

$12 = {
  meta = 0x55555555a1a8,
  active_idx = 9 '\t',
  pad = "\000\000\000\000\240\000",
  storage = 0x7ffff7ffed20 'a' <repeats 16 times>
}

0x7ffff7ffed10:	0x000055555555a1a8	0x0000a00000000009
0x7ffff7ffed20:	0x6161616161616161	0x6161616161616161
0x7ffff7ffed30:	0x0000000000000000	0x0000000000000000
0x7ffff7ffed40:	0x0000000000000000	0x0003a1000000000c
0x7ffff7ffed50:	0x0000000000000000	0x0000000000000000
0x7ffff7ffed60:	0x0000000000000000	0x0000000000000000
0x7ffff7ffed70:	0x0000000000000000	0x000000000000000c

struct malloc_context {
	uint64_t secret;
#ifndef PAGESIZE
	size_t pagesize;
#endif
	int init_done;
	unsigned mmap_counter;
	struct meta *free_meta_head;
	struct meta *avail_meta;
	size_t avail_meta_count, avail_meta_area_count, meta_alloc_shift;
	struct meta_area *meta_area_head, *meta_area_tail;
	unsigned char *avail_meta_areas;
	struct meta *active[48];
	size_t usage_by_class[48];
	uint8_t unmap_seq[32], bounces[32];
	uint8_t seq;
	uintptr_t brk;
};

$4 = {
  secret = 5586872504063590636,
  init_done = 1,
  mmap_counter = 0,
  free_meta_head = 0x0,
  avail_meta = 0x55555555a1d0,
  avail_meta_count = 90,
  avail_meta_area_count = 0,
  meta_alloc_shift = 0,
  meta_area_head = 0x55555555a000,
  meta_area_tail = 0x55555555a000,
  avail_meta_areas = 0x55555555b000 <error: Cannot access memory at address 0x55555555b000>,
  active = {0x0, 0x0, 0x55555555a1a8, 0x55555555a0e0, 0x0, 0x0, 0x0, 0x55555555a0b8, 0x0, 0x0, 0x0, 0x55555555a090, 0x0, 0x0, 0x0, 0x55555555a158, 0x0, 0x0, 0x0, 0x55555555a040, 0x0, 0x0, 0x0, 0x55555555a018, 0x0 <repeats 24 times>},
  usage_by_class = {0, 0, 10, 0 <repeats 45 times>},
  unmap_seq = '\000' <repeats 31 times>,
  bounces = '\000' <repeats 31 times>,
  seq = 0 '\000',
  brk = 93824992260096
}

```



```c
// malloc

void *malloc(size_t n)
{
	if (size_overflows(n)) return 0;
	struct meta *g;
	uint32_t mask, first;
	int sc;    // size_class
	int idx;
	int ctr;

    ...
    ...

	sc = size_to_class(n);   // 返回一个size的级别

	rdlock();			// 互斥锁
	g = ctx.active[sc];	//得到一个meta

	// use coarse size classes initially when there are not yet
	// any groups of desired size. this allows counts of 2 or 3
	// to be allocated at first rather than having to start with
	// 7 or 5, the min counts for even size classes.
	if (!g && sc>=4 && sc<32 && sc!=6 && !(sc&1) && !ctx.usage_by_class[sc]) {
		size_t usage = ctx.usage_by_class[sc|1];
		// if a new group may be allocated, count it toward
		// usage in deciding if we can use coarse class.
		if (!ctx.active[sc|1] || (!ctx.active[sc|1]->avail_mask
		    && !ctx.active[sc|1]->freed_mask))
			usage += 3;
		if (usage <= 12)
			sc |= 1;
		g = ctx.active[sc];
	}

	for (;;) {
		mask = g ? g->avail_mask : 0;
		first = mask&-mask;
		if (!first) break;
		if (RDLOCK_IS_EXCLUSIVE || !MT)
			g->avail_mask = mask-first;				// 取消空闲标记
		else if (a_cas(&g->avail_mask, mask, mask-first)!=mask)
			continue;
		idx = a_ctz_32(first);
		goto success;
	}
	upgradelock();

	idx = alloc_slot(sc, n);
	if (idx < 0) {
		unlock();
		return 0;
	}
	g = ctx.active[sc];

success:
	ctr = ctx.mmap_counter;
	unlock();
	return enframe(g, idx, n, ctr);
}
```



```
单个的chunk似乎长这个样子

0x7ffff7ffed10:	0x000055555555a1a8	0x0000a00000000009
0x7ffff7ffed20:	0x6161616161616161	0x6161616161616161
0x7ffff7ffed30:	0x0000000000000000	0x0000000000000000
0x7ffff7ffed40:	0x0000000000000000	0x0003a1000000000c
0x7ffff7ffed50:	0x0000000000000000	0x0000000000000000
0x7ffff7ffed60:	0x0000000000000000	0x0000000000000000
0x7ffff7ffed70:	0x0000000000000000	0x000000000000000c
0x7ffff7ffed80:	0x0000000000000000	0x0000000000000000
0x7ffff7ffed90:	0x0000000000000000	0x0000000000000000

申请0x20 实际分配0x30，前0x10 是啥啊

这个头大概是这样的
|    offset    |  reserved |   index |     unk		   |	size           |
|     16       |     3     |     5   |  	8	       |      32           |


只有一个meta的mem中的所有chunk都被free掉，这个meta才会被加入free_meta的list中

一个free掉的slot：
0x7ffff7ffed70:	0x0000000000000000	0x0000ff000000000c
0x7ffff7ffed80:	0x0000000000000000	0x0000000000000000
0x7ffff7ffed90:	0x0000000000000000	0x0000000000000000

```



IO_FILE

```c
#include <stdio.h>

int main()
{
    unsigned long * p = stdout;
    char * q = p;
    q[0] = 's';
    q[1] = 'h';
    q[2] = '\x00';
    p[4] = 0x1;
    p[9] = 0x7ffff7fa78c8;   // system
    fwrite("Hello", 0x1,0x10,stdout);
    return 0;
}
```

