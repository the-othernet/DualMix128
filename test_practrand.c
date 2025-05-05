#include <stdio.h>
#include <stdint.h>
#include <unistd.h> // For write or use fwrite
#include <time.h>   // For clock_gettime

const uint64_t GR = 0x9e3779b97f4a7c15L;

uint64_t state0;
uint64_t state1;

uint64_t rotateLeft(unsigned long long x, int k) { return (x << k) | (x >> (64 - k)); }


// Function that implements the new algorithm and returns raw 64-bit state

inline uint64_t dualMix128() {
    uint64_t mix = state0 + state1;
    state0 = mix + rotateLeft(state0, 16);
    state1 = mix + rotateLeft(state1, 2);

    return GR * mix;
}



// SplitMix64: used for seeding
// Takes a pointer to its state, updates it, and returns a generated value.
uint64_t splitmix64_next(uint64_t *sm_state) {
    uint64_t z = (*sm_state += 0x9e3779b97f4a7c15ULL); // Use ULL suffix
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}




int main(void) {

   struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("clock_gettime failed");
        return 1; // Exit if cannot get time
    }

    // Combine seconds and nanoseconds into a single 64-bit seed value
    uint64_t time_seed = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;

    state0 = splitmix64_next(&time_seed);
    state1 = splitmix64_next(&time_seed);

  uint64_t raw_value;
  // Loop infinitely, generating and writing raw 64-bit values
  for (;;) {
    raw_value = dualMix128();
    // Write the binary representation of the 64-bit value to stdout
    if (fwrite(&raw_value, sizeof(raw_value), 1, stdout) != 1) {
      // Error writing to stdout (e.g., pipe broken), exit gracefully.
      perror("fwrite to stdout failed");
      return 1;
    }
  }
  return 0; // Should never reach here
}
