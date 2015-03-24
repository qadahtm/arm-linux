#include "sched.h"
#include <linux/syscalls.h>
#include <linux/printk.h>
/* tqadah */
#include <trace/events/sched.h>
#include <linux/sched.h>
#include <linux/myrbtree.h>
/* Yiyang: */
#include "myrbtree.h"
#include <linux/slab.h>
#include <linux/time.h>

//typedef unsigned long long u64;

/*
 * mycfs-task scheduling class.
 *
 * (NOTE: these are not related to SCHED_IDLE tasks which are
 *  handled in sched/fair.c)
 */

static int temp_limit = 0;
static unsigned int penalty = 0;

SYSCALL_DEFINE2(sched_setlimit,pid_t, pid, int, limit){
    printk(KERN_EMERG "for pid(%d) , limit is %d\n",(int) pid, limit);

    //struct task_struct* ts = find_task_by_vpid(pid);

    //if (ts == NULL) {
    //	printk(KERN_EMERG "[WARNING] No task_struct found with pid = %d\n", (int)pid);
    //    return -1;
    //}

    //ts->cpu_limit = limit;
    //ts->penalty = 0;

    temp_limit = limit;

    return 0;
}
//#define CONFIG_SMP

//#define SELIST_SIZE 1000
//static struct sched_entity * selist[SELIST_SIZE];
//static int head=-1;
//static int tail=0;

static red_blk_tree* rb_tree = NULL; //red-black tree for mycfs
static struct timeval tv_start = {};
static struct timeval tv_end;
static s64 t_start = 0;
static s64 t_end = 0;
//do_gettimeofday(&tv_start);
//t_start = timeval_to_ns(&tv_start);


#if BITS_PER_LONG == 32
# define WMULT_CONST	(~0UL)
#else
# define WMULT_CONST	(1UL << 32)
#endif

#define WMULT_SHIFT	32

/*
 * Shift right and round:
 */
#define SRR(x, y) (((x) + (1UL << ((y) - 1))) >> (y))

/*
 * delta *= weight / lw
 */
static unsigned long
calc_delta_mycfs(unsigned long delta_exec, unsigned long weight,
		struct load_weight *lw)
{
	u64 tmp;
	/*
	 * weight can be less than 2^SCHED_LOAD_RESOLUTION for task group sched
	 * entities since MIN_SHARES = 2. Treat weight as 1 if less than
	 * 2^SCHED_LOAD_RESOLUTION.
	 */
	if (likely(weight > (1UL << SCHED_LOAD_RESOLUTION)))
		tmp = (u64)delta_exec * scale_load_down(weight);
	else
		tmp = (u64)delta_exec;

	if (!lw->inv_weight) {
		unsigned long w = scale_load_down(lw->weight);

		if (BITS_PER_LONG > 32 && unlikely(w >= WMULT_CONST))
			lw->inv_weight = 1;
		else if (unlikely(!w))
			lw->inv_weight = WMULT_CONST;
		else
			lw->inv_weight = WMULT_CONST / w;
	}

	/*
	 * Check whether we'd overflow the 64-bit multiplication:
	 */
	if (unlikely(tmp > WMULT_CONST))
		tmp = SRR(SRR(tmp, WMULT_SHIFT/2) * lw->inv_weight,
			WMULT_SHIFT/2);
	else
		tmp = SRR(tmp * lw->inv_weight, WMULT_SHIFT);

	return (unsigned long)min(tmp, (u64)(unsigned long)LONG_MAX);
}

static inline struct task_struct *task_of(struct sched_entity *se)
{
	return container_of(se, struct task_struct, se);
}

//static inline struct rq *rq_of(struct cfs_rq *cfs_rq)
//{
//	return container_of(cfs_rq, struct rq, cfs);
//}

