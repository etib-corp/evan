# Commit Message Guidelines (and git usage tips)

This project follows the [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) specification for commit messages. This helps maintain a clear and consistent history, making it easier to understand the changes made over time.

## Commit Message Structure

A commit message should be structured as follows:

```<type>[optional scope]: <description>```

### Type

The `type` indicates the nature of the change. Common types include:
- `feat`: A new feature
- `fix`: A bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, missing semicolons, etc.)
- `refactor`: Code changes that neither fix a bug nor add a feature
- `test`: Adding or updating tests
- `chore`: Changes to the build process or auxiliary tools and libraries
- `optim`: Performance improvements

### Scope

The `scope` is optional and can be used to specify the area of the codebase affected by the change.

It can be something like:
- `vk`: for changes related to the Vulkan renderer
- `xr`: for changes related to the XR module
- `core`: for changes related to the core functionality

### Description

The `description` is a brief summary of the change. It should be concise and to the point, ideally no more than 50 characters.

## Examples

- `feat(vk): add support for ray tracing`
- `fix(xr): resolve memory leak in XR session handling`
- `docs: update README with new installation instructions`
- `style: reformat toto.hpp using clang-format`
- `refactor(core): simplify the event handling logic`
- `test(vk): add unit tests for Vulkan initialization`
- `chore: update dependencies to latest versions`
- `optim(core): improve performance of the rendering loop`

## Additional Guidelines

- Use the imperative mood in the subject line (e.g., "fix" not "fixed" or "fixes").
- Limit the subject line to 50 characters.
- Separate the subject from the body with a blank line if you need to add more details.
- Use the body to explain what and why vs. how if necessary.

Do not commit something like:
- `fix: corrected a bug in the rendering engine` (too vague)
- `update: made some changes to the XR module` (not following the type conventions)
- `refactor: code improvements` (lacks detail)
- `feat: added new stuff` (not descriptive enough)

Try the most of the time to not duplicate commits.

## Git Usage Tips

- Use `git commit --amend` to modify the most recent commit if you forgot to include something or need to fix the message.
- Use `git rebase -i HEAD~n` (where n is the number of commits to go back) to edit, squash, or reorder commits in your branch before merging.
- Use `git log --oneline` to view a concise history of commits.
- Always pull the latest changes from the main branch before starting new work to minimize merge conflicts.
- Write clear and descriptive commit messages to help others (and yourself) understand the history of changes.

If multiple persons are working on the same branch,
try to avoid rewriting history
(e.g., using `git push --force`) unless
absolutely necessary and agreed upon by the team.
Also if you pull changes, while having local commits,
prefer `git pull --rebase` or
`git fetch; git rebase origin/your-branch`
to keep a linear history.

Following these guidelines will help maintain a clean and understandable commit history, making collaboration easier for everyone involved in the project.

> Thank you for your contributions !