#include <stdio.h>
#include <time.h>
#include <signal.h>

timer_t firstTimerID;
timer_t secondTimerID;
timer_t thirdTimerID;

static void timerHandler(int sig, siginfo_t *si, void *uc) {
    timer_t *tidp;
    tidp = si->si_value.sival_ptr;

    if ( *tidp == firstTimerID )
        puts("1st\n");
    else if ( *tidp == secondTimerID )
        puts("2nd\n");
    else if ( *tidp == thirdTimerID )
        puts("3rd\n");
}

static int makeTimer(timer_t *timerID, int expire) {
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
        return(-1);
    }

    /* Set and enable alarm */
    te.sigev_notify = SIGEV_SIGNAL; // SIGEV_SIGNAL = upon timer expiration, generate the signal sigev_signo
    te.sigev_signo = sigNo;
    te.sigev_value.sival_ptr = timerID;
    timer_create(CLOCK_REALTIME, &te, timerID);

    // specify the new initial value and the new interval for the timer
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = expire;
    its.it_value.tv_nsec = 0;//expireMS * 1000000;
    timer_settime(*timerID, 0, &its, NULL);

    return(0);
}

/* int main() {
    makeTimer(&firstTimerID, 2);
    makeTimer(&secondTimerID, 5);
    while(1);
} */