static inline u64 max_vruntime(u64 max_vruntime, u64 vruntime)
{
	s64 delta = (s64)(vruntime - max_vruntime);
	if (delta > 0)
		max_vruntime = vruntime;

	return max_vruntime;
}

static inline u64 min_vruntime(u64 min_vruntime, u64 vruntime)
{
	s64 delta = (s64)(vruntime - min_vruntime);
	if (delta < 0)
		min_vruntime = vruntime;

	return min_vruntime;
}

static inline int entity_before(struct sched_entity *a,
				struct sched_entity *b)
{
	return (s64)(a->vruntime - b->vruntime) < 0;
}

static void update_stats_penalty(struct rq *rq, struct task_struct *p, unsigned int penalty)
{
    // update task statistics
    struct mycfs_rq * mycfs = &(rq->mycfs);
    struct sched_entity *curr_se = &(p->se); 
//    struct sched_entity *curr_se = &(mycfs->curr); 
    struct task_struct *curr = p;//task_of(curr_se);
    u64 now = rq->clock_task;
    unsigned long delta_exec;
    unsigned long delta_exec_weighted;
    //unsigned long delta_exec_weighted;
    u64 vruntime = mycfs->min_vruntime;

    delta_exec = (unsigned long) (now - curr_se->exec_start);

    if (!delta_exec)
        return;
//    printk(KERN_EMERG "update_stats for curr = %s, now = %llu ,curr_se->exec_start = %llu, delta = %lu, vruntime = %llu\n",
//            curr->comm, now, curr_se->exec_start, delta_exec,curr_se->vruntime);
//    curr_se->statistics.exec_max = max((u64) delta_exec, curr_se->statistics.exec_max);
    schedstat_set(curr_se->statistics.exec_max,
            max((u64) delta_exec, curr_se->statistics.exec_max));

    schedstat_add(mycfs, exec_clock, delta_exec);
//    mycfs->exec_clock += delta_exec;
    //delta_exec_weighted = calc_delta_fair(delta_exec, curr);
    curr_se->sum_exec_runtime += delta_exec;
    delta_exec_weighted = calc_delta_mycfs(delta_exec, NICE_0_LOAD, &curr_se->load);
//    printk(KERN_EMERG "update_stats: curr->load (%lu) , delta_exec(%llu), delte_exec_weighted(%llu)\n",
//            curr_se->load.weight,delta_exec,delta_exec_weighted );
    curr_se->vruntime += delta_exec_weighted*penalty;
   

//    if (mycfs->curr){
//        printk(KERN_EMERG "update_stats: mycfs->curr is not zero\n");
//        vruntime = mycfs->curr->vruntime;
//    }
        
    vruntime = curr_se->vruntime;
    /* ensure we never gain time by being placed backwards. */
    mycfs->min_vruntime = max_vruntime(mycfs->min_vruntime, vruntime);

    curr_se->exec_start = now;
    
//    if (entity_is_task(curr)) {
//            struct task_struct *curtask = task_of(curr_se);
    
//            trace_sched_stat_runtime(p, delta_exec, curr_se->vruntime);
            trace_sched_stat_runtime(curr, delta_exec, curr_se->vruntime);
            cpuacct_charge(curr, delta_exec);
//            account_group_exec_runtime(curr, delta_exec);
//    }
//    printk(KERN_EMERG "update_stats for curr = %s, curr_se->sum_exec_time = %llu, vruntime = %llu\n",
//        curr->comm, curr_se->sum_exec_runtime, curr_se->vruntime);
//    printk(KERN_EMERG "update_stats for curr = %s, rq->curr = %s, mycfs->exec_clock = %llu, rq->clock_task = %llu\n",
//            curr->comm, rq->curr->comm, mycfs->exec_clock, rq->clock_task);
}

