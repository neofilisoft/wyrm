---
name: the-7-coding-laws
description: >-
  Provides guidelines and best practices based on The 7 Coding Laws for writing clean, maintainable, and robust code. Use when designing, writing, reviewing, or refactoring code.
---

# The 7 Coding Laws

Guidelines and principles for writing clean, maintainable, and reliable systems code.

## The 7 Coding Laws

### 1. Keep the main path easy to follow
Avoid deeply nested conditional logic. Use guard clauses and early returns to handle edge cases up front so the primary operation remains clear and visible.

### 2. Name things by meaning
Avoid vague identifiers like `data`, `result`, or `item`. Use domain-specific names (e.g., `pendingOrder`, `processOrder`) that clearly convey intent and reduce guesswork.

### 3. Keep external systems behind a boundary
Do not let third-party data structures or API schemas leak throughout your codebase. Translate external inputs into internal domain models at the boundary so upstream changes can be handled in a single place.

### 4. Make invalid states harder to represent
Avoid making every field optional. Model domain states explicitly (e.g., a dedicated type for a PaidOrder requiring payment details) so incorrect states cannot easily exist.

### 5. Separate decisions from actions
Decouple business logic and decision-making from side effects (such as database writes, external API calls, or sending emails). This makes rules straightforward to test in isolation.

### 6. Make errors useful
Use clear, human-readable messages alongside structured error codes for systems. Include helpful context for debugging while ensuring sensitive data (passwords, tokens, secrets) is never logged.

### 7. Keep your changes focused
Avoid monolithic pull requests that combine new features, refactoring, and database schema updates. Smaller, single-purpose changes are significantly easier to review, test, debug, and revert if necessary.
