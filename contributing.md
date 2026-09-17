# Contributing to VSTProfiler

Contributions to code, tests, documentation, and compatibility testing are welcome. Follow the [code of conduct](CODE_OF_CONDUCT.md). For suspected vulnerabilities, use the [security policy](SECURITY.md) instead of a public bug report.

## Before you start

Check existing issues and pull requests to avoid duplicate work. The [project board](https://github.com/users/QuentinStoll/projects/5) tracks ongoing work; new proposals are welcome too.

Discuss substantial features, breaking changes, and architectural changes in an issue or an early draft pull request. Keep each contribution focused so it is practical to review and test.

## Local setup and project structure

Follow the [README installation instructions](README.md#installation) and [usage examples](README.md#usage).

- `Source/Include/`: headers, shared interfaces, and styling.
- `Source/Src/`: processor, editor, views, and UI modules.
- `Source/Assets/`: images and other interface assets.
- `Tests/`: automated tests and test support.
- `cmake/`: dependencies, presets, and compiler configuration.
- `.github/`: workflows and contribution templates.

## Branches and pull request titles

The branch-name workflow accepts `feat/`, `fix/`, `docs/`, `poc/`, or `chore/`, followed by lowercase letters, digits, dots, underscores, or hyphens. For example:

```sh
git checkout -b docs/improve-setup
```

The workflow also allows promotion from `Dev` to `Main`. Follow the existing change's target branch when updating an open PR; discuss the appropriate target with maintainers when uncertain.

PR titles must use a type accepted by CI: `feat`, `fix`, `docs`, `chore`, or `refactor`. For example: `docs: clarify Windows setup`. Write concise commits that explain the change; the current title-check workflow validates PR titles, despite its commit-check filename.

## Code style and validation

Follow the existing C++ style and the repository's `.clang-format` configuration. CI runs `clang-format --dry-run --Werror` on C/C++ files under `Source` and `Tests`.

Run the automated tests before submitting code changes:

```powershell
# Windows
.\install.bat test
```

```sh
# Linux
bash ./install.sh test
```

Verify that affected application and plugin targets still build. Manually check affected profiling/model-loading, UI, and performance behavior as appropriate. Include relevant platform, host, audio-device, sample-rate, and buffer-size details in your report.

Add or update tests for meaningful behavior changes, and update documentation when installation or observable behavior changes. For documentation-only changes, verify links, examples, and claims against the repository; explain why audio tests or builds were not needed.

## Pull requests and review

Use the repository's pull request template. The PR description must be **human-written**, explaining what changed, why, and how someone else can reproduce your validation. State which checks passed and disclose checks not run.

Your contribution must align with the project's [GPLv3-or-later notice](LICENCE.md), and you must have the right to contribute any included code or assets.

AI-assisted contributions are welcome only when a qualified human reviewer verifies, tests, and understands the code. The person submitting the PR must be able to explain its correctness and maintainability.

To open an early draft:

1. Push your branch to your fork.
2. Open a pull request against the project using your branch.
3. Select **Create draft pull request** from the creation dropdown.
4. Address feedback, then mark it **Ready for review** when complete.

At least one founder-team member must approve the change before it can be merged. Maintainers may request revisions or decline changes that do not fit the project's direction or quality expectations. Missing information may result in a PR being closed.
