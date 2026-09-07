# GitHub Linguist Submission Guide & Pull Request Template for Wyrm

This document contains everything needed to submit the **Wyrm** programming language to [github-linguist/linguist](https://github.com/github-linguist/linguist).

---

## 1. Submission Overview

- **Language Name**: Wyrm
- **Primary Extension**: `.wyr`
- **Color**: `#6f3799` (Royal Wyrm Purple)
- **Type**: `programming`
- **TextMate Scope**: `source.wyr` (matches the scopeName shipped in the VSCode extension grammar - do not change one without the other)
- **Grammar Location**: https://github.com/neofilisoft/wyrm/tree/master/extension/syntaxes/wyrm.tmLanguage.json
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
  tm_scope: source.wyr
  ace_mode: text
  codemirror_mode: clike
  codemirror_mime_type: text/x-wyrm
  language_id: <RUN script/update-ids TO GENERATE - see step 4>
```

**Before opening the PR**: clone your Linguist fork, add this entry with a placeholder ID, then run `script/update-ids` from the repo root. It rewrites the `language_id` to the next free, non-colliding value automatically. Never hand-pick or guess this number - a collision will fail CI and can silently corrupt another language's stats if merged.

---

## 3. Pull Request Title & Body Template

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

### Evidence of Usage
<!--
Fill this in honestly with what actually exists at submission time e.g. a list of
public, non-fork repositories using .wyr, or an explanation that Wyrm is a new
language and this submission is the reference implementation's own usage.
Linguist maintainers weigh new-language submissions against adoption; do not
inflate this section with search queries that don't yet return real results.
-->
```

---

## 4. Step-by-Step Instructions to Submit

1. **Fork Linguist**:
   Visit https://github.com/github-linguist/linguist and click **Fork**.

2. **Add Files to Fork**:
   - Paste the YAML snippet into `lib/linguist/languages.yml` (with placeholder `language_id`)
   - Copy `tools/linguist/samples/Wyrm/` to `samples/Wyrm/`
   - Import the TextMate grammar as a proper vendored grammar submodule:
     ```bash
     bundle exec script/add-grammar https://github.com/neofilisoft/wyrm
     ```
     This also adds the required entry to `vendor/README.md`.

3. **Generate a real language ID**:
   ```bash
   script/update-ids
   ```
   Commit the resulting change to `languages.yml`.

4. **Run Tests**:
   ```bash
   bundle exec rake test
   ```
   Fix any classifier ambiguity failures - this is expected the first time and usually means more/varied samples are needed (see `samples/Wyrm/`).

5. **Submit Pull Request**:
   Open a PR using the title and description template above, with the Evidence of Usage section filled in truthfully.