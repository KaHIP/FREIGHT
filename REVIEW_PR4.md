# Code Review: PR #4 — Multipass performance optimizations (~4.3x speedup)

> Reviewed branch: `perf/multipass-speedup`  
> Reviewed files: 11 changed files, +470 / −166 lines

---

## 🔴 Critical Issues

### 1. Data race on `nodes_moved` counter (`freight.cpp`)

**File:** `code_for_hypergraphs/app/freight.cpp`

Inside the `#pragma omp parallel for` loop, `nodes_moved` is a plain `LongNodeID`
variable shared across threads with no synchronization:

```cpp
LongNodeID nodes_moved = 0;
...
#pragma omp parallel for schedule(runtime)
for (LongNodeID curr_node = 0; curr_node < config.n_batches; curr_node++) {
    ...
    if (config.restream_number > 0 && block != old_block_for_move_check) {
        nodes_moved++;   // ← data race: multiple threads write without synchronization
    }
}
```

**Fix:** Use `#pragma omp atomic` or declare `nodes_moved` as `std::atomic<LongNodeID>`:

```cpp
#pragma omp atomic
nodes_moved++;
```

The same issue applies in `code_for_graphs/app/streammultisection.cpp` if a
similar counter is later added there.

---

### 2. Data race on `first_pass_bits` and `first_pass_connectivity` (`freight.cpp`)

**File:** `code_for_hypergraphs/app/freight.cpp`

The inline connectivity computation performed inside the parallel loop is not
thread-safe. Two threads may process different nodes that both reference the
same net (`net_idx`), leading to a read-modify-write race on both the bitset
array and the floating-point accumulator:

```cpp
#pragma omp parallel for schedule(runtime)
for (LongNodeID curr_node = 0; ...) {
    ...
    if (n_edges_for_bits > 0) {
        while (cc < ls) {
            size_t net_idx = ln[cc] - 1;
            uint64_t old_word = first_pass_bits[net_idx];    // read
            if (!(old_word & bit)) {
                if (old_word != 0) first_pass_connectivity += 1;  // race
                first_pass_bits[net_idx] = old_word | bit;        // race
            }
        }
    }
}
```

Both `first_pass_bits[net_idx]` (bit-OR update) and `first_pass_connectivity`
(increment) are accessed concurrently by multiple threads without any
`atomic`, `critical`, or reduction clause.

**Fix options:**
- Use `std::atomic<uint64_t>` for `first_pass_bits` entries and
  `#pragma omp atomic` for `first_pass_connectivity`.
- Or move the connectivity accumulation to a single-threaded post-processing
  step over the bitset (sum popcount - net_count logic), which avoids
  per-node synchronization entirely and is faster for large inputs.

---

## 🟠 Medium Issues

### 3. Missing error check on `open()` before `fstat()` / `mmap()` (both `graph_io_stream.h` copies)

**Files:** `code_for_hypergraphs/lib/io/graph_io_stream.h`,
`code_for_graphs/lib/io/graph_io_stream.h`

In `loadLinesFromStreamToBinary`, `open()` can fail and return `-1`, but its
return value is never checked before being passed to `fstat()`:

```cpp
int fd = open(partition_config.graph_filename.c_str(), O_RDONLY);
struct stat st;
fstat(fd, &st);                     // ← undefined behavior if fd == -1
size_t file_size = (size_t)st.st_size;  // ← uninitialized if fstat failed
void* mmap_addr = mmap(nullptr, file_size, ...);
close(fd);                          // ← close(-1) is benign but noisy
```

If `open()` fails (e.g., permissions, file not found), `fstat(-1, &st)` fails
silently and `st.st_size` contains an indeterminate value. `mmap` will then
also fail (fd=-1), falling back to the `buf` path, but with `file_size`
potentially holding garbage — this is technically undefined behavior.

**Fix:**

```cpp
int fd = open(partition_config.graph_filename.c_str(), O_RDONLY);
struct stat st;
size_t file_size = 0;
void* mmap_addr = MAP_FAILED;
if (fd != -1 && fstat(fd, &st) == 0) {
    file_size = (size_t)st.st_size;
    mmap_addr = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
}
if (fd != -1) close(fd);
```

---

### 4. Duplicate code bodies in OMP critical guards (`vertex_partitioning.h`, both copies)

**Files:** `code_for_hypergraphs/lib/partition/onepass_partitioning/vertex_partitioning.h`,
`code_for_graphs/lib/partition/onepass_partitioning/vertex_partitioning.h`

The optimization to skip `#pragma omp critical` when single-threaded is sound,
but the resulting if/else has identical code in both branches:

```cpp
if (n_threads > 1) {
#pragma omp critical(update_self_sorting_vector)
    this->sorted_blocks.increment(decision);
} else {
    this->sorted_blocks.increment(decision);  // ← identical body
}
```

This applies to `increment` in two `case` blocks and `decrement_sorted_block`.
Maintaining the same logic in two places is error-prone. A cleaner approach is
to use a helper lambda or macro:

