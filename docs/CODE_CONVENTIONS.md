# Code Conventions

This document outlines the coding standards and conventions followed in this project to ensure code quality, readability, and maintainability.

## General Guidelines

- Follow consistent naming conventions for variables, functions, classes, and files.
- Use meaningful and descriptive names that convey the purpose of the entity.
- Maintain a consistent indentation style (e.g., 4 spaces per indentation level).
- Limit line length to 80-100 characters for better readability.
- Use comments to explain complex logic, but avoid over-commenting obvious code.
- Write modular and reusable code by breaking down large functions into smaller, focused ones.
- Adhere to the DRY (Don't Repeat Yourself) principle to minimize code duplication.
- Ensure proper error handling and input validation.

## Language-Specific Conventions

### C++

- Use `CamelCase` for class names and `PascalCase` for variable and function names.
- For member variables, use a leading underscore (e.g., `_memberVariable`).
- Use `nullptr` instead of `NULL` or `0` for pointer initialization.
- Prefer `std::unique_ptr` and `std::shared_ptr` for dynamic memory management.
- Use `const` correctness to indicate immutability where applicable.
- Prefer `enum class` over traditional enums for better type safety.
