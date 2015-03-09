#include "sched.h"
#include <linux/syscalls.h>
#include <linux/printk.h>
/*
 * mycfs-task scheduling class.
 *
 * (NOTE: these are not related to SCHED_IDLE tasks which are
 *  handled in sched/fair.c)
 */
SYSCALL_DEFINE2(sched_setlimit,pid_t, pid, int, limit){
    printk(KERN_EMERG "for pid(%d) , limit is %d\n",(int) pid, limit);
    return 0;
}
#define CONFIG_SMP
#define SELIST_SIZE 1000

static struct sched_entity * selist[SELIST_SIZE];
static int head=0;
static int tail=0;

static inline struct task_struct *task_of(struct sched_entity *se)
{
	return container_of(se, struct task_struct, se);
}

static inline struct rq *rq_of(struct cfs_rq *cfs_rq)
{
	return container_of(cfs_rq, struct rq, cfs);
}

static void resched_task_mycfs(struct task_struct * p){
    resched_task(p);
}

//EXPORT_SYMBOL(sys_sched_setlimit);
/*
 * Idle tasks are unconditionally rescheduled:
 */
static void check_preempt_curr_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
    struct task_struct *curr = rq->curr;
    struct sched_entity *se = &curr->se, *pse = &p->se;
    
    if (se == pse) return;
    
    resched_task(curr);
}

static struct task_struct *pick_next_task_mycfs(struct rq *rq)
{
	struct task_struct *p;  
        int i=0;

        if (rq->curr == p) {
            printk(KERN_EMERG "picked task : %s at i = %d, pid= %d is already in rq of cpu(%d)\n"
                    ,p->comm, i, (int) p->pid,cpu_of(rq));            
            return NULL;
        }
        
        
        // linear search for an a task
        for (i=0; i< SELIST_SIZE; i++){
            if (selist[i] != 0) break; 
        }
        
        if (i < SELIST_SIZE) {
            p = task_of(selist[i]);
#ifdef CONFIG_SMP
	printk(KERN_EMERG "selected task : %s at i = %d, pid= %d is on cpu(%d) and picking for rq of cpu(%d)\n"
                    ,p->comm, i, (int) p->pid,p->on_cpu,cpu_of(rq));
#endif
            printk(KERN_EMERG "picked task : %s at i = %d, pid= %d, cpu = %d \n"
                    ,p->comm, i, (int) p->pid, cpu_of(rq));
            return p;
        }
        else {
//            printk(KERN_EMERG "no task to pick delegate to idle scheduler\n");
            //return idle_sched_class.pick_next_task(rq);
            return NULL;
        }
}

static void
enqueue_task_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
    int i = 0;
    printk(KERN_EMERG "enqueue first task at 0 %s\n",p->comm);
    selist[tail] = &p->se;
    tail++;
    inc_nr_running(rq);
}

/*
 * It is not legal to sleep in the mycfs task - print a warning
 * message if some code attempts to do it:
 */
static void
dequeue_task_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
    int i =0;
    
    printk(KERN_EMERG "dequeue task %s\n",p->comm);

    for (i=0; i < tail; i++){
        if (selist[i] == &p->se) break;
    }
    
    if (i < tail){
        //found
        printk(KERN_EMERG "task %s found in running-queue at i=%d\n",p->comm,i);
        for (; i < tail; i++){
            selist[i] = selist[i+1];
        }
        selist[i] = 0;
        tail--;
        printk(KERN_EMERG "tail was (%d), now is (%d)\n",(tail+1),tail);
        
    }
    else{
        printk(KERN_EMERG "task %s does not exist in running-queue\n",p->comm);
    }
    dec_nr_running(rq);
}

static void put_prev_task_mycfs(struct rq *rq, struct task_struct *prev)
{
}

static void task_tick_mycfs(struct rq *rq, struct task_struct *curr, int queued)
{
    //
}

static void set_curr_task_mycfs(struct rq *rq)
{
}

