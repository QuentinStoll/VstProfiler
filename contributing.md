# Contributing to VSTProfiler

Contributions to code, tests, documentation, and compatibility testing are welcome. Please follow the [code of conduct](CODE_OF_CONDUCT.md). For suspected vulnerabilities, follow the [security policy](SECURITY.md) instead of opening a public bug report.

## Contents

- [Plan your contribution](#plan-your-contribution)
- [Set up the project](#set-up-the-project)
- [Create a branch](#create-a-branch)
- [Make and validate changes](#make-and-validate-changes)
- [Write commit messages](#write-commit-messages)
- [Submit a pull request](#submit-a-pull-request)
- [Review and merge](#review-and-merge)

## Plan your contribution

Check existing issues and pull requests to avoid duplicate work. The [project board](https://github.com/users/QuentinStoll/projects/5) tracks ongoing priorities; new proposals are welcome too.

Discuss significant features, breaking changes, and architectural decisions in an issue or an early draft pull request. Keep each contribution focused so it is practical to review and test.

## Set up the project

Follow the [README installation instructions](README.md#installation) for prerequisites and the [usage examples](README.md#usage) to try the application.

From the repository root, configure and build with the script for your platform:

**Windows**

```powershell
.\install.bat all default
```

**Linux**

```sh
bash ./install.sh all default
```

### Repository structure

| Directory | Contents |
| --- | --- |
| `Source/Include/` | Headers, shared interfaces, and styling |
| `Source/Src/` | Audio processor, editor, views, and UI modules |
| `Source/Assets/` | Images and other interface assets |
| `Tests/` | Automated tests and test support |
| `cmake/` | Dependencies, build presets, and compiler configuration |
| `.github/` | Workflows and contribution templates |

## Create a branch

Use the format `type/short-description`.

| Prefix | Purpose |
| --- | --- |
| `feat/` | New features or significant improvements |
| `fix/` | Bug fixes |
| `docs/` | Documentation, research notes, or architecture decisions |
| `poc/` | Proofs of concept and technical experiments |
| `chore/` | Maintenance, configuration, or dependency updates |

After the prefix, use only lowercase letters, digits, hyphens, underscores, or dots. Do not use spaces, uppercase letters, accented characters, or other special characters.

```sh
git checkout -b docs/improve-setup
```

Other examples include `feat/profile-export` and `fix/amp-loading`.

The [branch-name workflow](.github/workflows/check-branch.yml) enforces these rules and also allows promotion from `Dev` to `Main`. When updating an existing PR, keep its target branch; discuss the appropriate target with maintainers when uncertain.

## Make and validate changes

Follow the existing C++ style and the [.clang-format](.clang-format) configuration. CI checks C/C++ files under `Source` and `Tests` with `clang-format --dry-run --Werror`.

Add or update tests for meaningful behavior changes. Update documentation whenever installation or observable behavior changes, and add examples where they help explain a feature.

### Automated tests

**Windows**

```powershell
.\install.bat test
```

**Linux**

```sh
bash ./install.sh test
```

### Build and manual checks

Verify that affected application and plugin targets still build. Manually check affected profiling/model-loading, UI, and performance behavior. Record the commands, results, and steps needed to reproduce your checks, including relevant platform, host, audio-device, sample-rate, and buffer-size details.

For documentation-only changes, verify links, examples, and claims against the repository. Explain any build or audio checks you did not run and why.

Your contribution must comply with the project's [GPLv3-or-later notice](LICENCE.md), and you must have the right to contribute any included code or assets.

## Write commit messages

Use Conventional Commits with the format `type(scope): description`. The scope is optional and identifies an affected area such as `dsp`, `ui`, or `build`.

| Type | Purpose |
| --- | --- |
| `feat` | New functionality |
| `fix` | Bug fixes |
| `docs` | Documentation changes |
| `style` | Formatting without behavior changes |
| `refactor` | Code restructuring without a feature or bug fix |
| `perf` | Performance improvements |
| `test` | New or corrected tests |
| `chore` | Build, dependency, or CI maintenance |

Use a lowercase type and a space after the colon. Write a concise imperative description without a final period, keeping it under 50 characters where practical.

```text
feat(ui): add profile export controls
fix(dsp): correct noise gate bypass
docs: clarify Windows installation
```

## Submit a pull request

1. Push your branch to your fork or the repository you have access to.
2. Open a pull request against the project and select the appropriate source and target branches.
3. Use the [pull request template](.github/PULL_REQUEST_TEMPLATE.md) to describe the change and its validation.
4. For work in progress, select **Create draft pull request** from the creation dropdown.
5. Address feedback, then mark the PR **Ready for review** when complete.

### PR title

Use the format `type(scope): description`, with one of the types accepted by CI: **`feat`, `fix`, `docs`, `chore`, or `refactor`**. For example: `docs: clarify Windows setup`.

The [PR-title workflow](.github/workflows/check-commits.yml) checks the PR title, despite its commit-check filename. It does not validate individual commit messages. The broader list above applies to commit messages; PR titles use this narrower list.

### Description and AI-assisted contributions

The PR description must be **human-written**. Explain what changed, why it was needed, and how reviewers can reproduce your validation. State which checks passed and disclose checks that were not run.

AI-assisted contributions are welcome only when a qualified human reviewer verifies, tests, and understands the code. The person submitting the PR must be able to explain its correctness and maintainability.

## Review and merge

At least one founder-team member must approve the change before it can be merged. Maintainers may request revisions or decline changes that do not fit the project's direction or quality expectations.

Missing information may result in a PR being closed. The final decision to merge rests with the maintainers.
