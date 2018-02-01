/*
 * Again a slightly more complex version of the previous module.
 * This time we use MODULE_PARAM to define inputs to the module.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

/* Documentation Constants */
#define LIC     "GPL"
#define AUTHOR  "Zachary Kaplan"
#define DESCR   "Improved first module, now with parameters!"

/* Global parameter storage */
static char *recipient     = "Mr. John Doe";
static char *addr_lines[2] = { "Main Street", NULL };
static int num_addr_lines  = 1;
static char *city          = "Suffragette City";
static int zip             = 12345;

/* Parameter definition and documentation */
module_param(recipient, charp, 0);
MODULE_PARM_DESC(recipient, "Recipient in mailing address");

module_param_array(addr_lines, charp, &num_addr_lines, 0);
MODULE_PARM_DESC(addr_lines, "Address lines, max 2");

module_param(city, charp, 0);
MODULE_PARM_DESC(city, "City in mailing address");

module_param(zip, int, 0);
MODULE_PARM_DESC(zip, "Zip code in mailing address");

static int __init hello3_init(void) {
  printk(KERN_INFO "Loading Hello 3: Hello World 3\n"
                   "==============================\n"
                   "Recipient  : %s\n"
                   "Address[0] : %s\n"
                   "Address[1] : %s\n"
                   "City       : %s\n"
                   "Zip Code   : %05d\n",
                   recipient,
                   (num_addr_lines > 0 ? addr_lines[0] : "NULL"),
                   (num_addr_lines > 1 ? addr_lines[1] : "NULL"),
                   city, zip);

  return 0;
}

static void __exit hello3_exit(void) {
  printk(KERN_INFO AUTHOR ": Goodbye World 3\n");
}

module_init(hello3_init);
module_exit(hello3_exit);

/* Documentation */

MODULE_LICENSE(LIC);
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCR);