static void update_stats(struct rq *rq, struct task_struct *p)
{
    // update task statistics
    struct mycfs_rq * mycfs = &(rq->mycfs);
    struct sched_entity *curr_se = &(p->se); 
//    struct sched_entity *curr_se = &(mycfs->curr); 
    struct task_struct *curr = p;//task_of(curr_se);
    u64 now = rq->clock_task;
    unsigned long delta_exec;
    unsigned long delta_exec_weighted;
    //unsigned long delta_exec_weighted;
    u64 vruntime = mycfs->min_vruntime;

    delta_exec = (unsigned long) (now - curr_se->exec_start);

    if (!delta_exec)
        return;
//    printk(KERN_EMERG "update_stats for curr = %s, now = %llu ,curr_se->exec_start = %llu, delta = %lu, vruntime = %llu\n",
//            curr->comm, now, curr_se->exec_start, delta_exec,curr_se->vruntime);
//    curr_se->statistics.exec_max = max((u64) delta_exec, curr_se->statistics.exec_max);
    schedstat_set(curr_se->statistics.exec_max,
            max((u64) delta_exec, curr_se->statistics.exec_max));

    schedstat_add(mycfs, exec_clock, delta_exec);
//    mycfs->exec_clock += delta_exec;
    //delta_exec_weighted = calc_delta_fair(delta_exec, curr);
    curr_se->sum_exec_runtime += delta_exec;
    delta_exec_weighted = calc_delta_mycfs(delta_exec, NICE_0_LOAD, &curr_se->load);
//    printk(KERN_EMERG "update_stats: curr->load (%lu) , delta_exec(%llu), delte_exec_weighted(%llu)\n",
//            curr_se->load.weight,delta_exec,delta_exec_weighted );
    curr_se->vruntime += delta_exec_weighted;
   

//    if (mycfs->curr){
//        printk(KERN_EMERG "update_stats: mycfs->curr is not zero\n");
//        vruntime = mycfs->curr->vruntime;
//    }
        
    vruntime = curr_se->vruntime;
    /* ensure we never gain time by being placed backwards. */
    mycfs->min_vruntime = max_vruntime(mycfs->min_vruntime, vruntime);

    curr_se->exec_start = now;
    
//    if (entity_is_task(curr)) {
//            struct task_struct *curtask = task_of(curr_se);
    
//            trace_sched_stat_runtime(p, delta_exec, curr_se->vruntime);
            trace_sched_stat_runtime(curr, delta_exec, curr_se->vruntime);
            cpuacct_charge(curr, delta_exec);
//            account_group_exec_runtime(curr, delta_exec);
//    }
//    printk(KERN_EMERG "update_stats for curr = %s, curr_se->sum_exec_time = %llu, vruntime = %llu\n",
//        curr->comm, curr_se->sum_exec_runtime, curr_se->vruntime);
//    printk(KERN_EMERG "update_stats for curr = %s, rq->curr = %s, mycfs->exec_clock = %llu, rq->clock_task = %llu\n",
//            curr->comm, rq->curr->comm, mycfs->exec_clock, rq->clock_task);
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
    
    update_stats(rq,p);
    update_stats(rq,curr);
    
    //resched_task_mycfs(curr);
}

static struct task_struct *pick_next_task_mycfs(struct rq *rq)
{
    struct task_struct *p; 
    struct mycfs_rq *mycfs;
    struct sched_entity * se;
    red_blk_node * rb_node;
#if 0 // Yiyang: comment out	
	
        int i=0;

//        if (rq->curr == p) {
//            printk(KERN_EMERG "picked task : %s at i = %d, pid= %d is already in rq of cpu(%d)\n"
//                    ,p->comm, i, (int) p->pid,cpu_of(rq));            
//            return NULL;
//        }
        
