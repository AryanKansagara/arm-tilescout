# TileScout

A portable C++17 integration contract laboratory for packed int8 matrix multiplication. Arm provides processor IP and optimized software including KleidiAI. Its micro-kernels already exist; integration still depends on packing, dimensions, tails, partitioning and instruction eligibility. This project tests authored wrappers before any real Arm kernel integration.

## Run

```sh
make test
make demo
```

Only GNU Make and a C++17 compiler are needed. The demo emits JSON from actual portable native computation. Public functions in `tilescout.hpp` accept caller-provided matrices and packing metadata; `main.cpp` executes a declared boundary corpus rather than pretending to be a general tensor file importer.

## Contract

`C = A x B`, signed int8 inputs and int64 accumulation, with each dimension in 0..64. Two packed B layouts are implemented: padded column-major and four-column interleaving. Both pad K/N to multiples of four. Offsets 0..7 exercise wrappers without assuming aligned data. Leading/trailing sentinels and zero padding are validated before execution. Packing metadata includes original and padded shapes, layout and offset; corrupted dimensions, unsupported layouts, padding, guard bytes and byte counts reject.

The blocked wrapper traverses padded tiles with explicit K/N tails. A separate row-major scalar routine indexes the original unpacked B matrix and supplies the comparison. An authored 2x3 by 3x2 golden matrix establishes expected values independently of both implementations. Row partitions must cover each output row once: holes, overlap, reversed and out-of-range ranges reject. The ranges are executed sequentially; **no thread safety or actual concurrent execution is validated**.

An injected `(aarch64, dotprod, layout)` policy returns eligibility or fallback. Eligibility never causes unsupported machine instructions to execute: both paths are portable C++. It is a dispatch-policy test, not actual CPU feature probing or an Arm acceleration result.

## Actual verification

`make test` passes **10 groups / 31 assertions**. One assertion checks the full generated corpus: **600** combinations of zero/odd/exact-tail dimensions, two layouts and three offsets match the independent indexing routine. The demo also counts detected seeded defects that drop the last tail column. Not every zero-output case can expose a dropped column, so the report gives detections rather than calling every injected case a detection. Other controls cover int8 extrema, padding and guard corruption, dispatch combinations, bad metadata, malformed partitions and shape bounds.

`demo.json` and `test-results.txt` are local evidence. Prepared CI has not been run remotely. Output is exact integer comparison; the tool makes no quantization-scale, floating-point tolerance or throughput claim.

## Sources and limitations

- [Arm KleidiAI](https://github.com/ARM-software/kleidiai)
- [Arm Compute Library](https://www.arm.com/products/development-tools/embedded-and-software/compute-library)

This complements existing kernels/runtimes rather than replacing them. No Arm instructions, KleidiAI adapter, real ISA detection, saturation/quantized scaling, real concurrent threads, performance comparison or deployment has been tested. A future adapter must pin the kernel API and license, honor its precise packing contract, detect actual host capabilities, and compare the same golden/boundary fixtures on compatible hardware.
