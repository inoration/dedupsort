/**
 * Data generator for input testing data generation
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	unsigned long long	m0 = 0, m = 0, l = 0;
	unsigned long n = 0;
	unsigned long i = 0;

	srandom((unsigned int)getpid());

	if (argc < 4) {
		printf("Usage: %s NUM MIN MAX\n", argv[0]);
		return 0;
	}

	sscanf(argv[1], "%lu", &n);
	sscanf(argv[2], "%llu", &l);
	sscanf(argv[3], "%llu", &m);

	printf("%lu\n", n);

	for (; i < n; i ++) {
		if (i) {
			printf(" ");
		}
		m0 = ((unsigned long long)random()) << 31 + random();
		printf("%llu", m0 % (m - l + 1) + l);
	}
	printf("\n");
	
	return 0;
}
