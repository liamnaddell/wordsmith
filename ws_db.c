#include <linux/rbtree.h>
#include <linux/slab.h>
#include <crypto/rng.h>
#include "ws_db.h"

struct crypto_rng *rng = NULL;
static struct wsdb WSDB = { .root = RB_ROOT };

static int get_random_numbers(u8 *buf, unsigned int len)
{
    char *drbg = "drbg_nopr_sha256"; /* Hash DRBG with SHA-256, no PR */
    int ret;

    if (!buf || !len) {
        pr_debug("No output buffer provided\n");
        return -EINVAL;
    }

    if (rng == NULL) {
	    rng = crypto_alloc_rng(drbg, 0, 0);
	    //TODO: Error handling, perform setup in init.
	    if (IS_ERR(rng)) {
		printk(KERN_INFO "could not allocate RNG handle for %s\n", drbg);
		return PTR_ERR(rng);
	    }
	    crypto_rng_reset(rng, NULL, 0);
    }


    //TODO:figure out log levels properly
    ret = crypto_rng_get_bytes(rng, buf, len);
    if (ret < 0)
        printk(KERN_INFO "generation of random numbers failed\n");
    else if (ret == 0)
        printk(KERN_INFO "RNG returned no data");
    else
        printk(KERN_INFO "RNG returned %d bytes of data\n", ret);

    //TODO:
    //crypto_free_rng(rng);
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

static void force_valid_word(char *w, unsigned len) {
	for (unsigned i = 0; i < len; i++) {
		char c=w[i];
		c %= 52;
		if (c < 26)
			c=0x41+c;
		else
			c=0x61+c-26;
		w[i]=c;
	}
	w[len-1]='\0';
}

struct word_entry *ws_db_gen(void) {
	struct word_entry *entry = kzalloc(sizeof(struct word_entry), GFP_KERNEL);
	unsigned len = 0; 
	int ret;
	get_random_numbers((u8 *) &len,1);
	len = len % WORD_MLEN;
	printk(KERN_INFO "Wordsmith: rng len: %d\n",len);

	//TODO: Fix non-ascii characters
	//TODO: Fix null byte (add 1 to mlen in word_entry)
	ret = get_random_numbers((u8 *) entry->keystring,len);
	force_valid_word(entry->keystring,len);
	/* TODO: Free bug */
	if (ret == -1)
		return NULL;
	entry->keystring[WORD_MLEN-1] = '\0';

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
