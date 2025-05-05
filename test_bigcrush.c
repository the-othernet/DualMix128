#include "/usr/include/testu01/unif01.h"
#include "/usr/include/testu01/bbattery.h"
#include <stdint.h>
#include <stdio.h> // For printf, perror
#include <time.h>  // For clock_gettime

// Golden ratio constant for DualMix128
const uint64_t GR = 0x9e3779b97f4a7c15ULL;

// --- State Variables (Static globals for the generator) ---
uint64_t state0;
uint64_t state1;

// --- Helper Functions ---

// Rotate left function (parameter type changed to uint64_t for consistency)
uint64_t rotateLeft(uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

// SplitMix64: used for seeding DualMix128 state
// Takes a pointer to its state, updates it, and returns a generated value.
uint64_t splitmix64_next(uint64_t *sm_state) {
    uint64_t z = (*sm_state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

// --- PRNG Implementation ---

// DualMix128 generator function - returns raw 64-bit output
static double dualMix128() {
    uint64_t mix = state0 + state1;
    // Use consistent uint64_t type for rotateLeft argument
    state0 = mix + rotateLeft(state0, 16);
    state1 = mix + rotateLeft(state1, 2);

    return ( (GR * mix) >> 11) * (1.0/9007199254740992.0);
}

// --- TestU01 Interface ---

// Main function to run TestU01 BigCrush battery
int main(void) {
    // Declare a TestU01 generator structure pointer
    unif01_Gen *gen;
    struct timespec ts;
    uint64_t time_seed;

    // --- Seed Initialization using SplitMix64 from System Time ---
    printf("Seeding DualMix128 using SplitMix64 from clock_gettime...\n");

    // Get high-resolution time
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("clock_gettime failed");
        return 1; // Exit if cannot get time
    }

    // Combine seconds and nanoseconds into a single 64-bit seed value for SplitMix64
    // Note: Potential overflow if ts.tv_sec is very large, but unlikely in practice.
    time_seed = (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;

    // Initialize DualMix128 state using SplitMix64
    state0 = splitmix64_next(&time_seed);
    state1 = splitmix64_next(&time_seed);

    // Basic check: Ensure seeds are not both zero (highly unlikely with time seeding).
    if (state0 == 0 && state1 == 0) {
         fprintf(stderr, "Warning: Both initial states are zero after seeding! Check seeding logic or time source.\n");
         // Adding a minimal fallback to prevent definite failure, though the root cause should be investigated.
         state0 = 0xDEADBEEF12345678ULL;
         state1 = 0xABCDEF0123456789ULL;
    }

    // Optional: Print seeds for verification/debugging
    // printf("Initial state0: 0x%016lx\n", state0);
    // printf("Initial state1: 0x%016lx\n", state1);
    printf("Seeding complete.\n");
    // --- End of Seeding ---

    // Create a TestU01 generator object from the external C function dualMix128
    // Update generator name to reflect seeding method
    printf("Creating TestU01 generator object...\n");
    gen = unif01_CreateExternGen01("DualMix128 (time seeded)", dualMix128);
    if (gen == NULL) {
        fprintf(stderr, "Error: Failed to create TestU01 generator.\n");
        return 1; // Exit if generator creation failed
    }

    // Run the BigCrush test battery on the generator
    printf("Starting TestU01 BigCrush for DualMix128...\n");
    bbattery_BigCrush(gen);
    printf("TestU01 BigCrush finished.\n");

    // Clean up: delete the generator object to free resources
    unif01_DeleteExternGen01(gen);

    // Return success
    return 0;
}
