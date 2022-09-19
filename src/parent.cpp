
#include <sdeventplus/event.hpp>
#include <sdeventplus/source/child.hpp>
#include <sdeventplus/source/signal.hpp>

#include <csignal>
#include <cstdio>

static void signalHandler(sdeventplus::source::Signal& source,
                          const struct signalfd_siginfo*)
{
    printf("\rPARENT: Signal %d received, terminating...\n",
           source.get_signal());
    source.get_event().exit(EXIT_SUCCESS);
}

static void handleChild(sdeventplus::source::Child& source, const siginfo_t* si)
{
    bool terminate = true;
    switch (si->si_code)
    {
        case CLD_EXITED:
            if (si->si_status == EXIT_SUCCESS)
            {
                printf("PARENT: child process %d successful finished.\n",
                       si->si_pid);
            }
            else
            {
                printf("PARENT: child process %d finished with code %d.\n",
                       si->si_pid, si->si_status);
            }
            break;

        case CLD_KILLED:
            printf("PARENT: child process %d killed by signal %d.\n",
                   si->si_pid, si->si_status);
            break;

        case CLD_DUMPED:
            printf("PARENT: child process %d killed by signal %d "
                   "and dumped core\n",
                   si->si_pid, si->si_status);
            break;

        case CLD_STOPPED:
            printf("PARENT: child process %d stopped, status=%d\n", si->si_pid,
                   si->si_status);
            terminate = false;
            break;

        case CLD_CONTINUED:
            printf("PARENT: child process %d continued, status=%d\n",
                   si->si_pid, si->si_status);
            terminate = false;
            break;

        default:
            printf("PARENT: child process %d terminated by unexpected signal. "
                   "{signo=%d, code=%d, status=%d}\n",
                   si->si_pid, si->si_signo, si->si_code, si->si_status);
            break;
    }

    if (terminate)
    {
        source.get_event().exit(EXIT_SUCCESS);
    }
    else
    {
        source.set_enabled(sdeventplus::source::Enabled::OneShot);
    }
}

int main()
{
    printf("PARENT: Start process\n");
    auto event = sdeventplus::Event::get_default();

    sigset_t ss;

    if (sigemptyset(&ss) < 0 || sigaddset(&ss, SIGTERM) < 0 ||
        sigaddset(&ss, SIGINT) < 0 || sigaddset(&ss, SIGCHLD) < 0)
    {
        fprintf(stderr, "PARENT ERROR: Failed to setup signal handlers, %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    if (sigprocmask(SIG_BLOCK, &ss, nullptr) < 0)
    {
        fprintf(stderr, "PARENT ERROR: Faile to block signals, %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    sdeventplus::source::Signal sigterm(event, SIGTERM, signalHandler);
    sdeventplus::source::Signal sigint(event, SIGINT, signalHandler);

    pid_t pid = fork();
    if (pid == 0)
    {
        execl("./build/test-fork-child", "test-child", nullptr);
        fprintf(stderr, "CHLD ERROR: execl failed, %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    else if (pid < 0)
    {
        fprintf(stderr, "PARENT ERROR: fork failed, %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf("PARENT: Child process %d started\n", pid);
    int options = WEXITED | WSTOPPED | WCONTINUED;
    sdeventplus::source::Child child(event, pid, options, handleChild);

    auto rc = event.loop();
    printf("PARENT: Finished, rc=%d\n", rc);
    return rc;
}
