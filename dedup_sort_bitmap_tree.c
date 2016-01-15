/**
 * Problem Description:
 *      Sorting and deduplication are the basis in data compression and
 * optimization. Now there are N big numbers between 0 and 9999999999 generated
 * by computer randomly, where n <= 500000. Your task is to get rid of repeated
 * numbers, and sort them.  (The judge system is a 32-bit system with memory
 * limit of <=512M.)
 *      (EMC Shanghai COE Coding Contest 2014, Round 1)
 *
 * Program Description:
 *      This Program is based on bitmap tree with algorithm optimization to
 * achieve O(N) on both time and space complexity. Meanwhile, optimization and
 * performance tuning were done on implementation to make it run faster, e.g.
 * Cache optimization, I/O optimization.
 *	(This is the version submitted to online judge system of the contest)
 *
 * Author:
 *      Windyon(Wenyu) Zhou <windyon9@gmail.com>
 *
 * Copyright (C) 2014, All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef int			s32;
typedef unsigned int		u32;
typedef unsigned long long	u64;
typedef u32			BOOL;
typedef u32			E;

//#define	ASM_OPT

#define	TRUE			1
#define FALSE			0

#define TOTAL_BITS		34
#define BYTE_BITS		3
#define BYTE_MASK		((1 << BYTE_BITS) - 1)
#define BITS_PER_BYTE		(1 << BYTE_BITS)
#define BTABLE_ORDERS		(BITS_PER_BYTE << 1)
#define BTABLE_SIZE		(1 << BTABLE_ORDERS)
#define BTREE_HEIGHT		4
#define BTREE_BYTE_DEGREE_BITS	3
#define BTREE_BYTE_DEGREE	(sizeof(u64))
#define BTREE_BIT_DEGREE_BITS	(BYTE_BITS + BTREE_BYTE_DEGREE_BITS)
#define BTREE_BIT_DEGREE	(1 << BTREE_BIT_DEGREE_BITS)

#define	U64_IN_LEN		16

typedef struct _EL {
	E*	base;
	E*	head;
	E*	tail;
	u32	count;
} EL;

char*	buf = NULL;
char*	bufp = NULL;
u64*	A = NULL;
E*	B = NULL;
EL*	elist = NULL;
u32	elist_length = 0;
u32	buf_size = 0;

u8	btable[BTABLE_SIZE] = {0};

u8*	btree[BTREE_HEIGHT] = {0};
u32	btree_width[BTREE_HEIGHT] = {0};
u32	btree_path[BTREE_HEIGHT] = {0};

u32	blist_shift = 0;
u32	blist_mask = 0;

inline u8	order32(u32 n)
{
	return (n >> 16) ? 16 + btable[n >> 16] : btable[n];
}

#ifdef ASM_OPT
inline u64	u32_suffix_0(u64 n)
{
	u64 ret;

	__asm__ __volatile__ (
		"tzcnt %1, %0"
		: "=b"(ret)
		: "a"(n), "b"(ret)
		: "1"
	);

	return ret;
}

inline u32	suffix_0(u64 n)
{
	return ((u32)n) ? u32_suffix_0((u32)n) : 32 + u32_suffix_0(n >> 32);
}
#endif

inline u8	get_min_bit_and_clear(u64 * d)
{
	u64	m = (*d) - ((*d) & (*d - 1));
	(*d) -= m;

#ifdef ASM_OPT
	return suffix_0(m);
#else
	return (m >> 32) ? 32 + order32(m >> 32) : order32(m);
#endif
}

inline BOOL	get_bit(u8* blist, u32 n)
{
	return blist[n >> BYTE_BITS] & ( 1 << (n & BYTE_MASK));
}

inline void	set_bit(u8* blist, u32 n)
{
	blist[n >> BYTE_BITS] |= ( 1 << (n & BYTE_MASK));
}

inline BOOL	get_bit_and_set(u8* blist, u32 n)
{
	BOOL b = get_bit(blist, n);

	if (!b) {
		set_bit(blist, n);
	}

	return b;
}

inline void	pute(EL* list, E e)
{
	*list->tail ++ = e;
}

inline E	gete(EL* list)
{
	return *list->head ++;
}

inline E	gete_rev(EL* list)
{
	return * -- list->tail;
}

inline void	list_rewind(EL* list)
{
	list->tail = list->head = list->base;
}

u32	bsort(EL* elist)
{
	register s32	i = 0, j = 0;
	register u32	n = 0;
	u64*		bm = NULL;
	u32		count = 0;

	while (elist->tail > elist->head) {
		n = gete(elist);
		
		for (i = BTREE_HEIGHT - 1; i >= 0; i --) {
			if (get_bit_and_set(btree[i], n)) {
				break;
			}
			n >>= BTREE_BIT_DEGREE_BITS;
		}
	}

	i = j = 0;
	list_rewind(elist);
	bm = (u64 *)&btree[0][0];
	
	while (i >= 0) {
		if (*bm) {
			n = (j << BYTE_BITS) + get_min_bit_and_clear(bm);
			if (i == BTREE_HEIGHT - 1) {
				pute(elist, n);
				count ++;
			} else {
				btree_path[i] = j;
				i ++;
				j = n << BTREE_BYTE_DEGREE_BITS;
				bm = (u64 *)&btree[i][j];
			}
		} else {
			i --;
			j = btree_path[i];
			bm = (u64 *)&btree[i][j];
		}
	}

	return count;
}

void	init(u32 N)
{
	u32	i = 0, n = 0;

	buf_size = N * U64_IN_LEN;
	buf = (char *)malloc(sizeof(char) * buf_size);
	bufp = buf;

	A = (u64 *)malloc(sizeof(u64) * N);
	B = (E *)malloc(sizeof(E) * N);

	for (i = 0; i < BTABLE_ORDERS; i ++) {
		btable[1 << i] = i;
	}

	n = 0;
	for (i = 0; i < BTREE_HEIGHT; i ++) {
		n += BTREE_BIT_DEGREE_BITS;
		btree_width[i] = (1 << n) >> BYTE_BITS;
		btree[i] = (u8 *)malloc(sizeof(u8) * btree_width[i]);
	}

	blist_shift = n;
	blist_mask = (1 << blist_shift) - 1;

	elist_length = 1 << (TOTAL_BITS - n);
	elist = (EL *)malloc(sizeof(EL) * elist_length);
	memset(elist, 0, sizeof(EL) * elist_length);
}

void	fini()
{
	u32	i = 0;

	free(buf);
	free(A);
	free(B);
	free(elist);
	
	for (; i < BTREE_HEIGHT; i ++) {
		free(btree[i]);
	}
}

inline u64 getu64()
{
	u64	n = 0;
	int	c = 0;

	while (!isdigit(c = *bufp ++))
		;
	n = c - '0';
	while (isdigit(c = *bufp ++)) {
		n = n * 10 + c - '0';
	}
	
	return n;
}

inline void putu64(u64 n)
{
	do {
		* -- bufp = n % 10 + '0';
		n /= 10;
	} while (n);
	* -- bufp = ' ';
}

u32	input()
{
	register u32	i = 0;
	u32		N = 0;

	scanf("%d\n", &N);

	init(N);

	fread(buf, buf_size, sizeof(char), stdin);

	for (; i < N; i ++) {
		//scanf("%llu", &A[i]);
		A[i] = getu64();
		elist[A[i] >> blist_shift].count ++;
	}

	elist[0].base = B;
	list_rewind(&elist[0]);
	for (i = 1; i < elist_length; i ++) {
		elist[i].base = elist[i - 1].base + elist[i - 1].count;
		list_rewind(&elist[i]);
	}

	for (i = 0; i < N; i ++) {
		pute(&elist[A[i] >> blist_shift], A[i] & blist_mask);
	}

	return N;
}

void	output(u32 N)
{
	register u64	d = 0;
	register s32	i = 0;

	bufp = buf + buf_size;
	* -- bufp = '\n';

	for (i = elist_length - 1; i >= 0; i --) {
		while (elist[i].tail > elist[i].head) {
			d = (u64)i << blist_shift | gete_rev(&elist[i]);
			putu64(d);
		}
	}

	*bufp = '\n';
	putu64(N);

	fwrite(bufp + 1, buf + buf_size - 1 - bufp, sizeof(char), stdout);

	fini();

}

int	main(int argc, char **argv)
{
	u32	i = 0;
	u32	N = 0;

	input();

	for (i = 0; i < elist_length; i ++) {
		if (elist[i].tail > elist[i].head) {
			N += bsort(&elist[i]);
		}
	}

	output(N);

	return 0;
}