        if (tail >= 0 && tail < SELIST_SIZE){
        
            if (head == -1){
                // first time pick the first task in the list
                
                for (i=0; i< SELIST_SIZE; i++){
                    // returning the first task in the list
                    if (selist[i] != 0) {
                        head = i;
                        p = task_of(selist[i]);
//                        printk(KERN_EMERG "picked task (first) : %s at i = %d, pid= %d, cpu = %d, head = %d \n"
//                            ,p->comm, i, (int) p->pid, cpu_of(rq),head);
                        return p;
                    }            
                }
                
            }
            else{
                // pick other tasks. 
                for (i=(head+1); i< SELIST_SIZE; i++){
                    // returning the first task in the list after head
                    if (selist[i] != 0) {
                        head = i;
                        p = task_of(selist[i]);
//                        printk(KERN_EMERG "picked task (head-start-other) : %s at i = %d, pid= %d, cpu = %d, head = %d \n"
//                            ,p->comm, i, (int) p->pid, cpu_of(rq),head);
                        return p;
                    }            
                }
                
                for (i=0; i< head; i++){
                    // returning the first task in the list after head
                    if (selist[i] != 0) {
                        head = i;
                        p = task_of(selist[i]);
//                        printk(KERN_EMERG "picked task (head-start-other) : %s at i = %d, pid= %d, cpu = %d, head = %d \n"
//                            ,p->comm, i, (int) p->pid, cpu_of(rq),head);
                        return p;
                    }            
                }
                
                
                // no other tasks, return same task if valid
                if (selist[head] != 0){
                    p = task_of(selist[head]);
//                    printk(KERN_EMERG "picked task (no-other) : %s at i = %d, pid= %d, cpu = %d, head = %d \n"
//                            ,p->comm, i, (int) p->pid, cpu_of(rq),head);
                    return p;
                }
                
                
            }
            return NULL;
        
        }
        else{
//            printk(KERN_EMERG "tail at(%d) is invalid\n");
            BUG();
        }
#endif        
//        printk(KERN_EMERG "picking next task for mycfs, check rb_tree is empty\n");
        if (red_blk_is_empty(rb_tree)) return NULL;
     
//        printk(KERN_EMERG "rb_tree is NOT empty\n");
//        red_blk_inorder_tree_print(rb_tree,rb_tree->root->left_child);
//        printk(KERN_EMERG "return NULL anyways\n");

	if (penalty > 0) {
		penalty--;
		return NULL;
	}

	rb_node = red_blk_find_leftmost(rb_tree);
	se = ((struct sched_entity *)(rb_node->key));
	p = task_of(se);

        //update_stats(rq,p);
        
        // set start exec timestamp
        se->exec_start = rq->clock_task;
        rq->mycfs.curr = se;
              
        if(se->on_rq){
            red_blk_delete_node(rb_tree,rb_node);
            se->myrb_node = NULL;
//            	schedstat_set(se->statistics.wait_max, max(se->statistics.wait_max,
//			rq->clock - se->statistics.wait_start));
//                schedstat_set(se->statistics.wait_count, se->statistics.wait_count + 1);
//                schedstat_set(se->statistics.wait_sum, se->statistics.wait_sum +
//                                rq->clock - se->statistics.wait_start);
#ifdef CONFIG_SCHEDSTATS
//                if (entity_is_task(se)) {
                        trace_sched_stat_wait(task_of(se),
                                rq->clock - se->statistics.wait_start);
//                }
#endif
                schedstat_set(se->statistics.wait_start, 0);
        }
        
        
        
        return p;
//        return NULL;
}

static void
enqueue_task_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
    struct sched_entity * curr_se = &(p->se);
//    printk(KERN_EMERG "enqueue task %s, \n",p->comm);
//    printk(KERN_EMERG "enqueue task = %s, now = %llu ,curr_se->exec_start = %llu, mycfs->exec_clock = %llu, vruntime = %llu\n",
//            p->comm, rq->clock_task, curr_se->exec_start, rq->mycfs.exec_clock,curr_se->vruntime);
    
    //selist[tail] = &p->se;
    //tail++;
    
    if (rq->curr != p){
         schedstat_set(se->statistics.wait_start, rq->clock);
    }
    
    update_stats(rq,rq->curr);
    update_stats(rq,p);
    
    p->se.myrb_node = red_blk_insert(&(p->se), rb_tree);
    inc_nr_running(rq);
}

