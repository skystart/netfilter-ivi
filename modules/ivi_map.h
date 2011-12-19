/* File name     :  ivi_map.h
 * Author        :  Wentao Shang
 * 
 * Contents      :
 *    This file is the header file for the 'ivi_map.c' file,
 *    which contains all the system header files and definitions
 *    used in the 'ivi_map.c' file.
 *
 */

#ifndef IVI_MAP_H
#define IVI_MAP_H

#include <linux/module.h>

#include <linux/time.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include "ivi_config.h"

#ifndef NIP4
#define NIP4(addr) \
	((unsigned char *)&addr)[3], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[0]
#define NIP4_FMT "%u.%u.%u.%u"
#endif

#ifndef NIP6
#define NIP6(addr) \
	ntohs((addr).s6_addr16[0]), \
	ntohs((addr).s6_addr16[1]), \
	ntohs((addr).s6_addr16[2]), \
	ntohs((addr).s6_addr16[3]), \
	ntohs((addr).s6_addr16[4]), \
	ntohs((addr).s6_addr16[5]), \
	ntohs((addr).s6_addr16[6]), \
	ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"
#endif


/* map entry structure */
struct map_tuple {
#ifdef IVI_HASH
	struct hlist_node out_node;  // Inserted to out_chain
	struct hlist_node in_node;   // Inserted to in_chain
#else
	struct list_head node;
#endif
	__be32 oldaddr;
	__be16 oldport;
	__be16 newport;
	struct timeval timer;
};

/* map list structure */
struct map_list {
	spinlock_t lock;
#ifdef IVI_HASH
	struct hlist_head out_chain[IVI_HTABLE_SIZE];  // Map table from oldport to newport
	struct hlist_head in_chain[IVI_HTABLE_SIZE];   // Map table from newport to oldport
#else
	struct list_head chain;
#endif
	int size;
	__be16 last_alloc;  // Save the last allocate port number
	time_t timeout;
};

#ifdef IVI_HASH
// Generic hash function for a 16 bit value, see 'Introduction to Algorithms, 2nd Edition' Section 11.3.2
__inline int port_hashfn(__be16 port)
{
	unsigned int m = port * GOLDEN_RATIO_16;
	return ((m & 0xf800) >> 11);  // extract 11bit to 16bit as hash result
}

// Generic hash function for a 32 bit value, see 'Introduction to Algorithms, 2nd Edition' Section 11.3.2
__inline int v4addr_port_hashfn(__be32 addr, __be16 port)
{
	__be32 m = addr + port;
	m *= GOLDEN_RATIO_32;
	return ((m & 0xf8000000) >> 27);
}
#endif

/* global map list variables */
extern __be16 ratio;
extern __be16 offset;
extern __be16 suffix;
extern __be16 adjacent;

extern struct map_list udp_list;
extern struct map_list icmp_list;

/* list operations */
extern void init_map_list(struct map_list *list, time_t timeout);
extern void refresh_map_list(struct map_list *list);
extern void free_map_list(struct map_list *list);

/* mapping operations */
extern int get_outflow_map_port(__be32 oldaddr, __be16 oldp, struct map_list *list, __be16 *newp);
extern int get_inflow_map_port(__be16 newp, struct map_list *list, __be32 *oldaddr, __be16 *oldp);

#endif /* IVI_MAP_H */
