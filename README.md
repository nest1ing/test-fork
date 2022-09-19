# test-fork

This project demonstrates how to use sd-event for watching the child process.

## build
```
meson build
meson compile -Cbuild
```

## test-fork-parent
This is a main entry point. It starts the child process, then waits its
completion. It also reports the received signals.

## test-fork-child
This is a dummy child process that just waits several seconds and finish.
It also reports the received signals.
