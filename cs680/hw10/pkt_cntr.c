#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

/* Documentation Macros */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zachary Kaplan");
MODULE_DESCRIPTION("Prints some INET packet stats aggregated over 1 minute "
                   "periods.");

/* For showing information in the proc entry */
static int pkt_cntr_show(struct seq_file *file, void *data) {
  return 0;  /* TODO: This. */
}

static int pkt_cntr_open(struct inode *inode, struct file *file) {
  try_module_get(THIS_MODULE);
  return single_open(file, pkt_cntr_show, NULL);
}

static int pkt_cntr_release(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  return single_release(inode, file);
}

/* Struct for entry in proc fs */
static struct proc_dir_entry *pkt_cntr_fentry;
static const struct file_operations pkt_cntr_fops = {
  .owner    = THIS_MODULE,
  .open     = pkt_cntr_open,
  .read     = seq_read,
  .llseek   = seq_lseek,
  .release  = pkt_cntr_release,
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
