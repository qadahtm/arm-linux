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

//EXPORT_SYMBOL(sys_sched_setlimit);
/*
 * Idle tasks are unconditionally rescheduled:
 */
static void check_preempt_curr_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
//	resched_task(rq->mycfs);
    //resched_task(rq->idle);
    
    idle_sched_class.check_preempt_curr(rq,p,flags);
}

static struct task_struct *pick_next_task_mycfs(struct rq *rq)
{
	struct task_struct *p;  
        
        
//        if (strncmp(p->comm,"p3tmycfs", TASK_COMM_LEN) == 0){
        //printk(KERN_EMERG "delegate picking to to idle %s\n",p->comm);
        
//        }
        //p = NULL;
	//return p;
        //return rq->idle;
        return idle_sched_class.pick_next_task(rq);
}

static void
enqueue_task_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
//    if (strncmp(p->comm,"p3tmycfs", TASK_COMM_LEN) == 0){
        printk(KERN_EMERG "enqueue task %s\n",p->comm);  
//    }
    inc_nr_running(rq);
}

/*
 * It is not legal to sleep in the mycfs task - print a warning
 * message if some code attempts to do it:
 */
static void
dequeue_task_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
//    if (strncmp(p->comm,"p3tmycfs", TASK_COMM_LEN) == 0){
        printk(KERN_EMERG "dequeue task %s\n",p->comm);
//    }
    dec_nr_running(rq);
}

static void put_prev_task_mycfs(struct rq *rq, struct task_struct *prev)
{
}

static void task_tick_mycfs(struct rq *rq, struct task_struct *curr, int queued)
{
}

static void set_curr_task_mycfs(struct rq *rq)
{
}

static void switched_to_mycfs(struct rq *rq, struct task_struct *p)
{
	BUG();
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
{}

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

