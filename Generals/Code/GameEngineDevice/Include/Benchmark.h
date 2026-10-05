#ifndef __BENCHMARK_STUB_H
#define __BENCHMARK_STUB_H

// Benchmark.h was stripped from the public EA dump (Libraries/Source/Benchmark is
// gitignored). The only referenced entry point is RunBenchmark, used from debug
// paths; this no-op stub keeps those call sites compiling.
#include "Lib/BaseType.h"

static inline void RunBenchmark(int /*flags*/, void* /*testMap*/, Real * /*floatBench*/, Real * /*intBench*/, Real * /*memBench*/)
{
}

#endif
