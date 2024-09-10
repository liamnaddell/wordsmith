#pragma once

#define WORD_MLEN 32

struct wsdb {
	struct rb_root root;
};

typedef enum validity_t {
	W_UNK = 0,
	W_VALID,
	W_INVALID,
} validity_t;

struct word_entry {
	struct rb_node node;
	/* The word itself */
	char keystring[WORD_MLEN];
	validity_t valid;
	/* How many times generated */
	unsigned count;
};

//TODO: Fix wsdb -> ws_db
void ws_db_init(void);
void ws_db_exit(void);
int ws_db_ins(struct word_entry *data);
struct word_entry *ws_db_search(char *string);
struct word_entry *ws_db_gen(void);
