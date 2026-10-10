#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>

static void free_map(char **m, int n)
{
	for (int i = 0; i < n; i++)
		free(m[i]);
	free(m);
}

static int min(int a, int b)
{
	return (a < b ? a : b);
}

static int bad(char c)
{
	return (c < 32 || c > 126);
}

static int num(char *s, int len)
{
	int n = 0;

	if (len < 1)
		return (-1);
	for (int i = 0; i < len; i++)
	{
		if (s[i] < '0' || s[i] > '9' || n > 100000000)
			return (-1);
		n = n * 10 + s[i] - '0';
	}
	return (n);
}

static int header(FILE *f, int *n, char *e, char *o, char *x)
{
	char *h = NULL;
	size_t cap = 0;
	int l = getline(&h, &cap, f);

	if (l < 5 || h[l - 1] != '\n')
		return (free(h), 0);
	*e = h[l - 4];
	*o = h[l - 3];
	*x = h[l - 2];
	*n = num(h, l - 4);
	if (*n < 0 && l >= 8 && h[l - 3] == ' ' && h[l - 5] == ' ' && h[l - 7] == ' ')
	{
		*e = h[l - 6];
		*o = h[l - 4];
		*x = h[l - 2];
		*n = num(h, l - 7);
	}
	free(h);
	return (*n > 0 && *e != *o && *e != *x && *o != *x
		&& !bad(*e) && !bad(*o) && !bad(*x));
}

static int bsq(FILE *f)
{
	int n, w = 0, bs = 0, bi = 0, bj = 0;
	char e, o, x;
	char **m;
	char *extra = NULL;
	int *dp;
	size_t cap = 0;
	ssize_t len;

	if (!f || !header(f, &n, &e, &o, &x))
		return (0);
	if (!(m = calloc(n, sizeof(char *))))
		return (0);
	for (int i = 0; i < n; i++)
	{
		cap = 0;
		len = getline(&m[i], &cap, f);
		if (len < 2 || m[i][len - 1] != '\n' || (i > 0 && len - 1 != w))
			return (free_map(m, n), 0);
		w = len - 1;
		for (int j = 0; j < w; j++)
		{
			if (m[i][j] != e && m[i][j] != o)
				return (free_map(m, n), 0);
		}
	}
	cap = 0;
	len = getline(&extra, &cap, f);
	free(extra);
	if (len != -1 || !(dp = calloc(n * w, sizeof(int))))
		return (free_map(m, n), 0);
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < w; j++)
		{
			if (m[i][j] == o)
				continue;
			dp[i * w + j] = 1;
			if (i > 0 && j > 0)
				dp[i * w + j] += min(min(dp[(i - 1) * w + j], dp[i * w + j - 1]), dp[(i - 1) * w + j - 1]);
			if (dp[i * w + j] > bs)
			{
				bs = dp[i * w + j];
				bi = i;
				bj = j;
			}
		}
	}
	for (int i = bi - bs + 1; i <= bi; i++)
	{
		for (int j = bj - bs + 1; j <= bj; j++)
		{
			m[i][j] = x;
		}
	}
	for (int i = 0; i < n; i++)
		fputs(m[i], stdout);
	free(dp);
	free_map(m, n);
	return (1);
}

int main(int ac, char *av[])
{
	FILE *f = stdin;

	if (ac > 2)
		return (fputs("Error: too many arguments\n", stdout), 1);
	if (ac == 2 && !(f = fopen(av[1], "r")))
		return (fputs("Error: cannot open file\n", stdout), 1);
	if (!bsq(f))
		fputs("Error: invalid map\n", stdout);
	if (ac == 2)
		fclose(f);
	return (0);
}