/*
 * It is not legal to sleep in the mycfs task - print a warning
 * message if some code attempts to do it:
 */
static void
dequeue_task_mycfs(struct rq *rq, struct task_struct *p, int flags)
{
#if 0 // Yiyang: comment out
    int i =0;
    update_stats(rq,p);
//    printk(KERN_EMERG "dequeue task %s\n",p->comm);

    for (i=0; i < tail; i++){
        if (selist[i] == &p->se) break;
    }
    
    if (i < tail){
        //found
//        printk(KERN_EMERG "task %s found in running-queue at i=%d\n",p->comm,i);
        for (; i < tail; i++){
            selist[i] = selist[i+1];
        }
        selist[i] = 0;
        tail--;
//        printk(KERN_EMERG "tail was (%d), now is (%d)\n",(tail+1),tail);
        
    }
//    else{
//        printk(KERN_EMERG "task %s does not exist in running-queue\n",p->comm);
//    }
#endif
    struct sched_entity * se = &(p->se);
    red_blk_node* to_dequeue = NULL;
//    printk(KERN_EMERG "dequeue task %s\n",p->comm);
    update_stats(rq, p);
    if (rq->curr != p){
    
          
//    schedstat_set(se->statistics.wait_max, max(se->statistics.wait_max,
//                     rq->clock - se->statistics.wait_start));
//     schedstat_set(se->statistics.wait_count, se->statistics.wait_count + 1);
//     schedstat_set(se->statistics.wait_sum, se->statistics.wait_sum +
//                     rq->clock - se->statistics.wait_start);
#ifdef CONFIG_SCHEDSTATS
//     if (entity_is_task(se)) {
             trace_sched_stat_wait(p,
                     rq->clock - se->statistics.wait_start);
//     }
#endif
     schedstat_set(se->statistics.wait_start, 0);
    
    }
    se->on_rq = 0;
	//to_dequeue = red_blk_search(&(p->se), rb_tree);
        to_dequeue = p->se.myrb_node;
	#if 0 // Yiyang: test
	if (to_dequeue == NULL) {
		printk(KERN_EMERG "[ERROR] This shouldn't happen!\n");
		BUG();
	}
	if (to_dequeue == rb_tree->nil) {
		printk(KERN_EMERG "[WARNING] The data is not in the tree!\n");
	}
	#endif

        if (to_dequeue != NULL){
            red_blk_delete_node(rb_tree, to_dequeue);
            p->se.myrb_node = NULL;
        }	
    dec_nr_running(rq);
}

static void put_prev_task_mycfs(struct rq *rq, struct task_struct *prev)
{
   // update_stats(rq,prev);
}


