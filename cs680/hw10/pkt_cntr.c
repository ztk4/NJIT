#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/timer.h>

/* Documentation Macros */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zachary Kaplan");
MODULE_DESCRIPTION("Prints some INET packet stats aggregated over 1 minute "
                   "periods.");

static const unsigned kPeriodMs = 60 * 1000;  /* 1 minute period for stats */
static const unsigned kPollUs = 10;           /* 10 us period for polling */

/* Aggregate statistics. */
static u64 nr_pkt_out     = 0;
static u64 nr_pkt_in      = 0;
static u64 nr_pkt_merges  = 0;
static u64 nr_enqueue     = 0;
static u64 nr_dequeue     = 0;
static u64 nr_requeue     = 0;
static struct mutex aggregate_mutex;  /* Guards above aggregates */

/* Buffer for output string. */
#define PKT_CNTR_BUFF_SIZE (1 << 10)
static char buff[PKT_CNTR_BUFF_SIZE] = {0};

/* For showing information in the proc entry */
static int pkt_cntr_show(struct seq_file *file, void *p) {
  seq_puts(file, buff);
  return 0;
}

static int pkt_cntr_open(struct inode *inode, struct file *file) {
  try_module_get(THIS_MODULE);
  return single_open(file, pkt_cntr_show, NULL);
}

static int pkt_cntr_release(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  return single_release(inode, file);
}

/* Timer periodic on kPeriodMs milliseconds. */
struct timer_list pkt_cntr_aggregate_timer;
/* Struct for entry in proc fs. */
static struct proc_dir_entry *pkt_cntr_fentry;
/* Entry name in the /proc dir. */
static const char *const kEntryName = "pkt_cntr";

/* Called every 1 minute, prints current aggregates and then resets */
static void pkt_cntr_flush_aggregates(struct timer_list *timer) {
  int n;
  /* Tell timer to call us again in one period. */
  mod_timer(timer, jiffies + msecs_to_jiffies(kPeriodMs));

  /* Do periodic work (holding lock on mutex for aggregates). */
  mutex_lock(&aggregate_mutex); /* LOCK */
  n = snprintf(buff, PKT_CNTR_BUFF_SIZE,
      "INET Packet Statistics (aggregated over 1 minute):\n"
      "Number of outgoing packets   : %llu\n"
      "Number of incoming packets   : %llu\n"
      "Number of queue merges       : %llu\n"
      "Number of enqueue operations : %llu\n"
      "Number of dequeue operations : %llu\n"
      "Number of requeue operations : %llu\n",
      nr_pkt_out, nr_pkt_in, nr_pkt_merges, nr_enqueue, nr_dequeue, nr_requeue);
  nr_pkt_out    = 0;
  nr_pkt_in     = 0;
  nr_pkt_merges = 0;
  nr_enqueue    = 0;
  nr_dequeue    = 0;
  nr_requeue    = 0;
  mutex_unlock(&aggregate_mutex); /* UNLOCK */

  if (n < 0) {
    printk(KERN_WARNING "Zachary Kaplan: Failed to update %s\n", kEntryName);
    proc_set_size(pkt_cntr_fentry, 0);  /* Failure */
  } else if (n > PKT_CNTR_BUFF_SIZE) {
    printk(KERN_INFO "Zachary Kaplan: Buffer size not large enough for %s\n",
           kEntryName);
    proc_set_size(pkt_cntr_fentry, PKT_CNTR_BUFF_SIZE);  /* Okay.. */
  } else {
    proc_set_size(pkt_cntr_fentry, n);  /* Success */
  }
}

/* Struct for file operations allowed on pkt_cntr entry. */
static const struct file_operations pkt_cntr_fops = {
  .owner    = THIS_MODULE,
  .open     = pkt_cntr_open,
  .read     = seq_read,
  .llseek   = seq_lseek,
  .release  = pkt_cntr_release,
};

static int __init pkt_cntr_init(void) {
  /* Create proc fs entry (default mode 0 is 0444). */
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

  /* Initialize mutex */
  mutex_init(&aggregate_mutex);

  /* Initialize periodic timer for publishing aggregates */
  timer_setup(&pkt_cntr_aggregate_timer, pkt_cntr_flush_aggregates, 0);
  /* Start timer loop by calling timer routine */
  pkt_cntr_flush_aggregates(&pkt_cntr_aggregate_timer);

  return 0;
}

static void __exit pkt_cntr_exit(void) {
  /* Destroy timer and mutex */
  del_timer(&pkt_cntr_aggregate_timer); 
  mutex_destroy(&aggregate_mutex);

  /* Then remove proc fs entry. */
  proc_remove(pkt_cntr_fentry);
  printk(KERN_INFO "Zachary Kaplan: /proc/%s entry removed\n", kEntryName);
}

module_init(pkt_cntr_init);
module_exit(pkt_cntr_exit);
