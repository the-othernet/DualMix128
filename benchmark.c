#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> // For atoll
#include <time.h>   // For clock_gettime


// Golden ratio constant for DualMix128 - golden ratio fractional part * 2^64
const uint64_t GR = 0x9e3779b97f4a7c15ULL;

// --- State Variables (Global for this benchmark) ---
uint64_t state0, state1; // For DualMix128
uint64_t xoro_s[2];      // For xoroshiro128++
uint64_t wyrand_seed = 0; // For wyrand


// --- Helper Functions ---

// Rotate left function (used by DualMix and xoroshiro)
inline uint64_t rotateLeft(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}


// Simple seeding for DualMix128 (ensures non-zero state)
void seed_dualmix128(uint64_t seed1, uint64_t seed2) {
    state0 = seed1;
    state1 = seed2;
    // A robust implementation would check if both seeds are 0
}


// Simple seeding for xoroshiro128++ (assumes non-zero input for valid state)
void seed_xoroshiro128pp(uint64_t seed1, uint64_t seed2) {
    xoro_s[0] = seed1;
    xoro_s[1] = seed2;
    // A robust implementation would check if both seeds are 0
}


// Get time using CLOCK_MONOTONIC for reliable interval timing
double get_time_sec() {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("clock_gettime failed");
        exit(EXIT_FAILURE);
    }
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}


// --- PRNG Implementations ---

// DualMix128 generator function
inline uint64_t dualMix128() {
    uint64_t mix = state0 + state1;
    state0 = mix + rotateLeft(state0, 16);
    state1 = mix + rotateLeft(state1, 2);

    return GR * mix;
}


// Wyrand generator function (requires __uint128_t support)
inline uint64_t wyrand(void) {
    wyrand_seed += 0xa0761d6478bd642fULL;
    __uint128_t t = (__uint128_t)(wyrand_seed ^ 0xe7037ed1a0b428dbULL) * wyrand_seed;
    return (uint64_t)(t >> 64) ^ (uint64_t)t;
}


// xoroshiro128++ generator function
inline uint64_t xoroshiro128pp(void) {
    const uint64_t s0 = xoro_s[0];
    uint64_t s1 = xoro_s[1];
    const uint64_t result = rotateLeft(s0 + s1, 17) + s0;

    s1 ^= s0;
    xoro_s[0] = rotateLeft(s0, 49) ^ s1 ^ (s1 << 21); // a, b
    xoro_s[1] = rotateLeft(s1, 28); // c
    return result;
}


// --- Main Benchmark Routine ---
int main(int argc, char **argv) {
    uint64_t num_iterations = 10000000000ULL; // Default: 10 Billion iterations

    // Parse command-line argument for number of iterations
    if (argc > 1) {
        long long arg_val = atoll(argv[1]);
        if (arg_val > 0) {
            num_iterations = (uint64_t)arg_val;
        } else {
            fprintf(stderr, "Warning: Invalid number of iterations '%s', using default %llu\n",
                    argv[1], (unsigned long long)num_iterations);
        }
    }

    printf("Benchmarking PRNGs for %llu iterations...\n\n", (unsigned long long)num_iterations);

    // Use volatile to prevent compiler optimizing out the PRNG calls
    volatile uint64_t dummyVar = 0;
    double start_time, end_time, duration;
    double ns_per_call;


    // --- Benchmark DualMix128 ---
    printf("Benchmarking DualMix128...\n");
    seed_dualmix128(0xDEADBEEF12345678ULL, 0xABCDEF0123456789ULL); // Arbitrary non-zero seeds
    start_time = get_time_sec();
    for (uint64_t i = 0; i < num_iterations; ++i)
        dummyVar = dualMix128();

    end_time = get_time_sec();
    duration = end_time - start_time;
    ns_per_call = (duration * 1e9) / num_iterations;
    printf("  DualMix128 ns/call:     %.3f ns\n", ns_per_call);

    // --- Benchmark xoroshiro128++ ---
    printf("\nBenchmarking xoroshiro128++...\n");
    seed_xoroshiro128pp(0x12345678DEADBEEFULL, 0x6789ABCDEF012345ULL); // Different arbitrary seeds
    start_time = get_time_sec();
    for (uint64_t i = 0; i < num_iterations; ++i)
        dummyVar = xoroshiro128pp();

    end_time = get_time_sec();
    duration = end_time - start_time;
    ns_per_call = (duration * 1e9) / num_iterations;
    printf("  xoroshiro128++ ns/call: %.3f ns\n", ns_per_call);

    // --- Benchmark wyrand ---
    printf("\nBenchmarking wyrand...\n");
    wyrand_seed = 0; // Reset wyrand seed for benchmark consistency
    start_time = get_time_sec();
    for (uint64_t i = 0; i < num_iterations; ++i)
        dummyVar = wyrand();

    end_time = get_time_sec();
    duration = end_time - start_time;
    ns_per_call = (duration * 1e9) / num_iterations;
    printf("  wyrand ns/call:         %.3f ns\n", ns_per_call);

    printf("\nBenchmark complete.\n");

    return 0;
}
