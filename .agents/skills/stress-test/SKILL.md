---
name: stress-test
description: Measure evan's rendering performance - how many meshes and pipelines hold a frame rate - or compare performance before and after a change. Use for optimisation work and any request about fps, draw calls, culling, instancing or renderer limits.
---

# Stress-test the renderer

Read `docs/STRESS_TESTING.md` first: it defines the indicators and how to read
the results. `docs/PROFILING.md` covers the per-frame statistics.

```sh
scripts/run-stress.sh --quick                   # short run
scripts/run-stress.sh --thresholds "60 30"      # chosen frame rates
scripts/run-stress.sh --help                    # every option
```

- It opens windows and drives the `examples/GLFW-stress` example, so it needs
  a desktop session and a Vulkan driver. It cannot run in CI.
- Results go to `build/stress-results/<UTC timestamp>/` as markdown and CSV.
- To compare a change, run the same command before and after with
  `--out build/stress-results/<label>` and compare the two reports. Keep the
  window size, present mode and thresholds identical.
- Numbers depend on the machine and its load. Report the machine, the exact
  command and both runs; do not quote a single run as a result.
- `scripts/run-poc.sh` is the older proof-of-concept harness for the macOS
  example (`docs/DESIGN_P3_OPTIMIZATIONS.md`).

An optimisation commit uses the `optim` type (`docs/COMMIT_CONVENTIONS.md`)
and records its measurements in the docs, as the existing ones do.
