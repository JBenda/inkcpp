# Contribution Guide

Want to contribute to inkcpp? Great! There's plenty to do

## What Should I Work On?

Check the project's Issues page to see if there's anything you can take care of. If not, check out the inkcpp v1 project on GitHub. Anything under "Could Use Help With" is fair game. Contact brwarner if you're unsure and need help.

# Guidelines

Here's some quick guidelines when writing code for the project.

## Code Style

Broadly, the interface style of inkcpp should resemble that of the C++ standard library.

* All type names (classes, structs, typedefs, namespaces, etc.) should be written in snake_case (lowercase, words seperated by underlines).
* Typedefs/usings to integral types should have the suffix `_t`. E.g. `hash_t` and `uint32_t`.
* When defining content in a nested namespace, use the new C++ syntax (`namespace a::b::c { ... }`) to reduce the amount of indentation

## Namespaces
* Compiler code lives in the `ink::compiler` namespace
* Runtime code lives in the `ink::runtime` namespace
* Shared code lives in the `ink` namespace
* Any non-user facing code should live in the corresponding `internal` namespace.
** For example, the `restorable_array` class is never exposed outside the library, and thus lives in `ink::runtime::internal`

## Unit Tests

Write unit tests in inkcpp_test using Catch2 whenever you create new utility types or functions, especially if you anticipate a lot of potential runtime breaking edge cases. This is especially important for the collection types which support save/restore and threading.

## (Near) Zero Heap Allocation

The inkcpp runtime should never dynamically allocate memory unless there's an emergency (stack overflow, unable to store dynamic string, etc.) The average case of the runtime should have zero heap allocations.

## C++ Standard Template Library

Any use of the C++ Standard Template Library needs to be wrapped in `#ifdef INK_ENABLE_STL`. STL should only be used to create cleaner interfaces for existing functions in inkcpp. *Inkcpp must be able to run, fully-featured, without STL.* Do not use STL in any implementations.

The only exceptions are the compiler, which uses various STL collections, and the inkcpp_cl executable code. Everything else must be wrapped.

## Unreal

Where useful, add helper functions with Unreal types to the inkcpp interface. Wrap them in `#ifdef INK_ENABLE_UNREAL`. This flag will only be set when building the Unreal plugin.
