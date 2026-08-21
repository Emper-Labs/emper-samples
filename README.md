# Emper Samples

Example applications and experimental workloads built with Emper Engine.

Samples provide a practical way to explore the Emper ecosystem, validate engine and module design, and experiment with simulation algorithms.

They are intentionally separate from the engine and reusable modules.

---

## Purpose

Samples serve several purposes:

* Demonstrate how Emper is used
* Validate engine APIs through real applications
* Experiment with simulation algorithms
* Benchmark implementations
* Explore new ideas before turning them into reusable modules
* Provide simple starting points for developers

A sample does not need to become a permanent part of the engine or module ecosystem.

---

## Architecture

Samples sit at the top of the Emper dependency hierarchy.

```text id="d4k8v2"
        Sample Application
                │
        ┌───────┴───────┐
        ▼               ▼
     Modules         Engine
        │               │
        └───────┬───────┘
                ▼
             Backends
```

Samples consume the infrastructure provided by the rest of the ecosystem.

They should not introduce dependencies back into the engine or reusable modules.

---

## Samples as Experiments

A sample is more than a demonstration.

It is an environment for answering practical questions:

```text id="q7m3x1"
Can this API express the simulation?
        ↓
Does the implementation behave correctly?
        ↓
How does it perform?
        ↓
What problems appear in a real workload?
        ↓
What should the architecture learn from it?
```

This makes samples an important part of Emper's development process.

Architecture should be validated through real workloads rather than designed entirely in advance.

---

## From Experiment to Module

When an algorithm proves useful beyond a single experiment, it can be extracted into a reusable module.

```text id="p2v8k6"
Experiment
    │
    ▼
Sample
    │
    ├── Validate correctness
    ├── Measure performance
    └── Discover reusable abstractions
            │
            ▼
       Reusable Module
```

The sample can then remain as an example and validation workload for the resulting module.

---

## What Belongs in a Sample?

Samples may contain application-specific code such as:

* Simulation configuration
* Visualization
* User interaction
* Input handling
* Debug information
* Benchmarking
* Experiment-specific algorithms
* Temporary prototypes

Not every piece of sample code needs to become part of the reusable ecosystem.

The goal is to make experiments easy to build and evaluate.

---

## What Does Not Belong in a Sample?

Reusable infrastructure should not be duplicated across samples.

If functionality is:

* broadly reusable,
* independent of one experiment,
* useful to multiple applications,

it should generally be considered for the engine or a module instead.

Likewise, samples should not modify engine internals simply to support a single experiment unless the experiment exposes a genuine engine-level requirement.

---

## Performance and Benchmarking

Samples may be used to evaluate performance under realistic workloads.

Performance results should be treated as workload-specific measurements rather than universal claims.

When reporting benchmarks, document relevant conditions such as:

* Hardware
* Operating system
* Build configuration
* Simulation size
* Backend
* Execution mode

Optimization should be driven by profiling and measurement.

---

## Headless and Visual Workloads

A sample may be visual or headless depending on what is being tested.

Visual samples are useful for:

* Debugging
* Visualization
* Interaction
* Demonstration

Headless samples are useful for:

* Benchmarking
* Automated testing
* Large-scale simulation
* Deterministic experiments

The simulation itself should remain independent from whether a sample renders anything.

---

## Development Principles

### Keep Samples Focused

A sample should answer a specific question or demonstrate a coherent workload.

### Prefer Real Workloads

Use samples to test real simulation problems rather than artificial architecture exercises.

### Keep Experiments Reproducible

Where practical, provide enough information for others to build and run the workload.

### Do Not Over-Engineer

Experimental code is allowed to be simple, temporary, or imperfect.

### Feed Results Back Into the Ecosystem

Problems discovered in samples should influence the evolution of the engine and modules when appropriate.

---

## Project Status

Emper Samples is under active development.

The collection of samples is expected to evolve as new simulation workloads are explored.

Samples may be added, removed, or reorganized without requiring changes to the underlying engine architecture.

---

## License

Apache License 2.0
