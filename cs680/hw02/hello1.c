/*
 * Most basic hello world external kernel module.
 * Simple logs with level INFO when it is loaded and unloaded.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* TODO: Check if My Name should actually be my name */
static int hello1_init(void) {
  printk(KERN_INFO "My Name: Loading Hello1 module - Hello World 1.\n");
  return 0;
}

static void hello1_exit(void) {
  printk(KERN_INFO "My Name: Exiting Hello1 module - Goodbye World 1.\n");
}

module_init(hello1_init);
module_exit(hello1_exit);
