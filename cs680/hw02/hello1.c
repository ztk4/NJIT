/*
 * Most basic hello world external kernel module.
 * Simple logs with level INFO when it is loaded and unloaded.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* Documentation Constants */
#define LIC     "GPL"
#define AUTHOR  "Zachary Kaplan"
#define DESCR   "My first module"

static int hello1_init(void) {
  printk(KERN_INFO AUTHOR ": Loading Hello1 module - Hello World 1.\n");
  return 0;
}

static void hello1_exit(void) {
  printk(KERN_INFO AUTHOR ": Exiting Hello1 module - Goodbye World 1.\n");
}

module_init(hello1_init);
module_exit(hello1_exit);

/* Documentation */

MODULE_LICENSE(LIC);
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCR);
