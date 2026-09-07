# GitHub Linguist Submission Guide & Pull Request Template for Wyrm

This document contains everything needed to submit the **Wyrm** programming language to [github-linguist/linguist](https://github.com/github-linguist/linguist).

---

## 1. Submission Overview

- **Language Name**: Wyrm
- **Primary Extension**: `.wyr`
- **Color**: `#6f3799` (Royal Wyrm Purple)
- **Type**: `programming`
- **TextMate Scope**: `source.wyrm`
- **Grammar Location**: [https://github.com/neofilisoft/wyrm/tree/master/extension](https://github.com/neofilisoft/wyrm/tree/master/extension) (or dedicated repository)
- **Grammar License**: MIT License
- **Sample Files License**: MIT License (written specifically for Wyrm and Linguist inclusion)

---

## 2. Changes for `languages.yml`

Add the following entry in alphabetical order under `W` in `lib/linguist/languages.yml`:

```yaml
Wyrm:
  type: programming
  color: "#6f3799"
  extensions:
    - ".wyr"
  tm_scope: source.wyrm
  ace_mode: text
  codemirror_mode: clike
  codemirror_mime_type: text/x-wyrm
  language_id: 87463920
```

*(Note: Run `script/update-ids` inside your cloned Linguist fork to verify the language ID)*

---

## 3. Pull Request Title & Body Template

Use the exact text below when opening the Pull Request on [github-linguist/linguist](https://github.com/github-linguist/linguist/pulls):

### PR Title:
```text
Add support for Wyrm language (.wyr)
```

### PR Body:
```markdown
### Summary
This pull request adds language detection and syntax highlighting support for **Wyrm** (`.wyr`), a statically typed, native systems programming language featuring LLVM backend code generation, memory ownership semantics, and standard graphics/game library capabilities.

- Language website / repository: https://github.com/neofilisoft/wyrm
- Documentation: https://github.com/neofilisoft/wyrm/blob/master/docs/Docs.md

### Checklist
- [x] Language entry added to `lib/linguist/languages.yml`
- [x] Syntax grammar linked in `vendor/README.md` and imported with `script/add-grammar`
- [x] Representative real-world samples included in `samples/Wyrm/`
- [x] Samples are covered by the MIT License
- [x] Ran `bundle exec rake test` to ensure all tests pass cleanly

### Search Query & Evidence of Usage
Search query demonstrating public `.wyr` usage across repositories on GitHub:
- `path:*.wyr` / `extension:wyr`

### Sample File Licenses
The sample files in `samples/Wyrm/` are provided under the MIT License by the Wyrm language project.
```

---

## 4. Step-by-Step Instructions to Submit

1. **Fork Linguist**:
   Visit [https://github.com/github-linguist/linguist](https://github.com/github-linguist/linguist) and click **Fork**.

2. **Add Files to Fork**:
   - Paste the YAML snippet into `lib/linguist/languages.yml`
   - Copy `tools/linguist/samples/Wyrm/` to `samples/Wyrm/`
   - Import the TextMate grammar:
     ```bash
     bundle exec script/add-grammar https://github.com/neofilisoft/wyrm
     ```

3. **Run Tests**:
   ```bash
   bundle exec rake test
   ```

4. **Submit Pull Request**:
   Open a PR using the title and description template above.
