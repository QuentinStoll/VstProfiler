# Contributing instruction

## **MODALITY**

### contributing limits
You can only merge your branch once its validated by at least 1 memeber of the founder team.
Its better to follow the ongoing issues in the Github Project: https://github.com/users/QuentinStoll/projects/5 but you can propose your own new features.

### install inctruction
To instal the project please just follow the README.md file Installation: instructions https://github.com/QuentinStoll/Profiler?tab=readme-ov-file#installation

### repository structure
The sources of the repository are in the folder ./Source, then you have the include and src folders. The other folders are useless for you exept the cmake forlder if you need to add new files of build method.

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
* **Testing:** Verify main features that can be afected by your changes:
  * Profiling
  * User interface
  * Efficiency
* **License compliance:** Your code must align with VstProfiler license
* **Build:** You have to verify that all the parts of the project still build
* **Documentation:** If needed, update the documentation or create your own

> Important: Prefear opening small PR, focused on specific functionality that directly add value. Avoid to change multiple unrelated functionality to facilitate test.

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
