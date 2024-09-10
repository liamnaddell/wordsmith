#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include "ws_db.h"

static int ws_threadfn(void *data) {
	for (int i = 0; i < 5; i++) {
		printk(KERN_INFO "Generated word: %s\n", ws_db_gen()->keystring);
	}
	return 0;
}


//TODO: Add module arguments (db size)
//TODO: Add argument for verbose debugging.
static int ws_init(void) {
	printk(KERN_INFO "Wordsmith: Initializing\n");
	ws_db_init();

	struct task_struct *ws_tfn = kthread_run(ws_threadfn, NULL, "ws_worker_thread");
	fsleep(1000);
	printk(KERN_INFO "Wordsmith: ws_tfn: %p\n",ws_tfn);
	return 0;
}

static void ws_exit(void) {
	printk(KERN_INFO "Wordsmith: Exiting\n");
}

module_init(ws_init);
module_exit(ws_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Liam Naddell");
MODULE_DESCRIPTION("Wordsmith Kernel Module");
