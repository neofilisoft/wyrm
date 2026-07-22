# Configure GitHub Language Statistics

**Goal**: Ensure that GitHub displays the repository languages in the desired order:
1. **Wyrm** (most prominent)
2. **C** (second most prominent)
3. Supporting languages (Python, HTML, C++, JavaScript, CMake) as minorities.

## User Review Required

> [!IMPORTANT]
> This plan modifies repository configuration files (`.gitattributes`). Ensure you have commit access and are okay with committing these changes to the main branch.

## Proposed Changes

---
### [MODIFY] [.gitattributes](file:///C:/Users/BEST/.gemini/antigravity-ide/brain/de374b9a-ba65-4b0d-8b82-5f3deb642f4e/.gitattributes)
```diff
+# Mark Wyrm source files as the primary language
+*.wyrm linguist-language=Wyrm
+
+# Ensure C files are recognized as C (default, explicit for clarity)
+*.c linguist-language=C
+
+# Mark supporting language files as "documentation" to down‑weight them
+*.py linguist-documentation
+*.html linguist-documentation
+*.cpp linguist-language=C++ linguist-vendored
+*.js linguist-language=JavaScript linguist-vendored
+CMakeLists.txt linguist-language=CMake linguist-vendored
+
+# Exclude generated files and test output from language stats
+tests/** linguist-vendored
+*.pyc linguist-vendored
+__pycache__/** linguist-vendored
```
---

## Verification Plan

1. Commit the `.gitattributes` file locally.
2. Push to a feature branch and open a Pull Request.
3. Observe the **Languages** bar on the PR page (GitHub renders language stats after a short delay).
4. Verify that **Wyrm** appears as the dominant language, followed by **C**, with other languages shown as minor or documentation.
5. If the ordering is not as expected, adjust the `linguist-documentation` and `lingist-vendored` entries accordingly and repeat steps 1‑4.

Once verified, merge the PR into the default branch.
