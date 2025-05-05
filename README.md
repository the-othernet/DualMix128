# DualMix128: Fast and Robust 128-bit Pseudo-Random Number Generator

This repository contains `DualMix128`, a simple and extremely fast pseudo-random number generator (PRNG) with 128 bits of state. It is designed for non-cryptographic applications where speed and statistical quality are important.

## Features

* **High Performance:** Significantly faster than standard library generators and competitive with or faster than other modern high-speed PRNGs like xoroshiro128++ and wyrand.
* **Good Statistical Quality:** Has passed PractRand with zero anomalies (256MB to 32TB) as well as TestU01's BigCrush suite.

## Performance

* **Speed:** 9.05x Java random, 26% faster than Java xoroshiro128++, 104% faster than C xoroshiro128++
* Passed 256M to 32TB PractRand with zero anomalies
* Passed BigCrush with these lowest p-values:
    * `0.02` smarsa_CollisionOver (N=30, n=20000000, r=0, d=64, t=7)
    * `0.02` sstring_PeriodsInStrings (N=10, n=500000000, r=20, s=10)
    * `0.04` svaria_SampleMean test: (N=20000000, n=30, r=0)
    * `0.05` sknuth_CouponCollector test: (N=1, n=200000000, r=10, d= 8)
    * `0.05` sknuth_MaxOft test: (N=20, n=10000000, r=0, d=100000, t=24)

## Algorithm Details

```
// Golden ratio fractional part * 2^64
const uint64_t GR = 0x9e3779b97f4a7c15ULL;

// Initialized to non-zero with SplitMix64 (or equivalent)
uint64_t state0, state1; 

// Helper for rotation
static inline uint64_t rotateLeft(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

// --- DualMix128 ---
uint64_t dualMix128() {
    uint64_t mix = state0 + state1;
    state0 = mix + rotateLeft( state0, 26 );
    state1 = mix ^ rotateLeft( state1, 35 );

    return GR * mix;
}
```

*(Note: The code above shows the core logic. See implementation files for full seeding and usage examples.)*


## PractRand Testing (With Varied Seeds)

As running BigCrush and PractRand can behave differently depending on the initial seeded states, PractRand was also run multiple times from 256M to 8GB using varied initial seeds (seeding with SplitMix64). Below are the counts of total suspicious results when running PractRand 1000 times for the DualMix128 variants and some reference PRNGs:

```
DualMix128          0 failures, 24 suspicious
xoroshiro256++      0 failures, 27 suspicious
xoroshiro128++      0 failures, 28 suspicious
wyrand              0 failures, 32 suspicious
/dev/urandom        0 failures, 37 suspicious
```
