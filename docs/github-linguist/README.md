# GitHub Linguist Registration For Wyrm

Repository attributes:
*.wyr linguist-language=Wyrm
*.wyr linguist-detectable=true

Upstream GitHub Linguist language entry:
Wyrm:
  type: programming
  color: '#5B256B'
  extensions:
  - '.wyr'
  tm_scope: source.wyr
  ace_mode: text
  codemirror_mode: clike
  codemirror_mime_type: text/x-wyrm
  language_id: 987654321

GitHub can display Wyrm and color #5B256B after Wyrm is registered in github-linguist upstream.
The repository keeps .wyr detectable and excludes tooling files so the intended order is Wyrm first, C second, and minor tooling languages after that.

The VS Code TextMate grammar lives at extension/syntaxes/wyrm.tmLanguage.json and uses source.wyr.
