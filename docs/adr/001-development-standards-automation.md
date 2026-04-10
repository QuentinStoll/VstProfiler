# ADR 001: Development Standards and Workflow Automation

* **Status:** Accepted
* **Date:** 2026-04-10
* **Author:** @LeCentenaire
* **Deciders:** @LeCentenaire

## 1. Context & Problem Statement
As the project grows and opens to contributors, maintaining a clean git history and a consistent branch structure becomes difficult. Manually checking branch names and commit formats is time-consuming and prone to human error. We need an automated way to enforce these standards to ensure project maintainability.

## 2. Considered Options
* **Option 1: Manual Review** - Trust contributors to follow the `CONTRIBUTING.md` rules and manually reject PRs that don't comply.
* **Option 2: Git Hooks (Husky)** - Enforce rules locally on the developer's machine before they can commit or push.
* **Option 3: GitHub Actions (CI)** - Enforce rules on the server side during the Pull Request process.

## 3. Comparison Matrix (Research)
| Criteria | Option 1 (Manual) | Option 2 (Husky) | Option 3 (Actions) |
| :--- | :--- | :--- | :--- |
| **Reliability** | Low (Human error) | Medium (Can be bypassed) | High (Server-side) |
| **Setup Ease** | Easy (No tech) | Medium (Needs Node.js) | Medium (YAML config) |
| **Feedback Loop** | Slow (During review) | Fast (Instant) | Medium (After Push) |
| **Strictness** | Weak | Strong | Mandatory |

## 4. Decision Outcome
Chosen option: **Combination of Option 3 (GitHub Actions)** and **Documentation**.

We decided to use GitHub Actions for its "Source of Truth" reliability. Even if a contributor doesn't have a local setup, the CI will act as a gatekeeper. 

Specifically, we implemented:
1. **Branch Naming Check:** A regex-based workflow to ensure branches start with `feat/`, `fix/`, `docs/`, `poc/`, or `chore/`.
2. **Commitlint:** To enforce the Conventional Commits specification.
3. **PR Template:** A mandatory checklist to ensure Research (Veille) and ADRs are completed.

## 5. Consequences
* **Positive:** Consistent git history, easier automated changelog generation, and professional-looking repository.
* **Negative:** Slightly higher barrier to entry for new contributors who must learn the naming conventions.
* **Technical:** Added `.github/workflows/` files and a `commitlint.config.js` at the root.

---
## Proof of Functioning (PoC)
* **Status:** Implemented and tested.
* **Result:** Branches not following the naming convention now correctly trigger a failure in the GitHub "Checks" tab, preventing the "Merge" button from being used.
* **Example PR:** [Link to your PR if available]
