#include <linux/rbtree.h>
#include <linux/slab.h>
#include <crypto/rng.h>
#include "ws_db.h"

struct crypto_rng *rng = NULL;
static struct wsdb WSDB = { .root = RB_ROOT };

//TODO: Cache RNG. or accept seed value 
static int get_random_numbers(u8 *buf, unsigned int len)
{
    char *drbg = "drbg_nopr_sha256"; /* Hash DRBG with SHA-256, no PR */
    int ret;

    if (!buf || !len) {
        pr_debug("No output buffer provided\n");
        return -EINVAL;
    }

    rng = crypto_alloc_rng(drbg, 0, 0);
    if (IS_ERR(rng)) {
        pr_debug("could not allocate RNG handle for %s\n", drbg);
        return PTR_ERR(rng);
    }

    ret = crypto_rng_get_bytes(rng, buf, len);
    if (ret < 0)
        pr_debug("generation of random numbers failed\n");
    else if (ret == 0)
        pr_debug("RNG returned no data");
    else
        pr_debug("RNG returned %d bytes of data\n", ret);

    crypto_free_rng(rng);
    return ret;
}

//TODO: kmemleak check
int ws_db_ins(struct word_entry *data) {
	struct rb_root *root = &WSDB.root;
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		struct word_entry *this = container_of(*new, struct word_entry, node);
		int result = strcmp(data->keystring, this->keystring);

		parent = *new;
		if (result < 0)
			new = &((*new)->rb_left);
		else if (result > 0)
			new = &((*new)->rb_right);
		else
			return 0;
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&data->node, parent, new);
	rb_insert_color(&data->node, root);

	return 1;
}

//TODO: Impose a maximum amount of rb nodes
//TODO: Remove rb_root from the API
struct word_entry *ws_db_search(char *string) {
	struct rb_root *root = &WSDB.root;
	struct rb_node *node = root->rb_node;

	while (node) {
		struct word_entry *data = container_of(node, struct word_entry, node);
		int result;

		result = strcmp(string, data->keystring);

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}

struct word_entry *ws_db_gen(void) {
	struct word_entry *entry = kzalloc(sizeof(struct word_entry), GFP_KERNEL);
	u8 len; 
	get_random_numbers(&len,1);
	len = len < WORD_MLEN ? len : WORD_MLEN;

	//TODO: Fix non-ascii characters
	//TODO: Fix null byte (add 1 to mlen in word_entry)
	get_random_numbers((u8 *) &entry->keystring,len);
	entry->keystring[WORD_MLEN-1] = '\0';
	memcpy(entry->keystring,"obamna",5);

	ws_db_ins(entry);

	return entry;
}

void ws_db_init(void) {
	printk(KERN_INFO "Wordsmith: WSDB Init\n");
}

static void ws_db_free(struct rb_node *node) {
	if (node == NULL)
		return;
	ws_db_free(node->rb_left);
	ws_db_free(node->rb_right);
	struct word_entry *this = container_of(node,struct word_entry,node);
	rb_erase(node, &WSDB.root);
	kfree(this);
}

void ws_db_exit(void) {
	printk(KERN_INFO "Wordsmith: WSDB Exit\n");
	ws_db_free(WSDB.root.rb_node);
}
