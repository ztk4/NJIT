#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

/* Documentation Macros */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zachary Kaplan");
MODULE_DESCRIPTION("Prints some INET packet stats aggregated over 1 minute "
                   "periods.");

/* Supported file operations for this module */
static int pkt_cntr_fopen(struct inode *inode, struct file *file) {
  return !try_module_get(THIS_MODULE);
}
static int pkt_cntr_fclose(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  return 0;
}
static ssize_t
pkt_cntr_fread(struct file *file, char __user *buf, size_t len, loff_t *loff) {
  return 0;  /* TODO: This */
}

/* Struct for entry in proc fs */
static struct proc_dir_entry *pkt_cntr_fentry;
/* Struct for file ops on our proc entry */
static const struct file_operations pkt_cntr_fops = {
  .read     = pkt_cntr_fread,
  .open     = pkt_cntr_fopen,
  .release  = pkt_cntr_fclose,
};
/* Entry name in the /proc dir. */
static const char *const kEntryName = "pkt_cntr";

static int __init pkt_cntr_init(void) {
  /* Create proc fs entry. (default mode 0 is 0444). */
  pkt_cntr_fentry = proc_create(kEntryName, 0, NULL, &pkt_cntr_fops);
  if (!pkt_cntr_fentry) {
    printk(KERN_ALERT "Zachary Kaplan: Unable to open /proc/%s\n", kEntryName);
    return -ENOMEM;
  }

  /* Set user and group ownership to root */
  proc_set_user(pkt_cntr_fentry, KUIDT_INIT(0), KGIDT_INIT(0));
  /* Set size to 0 initially */
  proc_set_size(pkt_cntr_fentry, 0);

  printk(KERN_INFO "Zachary Kaplan: /proc/%s entry created\n", kEntryName);

  return 0;
}

static void __exit pkt_cntr_exit(void) {
  /* Remove proc fs entry. */
  proc_remove(pkt_cntr_fentry);
  printk(KERN_INFO "Zachary Kaplan: /proc/%s entry removed\n", kEntryName);
}

module_init(pkt_cntr_init);
module_exit(pkt_cntr_exit);
