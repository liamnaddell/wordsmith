#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include "ws_db.h"

static int ws_threadfn(void *data) {
    struct word_entry *we;
    while (!kthread_should_stop()) {
        we = ws_db_gen();
        if (we)
            printk(KERN_INFO "Generated word: %s\n", we->keystring);
        else
            printk(KERN_INFO "Failed to gen\n");

        fsleep(1000000);
    }
    printk(KERN_INFO "Wordsmith: received stop signal\n");
    return 0;
}
static struct task_struct *ws_tfn = NULL;
static int ws_init(void) {
    printk(KERN_INFO "Wordsmith: Initializing\n");
    bool success = ws_db_init();
    if (!success)
        return -1;

    ws_tfn = kthread_run(ws_threadfn, NULL, "ws_worker_thread");
    return 0;
}

static void ws_exit(void) {
    printk(KERN_INFO "Wordsmith: Exiting\n");
    if (ws_tfn != NULL)
        kthread_stop(ws_tfn);
}

module_init(ws_init);
module_exit(ws_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Liam Naddell");
MODULE_DESCRIPTION("Wordsmith Kernel Module");
