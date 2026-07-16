# Contributing instruction

## **MODALITY**

### contributing limits
You can only merge your branch once its validated by at least 1 member of the founder team.
Its better to follow the ongoing issues in the Github Project: https://github.com/users/QuentinStoll/projects/5 but you can propose your own new features.

### install inctruction
To install the project please just follow the README.md file Installation: instructions https://github.com/QuentinStoll/Profiler?tab=readme-ov-file#installation

### repository structure
The sources of the repository are in the folder ./Source, then you have the include and src folders. The other folders are useless for you exept the cmake folder if you need to add new files of build method.

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

### PR Requirements
When submitting a pull request, ensure:

* **Clear description:** Explain what the PR achieves and why the change is needed with **HUMAN generated** text. Also cleary explain how you tested it and how to replicate those test.
* **Testing:** Verify main features that can be affected by your changes:
  * Profiling
  * User interface
  * Efficiency
* **License compliance:** Your code must align with VstProfiler license
* **Build:** You have to verify that all the parts of the project still build
* **Documentation:** If needed, update the documentation or create your own

> Important: Prefer opening small PR, focused on specific functionality that directly add value. Avoid to change multiple unrelated functionality to facilitate test.

> Contributions generated with AI are welcome, provided that a qualified human reviewer verifies, tests, and understands the code. AI tools can assist in development, but all pull requests must be submitted by someone capable of ensuring correctness and maintainability. 

> Missing requirements may lead to requests for additional information and, if not provided, the PR may be closed. Regardless of the above, the final decision to merge a pull request is at the maintainer’s discretion.

### How to Open a Draft PR
1. Push your branch to your fork
2. Go to the main repository and click **"New Pull Request"**
3. Select your fork and branch
4. Click the dropdown arrow next to **"Create Pull Request"**
5. Select **"Create Draft Pull Request"**
6. Once ready for review, click **"Ready for review"** in the PR

This workflow helps avoid spending time on PRs that may not align with project goals.
