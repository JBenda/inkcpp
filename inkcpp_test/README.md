# inkcpp_test

Catch2-based test suite for inkcpp. Tests are written in BDD style using
`SCENARIO` / `GIVEN` / `WHEN` / `AND_WHEN` / `THEN` / `AND_THEN` macros.

## Running tests

```sh
# inside the build directory
# Run all tests
ctest -C Release

# Run test without bindings
ctest -V -C Release -R UnitTests
# or
inkcpp_test\Release\inkcpp_test.exe
# or inkcpp_test\Debug\inkcpp_test.exe
# or inkcpp_test\inkcpp_test.exe
# dependent on your build system


# Run a single test by scenario name
inkcpp_test.exe "Scenario: UE example story snapshot migratability"

# Run all tests matching a tag expression
inkcpp_test.exe "[migration]"
inkcpp_test.exe "[regression]"
inkcpp_test.exe "[migration][integration]"
```

## Tag scheme

Every `SCENARIO` is tagged along three independent dimensions. Tags compose
freely, so you can narrow a run to any intersection (e.g. `[regression][runtime]`
re-runs only runtime regression tests).

| Dimension     | Purpose                            | Values                                                                                                                                                                                                                                                                                    |
|---------------|------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Feature**   | What capability is under test      | `[array]`, `[callstack]`, `[choices]`, `[compiler]`, `[external-functions]`, `[globals]`, `[glue]`, `[labels]`, `[list-matching]`, `[lists]`, `[migration]`, `[move-to]`, `[observer]`, `[output]`, `[pointer]`, `[restorable]`, `[snapshot]`, `[stack]`, `[tags]`, `[utf-8]`, `[values]` |
| **Component** | Which subsystem owns the behaviour | `[compiler]`, `[internals]`, `[runtime]`                                                                                                                                                                                                                                                  |
| **Type**      | What kind of test it is            | `[integration]`, `[regression]`, `[unit]`                                                                                                                                                                                                                                                 |

### Feature tags

| Tag                    | What it covers                                                  |
|------------------------|-----------------------------------------------------------------|
| `[array]`              | `allocated_restorable_array` internals                          |
| `[callstack]`          | Thread forking and collapse on the internal callstack           |
| `[choices]`            | Choice presentation, selection, and bracketed choice edge cases |
| `[compiler]`           | Ink JSON → binary compilation                                   |
| `[external-functions]` | Binding and calling external (C++) functions from Ink           |
| `[globals]`            | Global variable get/set and observer callbacks                  |
| `[glue]`               | Glue (`<>`) joining lines across knots and functions            |
| `[labels]`             | Label-based conditional choices                                 |
| `[list-matching]`      | Hungarian solver and list similarity distance functions         |
| `[lists]`              | Ink list creation, modification, and iteration                  |
| `[migration]`          | Snapshot migration between story versions                       |
| `[move-to]`            | `runner::move_to()` — jumping to named knots at runtime         |
| `[observer]`           | Variable observer callbacks (typed, generic, ping, new/old)     |
| `[output]`             | Line output formatting, newlines, whitespace, and glue          |
| `[pointer]`            | `story_ptr` reference-counted pointer internals                 |
| `[regression]`         | Tests added to prevent a specific filed bug from recurring      |
| `[restorable]`         | `restorable<T>` collection save/restore/forget                  |
| `[snapshot]`           | Snapshot creation, serialisation, and restoration               |
| `[stack]`              | `simple_restorable_stack` push/pop/save/restore                 |
| `[tags]`               | Ink tag reading (global, knot, inline, choice tags)             |
| `[utf-8]`              | Multi-byte character handling throughout the pipeline           |
| `[values]`             | Internal `value` type arithmetic and equality                   |

### Component tags

| Tag           | What it covers                                                        |
|---------------|-----------------------------------------------------------------------|
| `[compiler]`  | `ink::compiler` — Ink JSON → inkcpp binary                            |
| `[internals]` | `ink::runtime::internal` — data structures not part of the public API |
| `[runtime]`   | `ink::runtime` — story, runner, globals public API                    |

### Type tags

| Tag             | What it covers                                                                        |
|-----------------|---------------------------------------------------------------------------------------|
| `[integration]` | Tests that exercise multiple components together (e.g. compiler + runtime + snapshot) |
| `[regression]`  | Tests tied to a specific GitHub issue, named `_ #NNN` in the scenario                 |
| `[unit]`        | Tests of a single class or function in isolation, usually with no story file          |

## BDD conventions

- **`GIVEN`** — sets up the state of the world (story loaded, globals created, etc.).
  Contains no assertions.
- **`WHEN`** — performs a single action (run a line, make a choice, take a snapshot).
  Contains no assertions.
- **`AND_WHEN`** — a dependent action that follows a prior `WHEN`. Placed *inside* the
  `WHEN` it depends on.
- **`THEN`** — asserts one observable outcome. Contains no story-advancing calls.
- **`AND_THEN`** — an additional assertion that belongs to the same observable moment.

Multiple independent `WHEN` blocks inside one `GIVEN` are run as separate test
paths — Catch2 re-enters the `GIVEN` once per `WHEN`.

For further reference see [`test-cases-and-sections.md`](https://github.com/catchorg/Catch2/blob/devel/docs/test-cases-and-sections.md)
in the repository root.