static void switched_to_mycfs(struct rq *rq, struct task_struct *p)
{
	if (!p->se.on_rq)
		return;
	if (rq->curr == p){
            printk(KERN_EMERG "handling task %s , reschedule task\n",p->comm);
            resched_task(rq->curr);
        }
	else
        {
            printk(KERN_EMERG "going to check preempt task %s\n",p->comm);
            check_preempt_curr_mycfs(rq, p, 0);
        }
}

static void
prio_changed_mycfs(struct rq *rq, struct task_struct *p, int oldprio)
{
	BUG();
}

static unsigned int get_rr_interval_mycfs(struct rq *rq, struct task_struct *task)
{
	return 0;
}

/*
 * sched_yield() is very simple
 *
 * The magic of dealing with the ->skip buddy is in pick_next_entity.
 */
static void yield_task_mycfs(struct rq *rq)
{
    //dequeue_task_mycfs(rq);
    //enqueue_task_mycfs(rq);    
}

static bool yield_to_task_mycfs(struct rq *rq, struct task_struct *p, bool preempt)
{
    return false;
}

/*
 * Preempt the current task with a newly woken task if needed:
 */
static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_flags)
{}

/*
 * called on fork with the child task as argument from the parent's context
 *  - child not yet on the tasklist
 *  - preemption disabled
 */
static void task_fork_mycfs(struct task_struct *p)
{}


static void switched_from_mycfs(struct rq *rq, struct task_struct *p)
{}

#ifdef CONFIG_FAIR_GROUP_SCHED
static void task_move_group_fair(struct task_struct *p, int on_rq)
{
	
}
#endif

#ifdef CONFIG_SMP
/*
 * sched_balance_self: balance the current task (running on cpu) in domains
 * that have the 'flag' flag set. In practice, this is SD_BALANCE_FORK and
 * SD_BALANCE_EXEC.
 *
 * Balance, ie. select the least loaded group.
 *
 * Returns the target CPU number, or the same CPU if no balancing is needed.
 *
 * preempt must be disabled.
 */

static int
select_task_rq_mycfs(struct task_struct *p, int sd_flag, int flags)
{
	return 0;
}

static void pre_schedule_mycfs(struct rq *rq, struct task_struct *prev)
{
	
}

static void post_schedule_mycfs(struct rq *rq)
{
	
}

static void rq_online_mycfs(struct rq *rq)
{

}

static void rq_offline_mycfs(struct rq *rq)
{

}

static void task_waking_mycfs(struct task_struct *p)
{
}
#endif

void init_mycfs_rq(struct mycfs_rq *mycfs_rq){
    memset(selist,0,sizeof(selist));
}

/*
 * Simple, special scheduling class for the per-CPU mycfs tasks:
 */
const struct sched_class mycfs_sched_class = {
	.next			= &idle_sched_class,
	.enqueue_task		= enqueue_task_mycfs,
	.dequeue_task		= dequeue_task_mycfs,
	.yield_task		= yield_task_mycfs,
	.yield_to_task		= yield_to_task_mycfs,

	.check_preempt_curr	= check_preempt_wakeup,

	.pick_next_task		= pick_next_task_mycfs,
	.put_prev_task		= put_prev_task_mycfs,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_mycfs,
	.rq_online		= rq_online_mycfs,
	.rq_offline		= rq_offline_mycfs,

	.task_waking		= task_waking_mycfs,
#endif

	.set_curr_task          = set_curr_task_mycfs,
	.task_tick		= task_tick_mycfs,
	.task_fork		= task_fork_mycfs,

	.prio_changed		= prio_changed_mycfs,
	.switched_from		= switched_from_mycfs,
	.switched_to		= switched_to_mycfs,

	.get_rr_interval	= get_rr_interval_mycfs,

#ifdef CONFIG_FAIR_GROUP_SCHED
	.task_move_group	= task_move_group_mycfs,
#endif
};