static void task_tick_mycfs(struct rq *rq, struct task_struct *curr, int queued)
{
    #if 0 // Yiyang: test task tick interval
    do_gettimeofday(&tv_end);
    t_end = timeval_to_ns(&tv_end);
    printk(KERN_EMERG "[Time elapsed] = %lld\n", t_end - t_start);
    do_gettimeofday(&tv_start);
    t_start = timeval_to_ns(&tv_start);
    #endif

    red_blk_node* lm_rbn = red_blk_find_leftmost (rb_tree);
    struct sched_entity * lm_se = (struct sched_entity *) lm_rbn->key;
    struct task_struct * lm_task = task_of(lm_se);
    
    unsigned long delta_exec;
    u64 now = rq->clock_task;
    struct sched_entity *curr_se = &(curr->se); 
    delta_exec = (unsigned long) (now - curr_se->exec_start);
    //printk(KERN_EMERG "[Last runtime on CPU] = %lu\n", delta_exec);
    
    if (delta_exec > 10000000) {
    	penalty = 100 / temp_limit;
    }

    //update_stats(rq,curr);
    update_stats_penalty(rq, curr, penalty);
    
    if (lm_se == &(curr->se)){
        red_blk_delete_node(rb_tree,lm_rbn);
        red_blk_insert (lm_se, rb_tree);
    }
//    else{
//        
//        printk(KERN_EMERG "[BUG] task_tick: we are not running leftmost curr(%s , vrt = %llu) leftmost(%s, vrt= %llu), need to reschedule\n", curr->comm, curr->se.vruntime,lm_task->comm,lm_se->vruntime);
        //BUG();
//        resched_task_mycfs(curr);
//    }
    
    // see if the leftmost has been updates
    lm_rbn = red_blk_find_leftmost (rb_tree);
    lm_se = (struct sched_entity *) lm_rbn->key;
    
    if (lm_se != &(curr->se)){
        // another task became leftmost, we need to reschedule
//        printk(KERN_EMERG "[INFO] task_tick:another task became leftmost, we need to reschedule\n");
        resched_task_mycfs(curr);
    }
    
//    if (rq->curr == curr){
//        printk(KERN_EMERG "[INFO] task_tick: before update - curr=rq->curr, task(%s), vruntime(%llu)\n",curr->comm,curr->se.vruntime);
//        update_stats(rq,curr);
////        printk(KERN_EMERG "[INFO] task_tick: after - curr=rq->curr, task(%s), vruntime(%llu)\n",curr->comm,curr->se.vruntime);
//    }
//    else{
//        printk(KERN_EMERG "[WARNING] task_tick: curr!!!=rq->curr\n");
//        update_stats(rq,curr);
//    }

    
    
    // preempt tasks on task ticks
//    if (rq->curr == curr)
//        resched_task_mycfs(curr);
//    else resched_task_mycfs(rq->curr);
}

static void set_curr_task_mycfs(struct rq *rq)
{  
}

static void switched_to_mycfs(struct rq *rq, struct task_struct *p)
{
	if (!p->se.on_rq)
		return;
	if (rq->curr == p){
//            printk(KERN_EMERG "handling task %s , reschedule task\n",p->comm);
            resched_task(rq->curr);
        }
	else
        {
//            printk(KERN_EMERG "going to check preempt task %s\n",p->comm);
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
    struct task_struct *p = (rq->curr);
    printk(KERN_EMERG "yielding task %s \n",p->comm);
    update_stats(rq, p);
    dequeue_task_mycfs(rq,p,0);
    enqueue_task_mycfs(rq,p,0);    
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

/* U64Destroy, U64Comp, U64Print are function pointers needed by
 * creating red black tree */
void U64Destroy(void* a) {
	kfree((unsigned long long*) a);
}

int U64Comp(const void* a, const void* b) {
	if( *(unsigned long long*)a > *(unsigned long long*)b) return(1);
    if( *(unsigned long long*)a < *(unsigned long long*)b) return(-1);
	return(0);
}

int vruntimeCompare(const void* a, const void* b) {
	if ( ((struct sched_entity*)(a))->vruntime > ((struct sched_entity*)(b))->vruntime )
		return 1;
	if ( ((struct sched_entity*)(a))->vruntime > ((struct sched_entity*)(b))->vruntime )
		return -1;
	return 0;
}

void U64Print(const void* a) {
	printk("%llu",*(unsigned long long*)a);
}


void init_mycfs_rq(struct mycfs_rq *mycfs_rq){
    //rb_tree = red_blk_create_tree(U64Comp, U64Destroy, U64Print);
    rb_tree = red_blk_create_tree(vruntimeCompare, U64Destroy, U64Print);

	#if 1 // Yiyang: test
	printk(KERN_EMERG "============ red_blk_create_tree() is called\n");
	#endif
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
//        .check_preempt_curr	= check_preempt_curr_mycfs,

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
