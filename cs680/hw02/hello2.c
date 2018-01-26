/*
 * Slightly more complex kernel modules that uses __init and __exit which allows
 * the kernel to optimize memory usage when possible.
 * Also uses __initdata which has similar memory optimizations applied.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* Documentation Constants */
#define LIC     "GPL"
#define AUTHOR  "Zachary Kaplan"
#define DESCR   "Improved first module"

static int hello2_data __initdata = 2;

/* TODO: Check if My Name should actually be my name */
static int __init hello2_init(void) {
  printk(KERN_INFO "My Name: Loading Hello%d module - Hello World %d.\n",
         hello2_data, hello2_data);
  return 0;
}

static void __exit hello2_exit(void) {
  /* NOTE: Can't use hello2_data here because it's freed after init returns */
  printk(KERN_INFO "My Name: Exiting Hello2 module - Goodbye World 2.\n");
}

module_init(hello2_init);
module_exit(hello2_exit);

/* Documentation */

MODULE_LICENSE(LIC);
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCR);
