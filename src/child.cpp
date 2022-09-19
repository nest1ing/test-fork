
#include <sdeventplus/clock.hpp>
#include <sdeventplus/event.hpp>
#include <sdeventplus/source/signal.hpp>
#include <sdeventplus/source/time.hpp>

#include <chrono>
#include <csignal>
#include <cstdio>

constexpr auto clockId = sdeventplus::ClockId::Monotonic;
using Clock = sdeventplus::Clock<clockId>;
using Time = sdeventplus::source::Time<clockId>;

template <class R, class P>
inline void startTimer(Time& timer, const std::chrono::duration<R, P>& delay)
{
    timer.set_time(Clock(timer.get_event()).now() + delay);
    timer.set_enabled(sdeventplus::source::Enabled::OneShot);
}

static void signalHandler(sdeventplus::source::Signal& source,
                          const struct signalfd_siginfo*)
{
    printf("\rCHILD: Signal %d received, terminating...\n",
           source.get_signal());
    source.get_event().exit(EXIT_SUCCESS);
}

int main()
{
    printf("CHILD: Start process\n");
    auto event = sdeventplus::Event::get_default();

    size_t count = 0;
    Time timer(event, {}, std::chrono::microseconds{1},
               [&count](Time& source, Time::TimePoint) {
                   printf("CHILD: Timer alarmed %zu-th time\n", ++count);
                   if (count < 10)
                   {
                       startTimer(source, std::chrono::seconds{2});
                   }
                   else
                   {
                       source.get_event().exit(EXIT_SUCCESS);
                   }
               });

    sigset_t ss;

    if (sigemptyset(&ss) < 0 || sigaddset(&ss, SIGTERM) < 0 ||
        sigaddset(&ss, SIGINT) < 0)
    {
        fprintf(stderr, "CHILD ERROR: Failed to setup signal handlers, %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    if (sigprocmask(SIG_BLOCK, &ss, nullptr) < 0)
    {
        fprintf(stderr, "CHILD ERROR: Faile to block signals, %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    sdeventplus::source::Signal sigterm(event, SIGTERM, signalHandler);
    sdeventplus::source::Signal sigint(event, SIGINT, signalHandler);

    auto rc = event.loop();
    printf("CHILD: Finished, rc=%d\n", rc);
    return rc;
}
