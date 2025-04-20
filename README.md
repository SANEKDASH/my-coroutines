# My Coroutines
Here in this repo you can see my implementation of coroutines in C.

# Ideas behind implementation

The functionality of coroutines was implemented
according to the next set of ideas:
- We want to map a routine on a specific CPU core
to avoid overhead associated with switching task from one core to another.
- We DON'T want our routine to be suspended by scheduler to avoid context switches between tasks during
the routine execution.
- We want default routine control interface (start/yield).