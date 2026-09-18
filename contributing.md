# Contributing to VSTProfiler

Contributions to code, tests, documentation, and compatibility testing are welcome. Follow the [code of conduct](CODE_OF_CONDUCT.md). For suspected vulnerabilities, use the [security policy](SECURITY.md) instead of a public bug report.

## Before you start

Check existing issues and pull requests to avoid duplicate work. The [project board](https://github.com/users/QuentinStoll/projects/5) tracks ongoing work; new proposals are welcome too.

Discuss substantial features, breaking changes, and architectural changes in an issue or an early draft pull request. Keep each contribution focused so it is practical to review and test.

## 🌿 Branch Naming Convention

To keep the repository organized and allow our automated workflows to pass, all branches must follow a specific naming convention.

### 🏗 Structure
The branch name must follow this pattern:  
`type/short-description`

### 📋 Allowed Types
| Type | Description |
| :--- | :--- |
| `feat/` | A new feature or significant evolution |
| `fix/` | A bug fix |
| `docs/` | Documentation changes only (ADR, Research, Wiki) |
| `poc/` | Proof of Concept, research, or technical experiments |
| `chore/` | Maintenance tasks, configuration, or dependencies update |

### 🛠 Rules & Constraints
* **Lowercase only:** All characters must be lowercase.
* **Separators:** Use hyphens `-`, underscores `_`, or dots `.` to separate words.
* **No Special Characters:** Do not use spaces, `@`, `!`, or accented characters.
* **Example of a valid branch:** `feat/add-user-auth` or `docs/update-adr-001`

## 💬 Commit Message Convention

We follow the **Conventional Commits** specification. This ensures a readable project history and allows for automated changelog generation.

### 🏗 Format
Every commit message must follow this structure:  
`type(scope): description`

* **type**: The intent of the change (see list below).
* **scope**: (Optional) The part of the project affected (e.g., `api`, `auth`, `ui`).
* **description**: A short, imperative summary of the change.

### 📋 Allowed Types
| Type | Use Case |
| :--- | :--- |
| `feat` | A new feature for the user |
| `fix` | A bug fix |
| `docs` | Changes to documentation (ADR, Research, Wiki) |
| `style` | Formatting, missing semi-colons, etc. (no code change) |
| `refactor` | Refactoring code that neither fixes a bug nor adds a feature |
| `perf` | Code changes that improve performance |
| `test` | Adding or correcting tests |
| `chore` | Updates to build tasks, package managers, or CI/CD config |

### 🛠 Rules
* **No Capitalization:** The type must be lowercase (e.g., `feat:`, not `Feat:`).
* **Space Required:** There must be a space after the colon (`type: description`).
* **No Period:** Do not end the description with a period.
* **Be Concise:** Keep the description under 50 characters if possible.

**Examples:**
- ✅ `feat(api): add endpoint for user profile`
- ✅ `fix: resolve memory leak in data parser`
- ✅ `docs: add research note about mermaid.js`

## **PR**
### Before You Start
1. **Check existing PRs and issues** to avoid duplicate work
2. **Open a Draft PR early** for significant changes to discuss your approach and get feedback before investing too much time
   - When creating a PR on GitHub, click the dropdown next to "Create Pull Request" and select **"Create Draft Pull Request"**
   - This gives visibility to other contributors and maintainers can provide early guidance
3. **Discuss breaking changes** or major architectural decisions in an issue or draft PR first

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
