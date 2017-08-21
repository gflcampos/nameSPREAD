#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/syscall.h>
/* timer_t firstTimerID;
timer_t secondTimerID;
timer_t thirdTimerID; */

// get dst_addr associated with tid on timers has table
// using dst_addr, get list of interested hosts from pnrs
// notify each host in the list that a timeout has occurred (retry?)
void timerHandler(int sig, siginfo_t *si, void *uc) {
    timer_t *tidp;
    tidp = si->si_value.sival_ptr;
    char * test = (char *)&(si->si_value.sival_ptr);
    printf("~~~%s_\n", test);
    printf("Key %s. Value: %s\n", cfuhash_exists(timers, test) ? "exists" : "does NOT exist!!!", (char *)cfuhash_get(timers, *tidp));
    
    /* if ( *tidp == firstTimerID ) {
        printf("2_%s\n", cfuhash_exists(timers, *tidp) ? "exists" : "does NOT exist!!!");
        //char *dst_addr = cfuhash_get(timers, *tidp);
        //printf("1st_%d_%s\n", tid, dst_addr);
        
    }
    else if ( *tidp == secondTimerID )
        puts("2nd\n");
    else if ( *tidp == thirdTimerID )
        puts("3rd\n"); */
}

char *makeTimer(timer_t *timerID, int expire) {
    struct sigevent te;
    struct itimerspec its;
    struct sigaction sa;
    int sigNo = SIGRTMIN;

    /* Set up signal handler. */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timerHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sigNo, &sa, NULL) == -1) {
        fprintf(stderr, "Failed to setup signal handling.\n");
        return NULL;
    }

    /* Set and enable alarm */
    te.sigev_notify = SIGEV_SIGNAL; // SIGEV_SIGNAL = upon timer expiration, generate the signal sigev_signo
    te.sigev_signo = sigNo;
    printf("~~~%s_\n", (char *)&timerID);
    te.sigev_value.sival_ptr = timerID;
    //te._sigev_un._tid = syscall(SYS_gettid);
    timer_create(CLOCK_REALTIME, &te, timerID);

    // specify the new initial value and the new interval for the timer
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = expire;
    its.it_value.tv_nsec = 0;//expireMS * 1000000;
    timer_settime(*timerID, 0, &its, NULL);

    return(strdup((char *)&timerID));
}

/* int main() {
    makeTimer(&firstTimerID, 2);
    makeTimer(&secondTimerID, 5);
    while(1);
} */