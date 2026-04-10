# ADR 002: Standardization of Git Branch Naming Convention

* **Status:** Accepted
* **Date:** 2026-04-10
* **Author:** @LeCentenaire
* **Deciders:** @LaurntZhang, @RychlewL, @LeCentenaire

## 1. Context & Problem Statement
The project initially used inconsistent and heterogeneous branch names (e.g., `CICD`, `PerfLog`, `patch-1`, `Liscence`, `ArchitectureDecisionRecords`). This lack of convention caused several issues:
* Difficulty in identifying the purpose of a branch at a glance.
* Inability to group branches logically in the GitHub interface.
* Challenges in automating CI/CD workflows based on branch types.
* Presence of typos in branch names (e.g., `Liscence`).

## 2. Considered Options
* **Option 1: No Convention (Status Quo)** - Developers name branches freely.
* **Option 2: Structured Naming Convention** - Adopting a `type/description` format based on industry standards (Git Flow/Conventional Commits).

## 3. Comparison Matrix (Research)
| Criteria | Option 1: No Convention | Option 2: Structured Convention |
| :--- | :--- | :--- |
| **Pros** | No setup effort. | Instant clarity, automatic grouping. |
| **Cons** | Cluttered repo, hard to automate. | Requires manual renaming of existing branches. |
| **Learning Curve** | Low | Low (simple prefix system) |
| **Project Fit** | Risky for collaboration. | Good (Professional & Scalable) |

## 4. Decision Outcome
Chosen option: **Option 2: Structured Naming Convention**, because:
1. It allows GitHub to group branches into virtual folders (e.g., `feat/`, `docs/`, `fix/`).
2. It enables specific CI/CD triggers (e.g., running the linter only on `feat/` and `fix/` branches).
3. It ensures the repository remains organized as the team and the codebase grow.
4. It fixes all historical naming errors.

## 5. Consequences
* **Positive:** Improved repository legibility and more professional project structure.
* **Negative:** All team members must prune their local branches and track the new remote names.
* **Technical:** The `branch-name-check.yml` workflow was implemented to enforce this rule on all future Pull Requests.

---
## Proof of Concept (PoC)
* **Pull Request:** N/A (Direct remote renaming performed)
* **Branch Mapping Table:**
    * `ArchitectureDecisionRecords` → **`feat/docs-and-architecture-setup`**
    * `CICD` → **`feat/ci-linter-setup`**
    * `ImproveAmpCapture` → **`refactor/amp-capture-optimization`**
    * `IRGenerator` → **`feat/ir-generator-module`**
    * `SavingAndConfigFiles` → **`chore/migrate-projucer-to-cmake`**
    * `Liscence` → **`docs/license-management`**
    * `PerfLog` → **`feat/performance-logger`**
    * `FixWindowsCmake` → **`fix/windows-cmake-build`**