```cpp
auto do_increment = [&]() { this->sorted_blocks.increment(decision); };
if (n_threads > 1) {
#pragma omp critical(update_self_sorting_vector)
    do_increment();
} else {
    do_increment();
}
```

Or even simpler, since `#pragma omp critical` with a single thread is
effectively a no-op at runtime, the real saving comes from the branch itself
avoiding the OMP overhead. But duplication should still be reduced.

---

### 5. PGO CMake flags are GCC/Clang-specific without a compiler guard (`CMakeLists.txt`, both copies)

**Files:** `code_for_hypergraphs/CMakeLists.txt`, `code_for_graphs/CMakeLists.txt`

The Profile-Guided Optimization flags are added unconditionally, without
checking that the compiler actually supports them:

```cmake
if(PGO_GENERATE)
  add_definitions(-fprofile-generate=${CMAKE_BINARY_DIR}/pgo_data)
  ...
endif()
```

The `-fprofile-generate` and `-fprofile-use` flags are GCC/Clang-only and will
cause build failures with MSVC or Intel ICC. Add a compiler identity check:

```cmake
if(PGO_GENERATE)
  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_definitions(-fprofile-generate=${CMAKE_BINARY_DIR}/pgo_data)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-generate=...")
  else()
    message(WARNING "PGO_GENERATE is only supported with GCC or Clang")
  endif()
endif()
```

---

### 6. `evaluate_cut` / `evaluate_connectivity` fields lack default initializers (`partition_config.h`)

**File:** `code_for_hypergraphs/lib/partition/partition_config.h`

The two new fields are plain `bool` with no default member initializer:

```cpp
bool evaluate_cut;
bool evaluate_connectivity;
```

If a `PartitionConfig` object is ever constructed without calling
`configuration::standard()` first, these fields will have indeterminate values,
resulting in undefined behavior. Add default initializers:

```cpp
bool evaluate_cut = true;
bool evaluate_connectivity = true;
```

---

### 7. Inconsistent use of `line_size` vs `line_numbers.size()` in `streamEvaluateHPartition_netl` (`graph_io_stream.cpp`)

**File:** `code_for_hypergraphs/lib/io/graph_io_stream.cpp`

The loop pre-computes `line_size` for efficiency, but the prefetch guard uses
the slower `.size()` method call instead:

```cpp
LongNodeID line_size = line_numbers.size();
...
while (col_counter < line_size) {          // uses line_size ✓
    ...
    if (col_counter < line_numbers.size()) {  // should use line_size ✗
        __builtin_prefetch(&edges_data[line_numbers[col_counter]-1], 1, 1);
    }
```

The prefetch guard should use `line_size` for consistency and to avoid a
redundant virtual dispatch on every iteration.

---

### 8. Cached `best_cut/best_con/best_qap/best_pins` variables appear to be unused

**File:** `code_for_hypergraphs/app/freight.cpp`

Four variables are declared and populated during the multi-pass best-partition
tracking:

```cpp
double best_cut = 0, best_con = 0;
EdgeWeight best_qap = 0;
LongNodeID best_pins = 0;
```

These are assigned inside the loop (`best_cut = pass_cut;` etc.) but are never
read again. The final evaluation always re-reads the file. Either these should
be used to skip the final re-evaluation (the apparent intention), or they should
be removed to avoid misleading readers.

---

## 🟡 Minor / Nits

### 9. Removed `config.remaining_stream_nodes--` in `readNodeOnePass_netl` without conditional

**File:** `code_for_hypergraphs/lib/io/graph_io_stream.h`

The `config.remaining_stream_nodes--` decrement was removed unconditionally
from `readNodeOnePass_netl`. The comment elsewhere says it is not needed for
`ram_stream`, but the original code also used it for non-ram_stream mode. A
comment explaining why this can be safely removed in all cases (or a conditional
like in the `_pinsl` variant) would aid reviewers.

### 10. The `cursor = curr_node` alias in `freight.cpp` is unnecessary

```cpp
LongNodeID cursor = curr_node;
graph_io_stream::register_result_from_input(config, curr_node, block, (*input)[cursor]);
```

`cursor` is always equal to `curr_node`. Using `curr_node` directly removes
the alias and makes the intent clearer.

---

## ✅ Positive observations

- The mmap fallback path is well-guarded: `mmap_addr != MAP_FAILED` is checked
  before both using the pointer and calling `munmap`.
- The `#ifdef MAP_POPULATE` guard correctly handles macOS vs. Linux portability.
- Replacing `std::unordered_set<PartitionID>` with a flat bitset for connectivity
  computation is a significant and correct algorithmic improvement.
- The `memset(..., 0xFF, ...)` substitution for the `INVALID_PARTITION` fill
  loop is a good micro-optimization (assuming `INVALID_PARTITION` maps to
  `0xFFFFFFFF` / all-ones, which should be verified in `definitions.h`).
- Moving `global_mapping_time` accumulation outside the parallel loop
  (`global_mapping_time += processing_t.elapsed()`) correctly fixes a
  pre-existing race on that counter.
- The `nodes_moved > 0` early-exit optimization for subsequent passes is a
  clean idea — just needs the race fix noted above.
