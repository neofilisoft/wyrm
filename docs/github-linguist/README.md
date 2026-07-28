# GitHub Linguist registration for Wyrm

Repository `.gitattributes` marks `*.wyr` as `linguist-language=Wyrm`, but GitHub will only display Wyrm in the Languages sidebar after Wyrm is registered in github-linguist upstream.

Suggested upstream language entry:

```yaml
Wyrm:
  type: programming
  color: "#B10C1A"
  extensions:
  - ".wyr"
  tm_scope: source.wyr
  ace_mode: text
  codemirror_mode: clike
  codemirror_mime_type: text/x-wyrm
  language_id: 987654321
```

The VS Code TextMate grammar lives at `extension/syntaxes/wyrm.tmLanguage.json` and uses `source.wyr`.