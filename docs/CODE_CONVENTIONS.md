# Code Conventions

This document defines coding standards for Evan. The goal is consistent,
readable, and maintainable code.

## General Rules

- Use clear and descriptive names.
- Keep indentation consistent (4 spaces unless file style differs).
- Target line length around 80 to 100 characters.
- Prefer small, focused functions over large monolithic blocks.
- Avoid duplicated logic (DRY principle).
- Add comments only when intent is not obvious.
- Validate inputs and handle errors explicitly.

## C++ Conventions

- Use `PascalCase` for classes and `camelCase` for functions and variables.
- Prefix member fields with `_`.
- Use `nullptr` instead of `NULL` or `0`.
- Prefer RAII and smart pointers (`std::unique_ptr`, `std::shared_ptr`).
- Apply `const` correctness whenever possible.
- Prefer `enum class` over unscoped enums.

## Class Layout

In class declarations, keep sections in this order:

1. `public`
2. `protected`
3. `private`

Within each section, keep this order:

1. Types (`struct`, `enum`, aliases)
2. Constructors and special members
3. Methods
4. Fields

## Checklist Before Opening A PR

- Naming follows conventions.
- Class and method order is consistent.
- No obvious duplication.
- Error handling is present where needed.
- Formatting is clean and consistent.
