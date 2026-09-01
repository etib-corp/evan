# Versioning & Support Policy

This document describes how Evan is versioned and supported.

## Versioning

Evan follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
(`MAJOR.MINOR.PATCH`):

- **MAJOR** — incompatible API changes.
- **MINOR** — backwards-compatible feature additions.
- **PATCH** — backwards-compatible bug fixes.

The current version is declared in `CMakeLists.txt` and mirrored in
`CHANGELOG.md`.

## Long-Term Support (LTS)

- The **1.x** line is the current supported series.
- Bug fixes and security patches are backported to the latest 1.x release.
- New features land in the next minor release.

## Deprecation Window

- Deprecated APIs are documented in the API reference and the `CHANGELOG.md`.
- A deprecated API is removed **no earlier than two minor releases** after it
  is marked deprecated.
- Each release ships a **migration guide** that documents breaking changes and
  how to update.

## Communication

- Every release is announced in `CHANGELOG.md`.
- Breaking changes are always accompanied by a migration guide entry.
- Security issues are handled per `SECURITY.md`.

## Release Cadence

Releases are cut when the maintainers determine the `main` branch is stable
and the release checklist is complete. There is no fixed calendar schedule.
