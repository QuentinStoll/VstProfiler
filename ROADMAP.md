# Roadmap

This is a proposed roadmap based on the repository's current limitations, not a committed release schedule. Maintainers should confirm priorities through the [project board](https://github.com/users/QuentinStoll/projects/5) and [issues](https://github.com/QuentinStoll/VstProfiler/issues).

## Now

- Validate the documented Windows and Linux setup from clean checkouts.
- Establish private security and conduct reporting channels.
- Verify model-loading, IR-loading, and profile workflows in supported hosts.
- Keep critical automated checks passing and document reproducible failures.

## Next

- Connect preset format selections to the plugin target consistently.
- Separate platform-specific dependencies and validate macOS/AU builds.
- Document a tested model compatibility matrix and asset-sharing workflow.
- Validate the next release's binaries and document its compatibility and release notes. The repository already has version tags; this roadmap does not reset that history.

## Later

- Explore an amplifier capture and model-training workflow.
- Evaluate audio quality, latency, CPU use, and broader host compatibility.
- Expand usage examples and contributor-facing DSP documentation.

Propose changes through an issue before starting substantial work. Items move between horizons as validation and contributor availability change.
