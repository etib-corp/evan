# Error Description

This document provides detailed descriptions of common errors that may occur while using the software, along with possible causes and suggested solutions.

## Base Error Class

The base class for all errors in the software is `Error`, defined in [Error.hpp](../src/Error.hpp). This class encapsulates common properties and methods for error handling.

### Properties

- `message`: A string that describes the error.
- `code`: An integer representing the error code.
- `severity`: An enum indicating the severity level of the error (Low, Medium, High).

### How error messages are constructed

Error messages are constructed using a combination of the `message` property and additional context provided at the time the error is thrown. This may include file names, line numbers, or specific function names where the error occurred.

*Example:*

```cpp
throw MyClassError("File not found", 404, ErrorSeverity::HIGH);
```

Will be shown as:

```
Error [404]: File not found
```

## Severity Levels

Errors are categorized into three severity levels:

- **Low Severity [1]**: These errors do not significantly impact the functionality of the software. They may cause minor inconveniences but can often be ignored or resolved with simple workarounds.

- **Medium Severity [2]**: These errors affect the usability of the software and may hinder certain features. Users should address these errors to ensure smooth operation. It can affect user experience or performance.

- **High Severity [3]**: These errors critically impact the functionality of the software, potentially causing crashes or data loss. Immediate attention is required to resolve these issues. The program will instantly terminate or become unusable.

### In code

It is represented as an enum called `ErrorSeverity` in [Error.hpp](../src/Error.hpp):

```cpp
enum ErrorSeverity {
    LOW = 1,
    MEDIUM = 2,
    HIGH = 3
};
```

