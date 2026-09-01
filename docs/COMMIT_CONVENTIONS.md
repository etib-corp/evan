# Commit Conventions

Evan uses [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/)
to keep history readable and useful.

## Commit Format

```text
<type>(optional scope): <description>
```

## Allowed Types

- `feat`: New feature.
- `fix`: Bug fix.
- `docs`: Documentation changes.
- `style`: Formatting or style-only changes.
- `refactor`: Internal code changes without feature or fix.
- `test`: Added or updated tests.
- `chore`: Tooling, dependencies, or build maintenance.
- `optim`: Performance improvements.

## Scope

The scope is optional and indicates the affected area, for example:

- `core`
- `render`
- `platform`

## Description

- Use imperative mood.
- Keep it concise.
- Target 50 characters or fewer.

## Good Examples

- `feat(render): add new material types`
- `fix(platform): prevent crash on startup`
- `docs: update installation instructions`
- `refactor(core): simplify frame loop`

## Avoid

- `update: made some changes`
- `feat: added new stuff`
- `refactor: improvements`

These messages are too vague or do not follow the expected type system.

## Git Usage Tips

- Use `git commit --amend` for quick fixes to the latest commit.
- Use `git rebase -i HEAD~n` to reorder or squash local commits.
- Use `git log --oneline` to review your branch history.
- Pull latest changes before starting work.
- Prefer `git pull --rebase` for a linear history.

If multiple contributors share a branch, avoid history rewriting commands such
as force-push unless explicitly agreed by the team.
