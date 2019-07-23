#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include "util.h"
#include "atom.h"
#include "log.h"

#define HASHTAB_NBUCKETS 2048

static unsigned ncollision;

struct hashtab_node {
	uint32 hash;
	uint32 len;
	const char *str;
	struct hashtab_node *next;
};

struct hashtab {
	uint32 n;
	struct hashtab_node *buckets[HASHTAB_NBUCKETS];
} g_atomtab;

static uint32 hashfunc(const char *s, size_t n)
{
	uint32 h = 0x811c9dc5;
	const byte *p = (const byte *)s;

	while (n--) {
		h = (*p++ ^ h) * 16777619;
	}

	return h;
}

static struct hashtab_node *hashtab_node(const char *str, size_t len)
{
	struct hashtab_node *node = xmalloc(sizeof(*node));
	node->hash = hashfunc(str, len);
	node->str = str;
	node->next = NULL;
	return node;
}

static inline bool hashtab_node_equals(const struct hashtab_node *a,
				       const struct hashtab_node *b)
{
	if (a->hash == b->hash) {
		if (a->len == b->len) {
			return !memcmp(a->str, b->str, a->len);
		}
		log_debug(
			"hash collision: '%s' and '%s'. collision_cnt = %4u\n",
			a->str, b->str, ++ncollision);
	}
	return false;
}

static inline double loadfactor(struct hashtab *ht)
{
	return (double)ht->n / HASHTAB_NBUCKETS;
}

static const char *hashtab_insert(struct hashtab *ht, const char *str,
				  size_t len)
{
	if (loadfactor(ht) > 8.) {
		log_warn("hashtable overfill: nstrings = %u loadfac = %.2lf\n",
			 ht->n, loadfactor(ht));
	}

	struct hashtab_node *const node = hashtab_node(str, len);
	assert(node != NULL);

	const size_t i = node->hash % HASHTAB_NBUCKETS;

	struct hashtab_node *iter = ht->buckets[i];
	while (iter) {
		if (hashtab_node_equals(node, iter)) {
			free(node);
			return iter->str;
		}
		iter = iter->next;
	}

	node->next = ht->buckets[i];
	ht->buckets[i] = node;

	return str;
}

const char *atom_fromstr(const char *str)
{
	assert(str != NULL);
	return atom_fromstrwlen(str, strlen(str));
}

const char *atom_fromstrwlen(const char *str, size_t len)
{
	assert(str != NULL);
	return hashtab_insert(&g_atomtab, str, len);
}

