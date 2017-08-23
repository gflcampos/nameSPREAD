#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>

char *msg;

// get dst_addr associated with tid on timers has table
// using dst_addr, get list of interested hosts from pnrs
// notify each host in the list that a timeout has occurred (retry?)
void timeout_handler(int sig_num, siginfo_t *si, void *uc) {
    int *timer_id = si->si_value.sival_ptr;
    char timer_id_str[12];
    char *dst_addr;

    sprintf(timer_id_str, "%d", *timer_id);
    dst_addr = (char *)cfuhash_get(timers, timer_id_str);

    printf("*** Timeout on NREQ for host %s (timer ID: %s)\n", dst_addr, timer_id_str);    
    asprintf(&msg, "*** Timeout on NREQ for host %s (timer ID: %s)\n", dst_addr, timer_id_str);
    log_msg(msg, own_addr);

    // TODO (improvement): notify requesters of timeout so that 
    // they don't have to wait until THEIR timer expires

    // remove entries in the Timers and PNRs tables
    
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

/* int main() {
    makeTimer(&firstTimerID, 2);
    makeTimer(&secondTimerID, 5);
    while(1);
} */