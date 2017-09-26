#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>

char *msg;
char *route_exists(char *dest_addr);

// get dst_addr associated with tid on timers hash table
// using dst_addr, get list of interested hosts from pnrs
// notify each host in the list that a timeout has occurred (retry?)
void timeout_handler(int sig_num, siginfo_t *si, void *uc) {
    int *timer_id = si->si_value.sival_ptr;
    char timer_id_str[12];
    char *dst_addr;

    sprintf(timer_id_str, "%d", *timer_id);
    dst_addr = (char *)cfuhash_get(timers, timer_id_str);

    //printf("*** Timeout on NREQ for host %s (timer ID: %s)\n", dst_addr, timer_id_str);
    asprintf(&msg, "[TIMEOUT] Timeout on NREQ for host %s (timer ID: %s)\n", dst_addr, timer_id_str);
    log_msg(msg, own_addr);

    // remove entries in the Timers table
    //print_hash_table(timers);
    //print_hash_table(pnrs);
    cfuhash_delete(timers, timer_id_str);
    timer_ids[*timer_id - 1] = 0;
    //cfuhash_delete(pnrs, dst_addr);
    //print_hash_table(timers);
    //print_hash_table(pnrs);

    /*
    NREP already received or route has been lost
    or
    NREQ has already been acked
    */
    if (!cfuhash_get(pnrs, dst_addr)) { // || exists_in_list(acks, dst_addr) == 1) {
        cfuhash_delete(pnrs, dst_addr);
        asprintf(&msg, "[TIMER-] Not reseting timer for %s\n", dst_addr);
        log_msg(msg, own_addr);
        return;
    }

    char *next_hop;
    asprintf(&msg, "[ TIMEOUT ] %s\n", "1");
    log_msg(msg, own_addr);

    if ((next_hop = route_exists(strdup(dst_addr))) != NULL) {
        asprintf(&msg, "[ TIMEOUT ] %s\n", "5");
        log_msg(msg, own_addr);
        // create new timer
        timer_t timer_obj;
        int *timer_id_index = next_free_timer_id();
        char timer_id_str[12];
        //printf("~~~new timer_id=%d\n", *timer_id_index);
        make_timer(&timer_obj, &(timer_ids[*timer_id_index - 1]), NREQ_TIMEOUT_SECS);
        // associate timer_id (as string) to dst_addr in timers table
        sprintf(timer_id_str, "%d", *timer_id_index);
        cfuhash_put(timers, timer_id_str, strdup(dst_addr));

        //printf("*** Started timer %d for NREQ of %s\n", *timer_id_index, dst_addr);
        asprintf(&msg, "[TIMER++] Reset timer %d for NREQ of %s\n", *timer_id_index, dst_addr);
        log_msg(msg, own_addr);

        request_name(dst_addr, next_hop);
    }
}

// if route to dst_addr exists, returns the next_hop
char *route_exists(char *dst_addr) {
    FILE *in;
    asprintf(&msg, "[ TIMEOUT ] %s\n", "2");
    log_msg(msg, own_addr);
    while (!(in = popen("route -n | tr -s ' '", "r"))) {
        asprintf(&msg, "[WAIT] %s\n", "Trying popen(route -n | tr -s ' ')...");
        log_msg(msg, own_addr);
    }
    char buf[2*ADDR_MAX_STRLEN + 1];
    char buf_copy[sizeof buf];
    char *route_dst_addr, *route_next_hop;
    asprintf(&msg, "[ TIMEOUT ] %s\n", "3");
    log_msg(msg, own_addr);
    while (fgets(buf, sizeof(buf), in)!=NULL) {
        strcpy(buf_copy, buf);

        route_dst_addr = strtok(buf_copy, " ");

        asprintf(&msg, "[ TIMEOUT ] 3.1 dst:%s\n", route_dst_addr);
        log_msg(msg, own_addr);

        if (strcmp(route_dst_addr, dst_addr) == 0) {
            route_next_hop = strdup(strtok(NULL, " "));
            asprintf(&msg, "[ TIMEOUT ] 3.1 dst:%s\n", route_dst_addr);
            log_msg(msg, own_addr);
            pclose(in);
            return route_next_hop;
        }
        //buf_copy[strlen(buf_copy) - 1] = '\0'; // remove trailing newline        
    }
    asprintf(&msg, "[ TIMEOUT ] %s\n", "4");
    log_msg(msg, own_addr);
    pclose(in);
    return NULL;
}

void make_timer(timer_t *timer_obj, int *timer_id, int timeout_secs) {
    //printf("~~~makeTimer: timer_id=%d\n", *timer_id);
    struct sigevent te;
    struct itimerspec its;
    struct sigaction sa;
    int sig_num = SIGRTMIN;

    // set the function to execute when a SA_SIGINFO signal is detected
    sa.sa_flags = SA_SIGINFO; // use sa_sigaction (instead of sa_handler) as the signal handler
    sa.sa_sigaction = timeout_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sig_num, &sa, NULL) == -1) {
        asprintf(&msg, "[ ERROR ] %s\n", "Failed to setup signal handling");
        log_msg(msg, own_addr);
        perror("Failed to setup signal handling");
        return;
    }

    // compose the sigevent structure that specifies how the calling
    // process should be notified when the timer expires
    te.sigev_notify = SIGEV_SIGNAL; // SIGEV_SIGNAL = upon timer expiration, generate the signal in sigev_signo
    te.sigev_signo = sig_num;
    te.sigev_value.sival_ptr = timer_id;
    //te._sigev_un._tid = syscall(SYS_gettid);

    // specify the expiration time
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = timeout_secs;
    its.it_value.tv_nsec = 0;//timeoutMS * 1000000;

    // create and start the timer
    timer_create(CLOCK_REALTIME, &te, timer_obj);
    timer_settime(*timer_obj, 0, &its, NULL);
}
