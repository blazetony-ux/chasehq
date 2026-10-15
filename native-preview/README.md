# RC5 Native Preview — offline visual POC

Windows x64: run `dist/ChaseHQ-Native-Preview.exe` (compiled native Win32 GUI via Go). No ROMs, Go runtime or SDL installation required.

This executable shows **synthetic** sprite and road-test scenes, not real game graphics or a live hybrid emulator. It is a tangible native-model progress viewer, not the eventual playable client. The real RC5 C++ native sprite and road candidates are compiled into `chq_runtime` and validated by CTest `native_candidates_tests`; the GUI currently embeds equivalent, independently compiled Go test algorithms and is NOT connected to emulator execution. No CPU routine is replaced or skipped. See `docs/RC5_NATIVE_PREVIEW_AND_SHADOW.md`.

Rebuild preview with `cd native-preview && go test ./... && GOOS=windows GOARCH=amd64 CGO_ENABLED=0 go build -trimpath -ldflags='-H=windowsgui -s -w' -o dist/ChaseHQ-Native-Preview.exe .`.
Windows startup still requires qualification on Windows.
