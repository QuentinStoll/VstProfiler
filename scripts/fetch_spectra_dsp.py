#!/usr/bin/env python3
"""Download the latest SpectraDsp release asset for this runner.

Prints GitHub Actions outputs on stdout: library and version.
Logs go to stderr. A missing token or release is fatal only with --require.
"""

import argparse
import json
import os
import platform
import sys
import urllib.error
import urllib.request

REPO = "QuentinStoll/SpectraDspDLL"
API = f"https://api.github.com/repos/{REPO}/releases/latest"


def asset_for_host():
    system = platform.system()
    machine = platform.machine().lower()
    if system == "Windows":
        if machine in ("arm64", "aarch64"):
            return "SpectraDsp-windows-arm64.dll", "SpectraDsp.dll"
        return "SpectraDsp-windows-x64.dll", "SpectraDsp.dll"
    if system == "Darwin":
        if machine in ("arm64", "aarch64"):
            return "SpectraDsp-macos-arm64.dylib", "libSpectraDsp.dylib"
        return "SpectraDsp-macos-x64.dylib", "libSpectraDsp.dylib"
    if machine in ("x86_64", "amd64"):
        return "SpectraDsp-linux-x64.so", "libSpectraDsp.so"
    return "SpectraDsp-linux-arm64.so", "libSpectraDsp.so"


def github_json(url, token):
    request = urllib.request.Request(
        url,
        headers={
            "Accept": "application/vnd.github+json",
            "Authorization": f"Bearer {token}",
            "X-GitHub-Api-Version": "2022-11-28",
            "User-Agent": "vstprofiler-spectra-fetch",
        },
    )
    with urllib.request.urlopen(request) as response:
        return json.load(response)


def download(url, token, destination):
    request = urllib.request.Request(
        url,
        headers={
            "Accept": "application/octet-stream",
            "Authorization": f"Bearer {token}",
            "X-GitHub-Api-Version": "2022-11-28",
            "User-Agent": "vstprofiler-spectra-fetch",
        },
    )
    with urllib.request.urlopen(request) as response, open(destination, "wb") as output:
        output.write(response.read())


def emit(key, value):
    print(f"{key}={value}")


def skip(message):
    print(message, file=sys.stderr)
    emit("library", "")
    emit("version", "")
    return 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dest", required=True)
    parser.add_argument("--require", action="store_true")
    args = parser.parse_args()

    token = os.environ.get("SPECTRA_DSP_TOKEN", "").strip()
    asset_name, runtime_name = asset_for_host()
    if not token:
        if args.require:
            print("SPECTRA_DSP_TOKEN is required to bundle SpectraDsp.", file=sys.stderr)
            return 1
        return skip("SpectraDsp token absent; link test and bundling skipped.")

    try:
        release = github_json(API, token)
    except urllib.error.HTTPError as error:
        print(f"Could not read the latest SpectraDsp release: HTTP {error.code}", file=sys.stderr)
        return 1 if args.require else skip("No SpectraDsp release is available yet.")

    asset = next((item for item in release.get("assets", []) if item.get("name") == asset_name), None)
    if asset is None:
        message = f"Latest SpectraDsp release {release.get('tag_name')} has no {asset_name}."
        print(message, file=sys.stderr)
        return 1 if args.require else skip(message)

    os.makedirs(args.dest, exist_ok=True)
    library = os.path.join(args.dest, runtime_name)
    download(asset["url"], token, library)
    version = release.get("tag_name", "")
    with open(os.path.join(args.dest, "SPECTRA_DSP_VERSION.txt"), "w", encoding="utf-8") as handle:
        handle.write(version + "\n")
    print(f"Fetched SpectraDsp {version} -> {library}", file=sys.stderr)
    emit("library", library)
    emit("version", version)
    return 0


if __name__ == "__main__":
    sys.exit(main())
