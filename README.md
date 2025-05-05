# DualMix128: Fast and Robust 128-bit Pseudo-Random Number Generators

This repository contains `DualMix128A` and `DualMix128B`, two simple and extremely fast pseudo-random number generators (PRNGs) using 128 bits of state (two 64-bit variables). They are designed for non-cryptographic applications where speed and statistical quality are important.

## Features

* **High Performance:** Significantly faster than standard library generators and competitive with or faster than other modern high-speed PRNGs like xoroshiro128++ and wyrand.
* **Good Statistical Quality:** Both have passed PractRand with zero anomalies (256MB to 32TB) as well as TestU01's BigCrush suite.

## DualMix128A

* **Speed:** 9.05x Java random, 26% faster than Java xoroshiro128++, 75% faster than C xoroshiro128++, 4% faster than C wyrand
* Passed 256M to 32TB PractRand with zero anomalies
* Passed BigCrush with these lowest p-values:
    * `0.02` smarsa_CollisionOver (N=30, n=20000000, r=0, d=64, t=7)
    * `0.02` sstring_PeriodsInStrings (N=10, n=500000000, r=20, s=10)
    * `0.04` svaria_SampleMean test: (N=20000000, n=30, r=0)
    * `0.05` sknuth_CouponCollector test: (N=1, n=200000000, r=10, d= 8)
    * `0.05` sknuth_MaxOft test: (N=20, n=10000000, r=0, d=100000, t=24)

## DualMix128B

* **Speed:** 9.09x Java random, 27% faster than Java xoroshiro128++, 94% faster than C xoroshiro128++, 15% faster than C wyrand
* Passed 256M to 32TB PractRand with zero anomalies
* Passed BigCrush with these lowest p-values:
    * `0.002` swalk_RandomWalk1 (N=1, n=100000000, r=0, s=5, L0=50, L1=50)
    * `0.003` sstring_AutoCor (N=10, n=1000000029, r=0, s=3, d=3)
    * `0.004` sstring_AutoCor (N=10, n=1000000029, r=27, s=3, d=3)
    * `0.007` sstring_AutoCor (N=10, n=1000000029, r=0, s=3, d=3)
    * `0.008` snpair_ClosePairs (N=30, n=6000000, r=0, t=3)

## Algorithm Details

```c
const unsigned long long GR = 0x9e3779b97f4a7c15ULL; // Golden ratio fractional part * 2^64
unsigned long long state0, state1; // initialized to non-zero with SplitMix64 (or equivalent)

// Helper for rotation
static inline uint64_t rotateLeft(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

// --- DualMix128A ---
unsigned long long nextDualMixA() {
    unsigned long long mix = state0 + state1;
    state0 = mix + rotateLeft( state0, 26 );
    state1 = mix ^ rotateLeft( state1, 35 );

    return GR * mix;
}

// --- DualMix128B ---
unsigned long long nextDualMixB() {
    unsigned long long mix = state0 + state1;
    state0 = mix + rotateLeft( state0, 26 );
    state1 = mix ^ rotateLeft( state1, 35 );

    return mix;
}
```

*(Note: The code above shows the core logic. See implementation files for full seeding and usage examples.)*


## PractRand Testing (With Varied Seeds)

As running BigCrush and PractRand can behave differently depending on the initial seeded states, PractRand was also run multiple times from 256M to 8GB using varied initial seeds (seeding with SplitMix64). Below are the counts of total suspicious results when running PractRand 1000 times for the DualMix128 variants and some reference PRNGs:

```
DualMix128A         0 failures, 24 suspicious
DualMix128B         0 failures, 27 suspicious
xoroshiro256++      0 failures, 27 suspicious
xoroshiro128++      0 failures, 28 suspicious
wyrand              0 failures, 32 suspicious
/dev/urandom        0 failures, 37 suspicious
```
