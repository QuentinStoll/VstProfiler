<a id="readme-top"></a>

<!-- PROJECT SHIELDS -->
[![Epitech](https://img.shields.io/badge/Epitech-Paris-blue)](https://www.epitech.eu/fr/)
[![JUCE](https://img.shields.io/badge/JUCE-8.0.12+-brightgreen)](https://juce.com/)
[![C++20](https://img.shields.io/badge/C++-20-blue)](https://en.cppreference.com/w/cpp/20)



<!-- PROJECT LOGO -->
<br />
<div align="center">
<!--
  <a href="https://github.com/QuentinStoll/Profiler">
    <img src="images/logo.png" alt="Logo" width="80" height="80">
  </a>
-->
<h3 align="center">VSTProfiler - Guitar Amplifier Simulator</h3>
  <p align="center">
    VSTProfiler is an open-source audio plugin (VST3 / AU / Standalone) designed to accurately model and reproduce the sound and dynamic behavior of legendary guitar amplifiers using impulse responses, circuit modeling, and lightweight machine learning.
    <br />
    <a href="https://github.com/QuentinStoll/Profiler/wiki"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="#getting-started">Getting Started</a>
    &middot;
    <a href="#contributing">Contributing</a>
    &middot;
    <a href="https://github.com/QuentinStoll/Profiler/issues/new?labels=enhancement">Request Feature</a>
    &middot;
    <a href="https://github.com/QuentinStoll/Profiler/issues/new?labels=bug">Report Bug</a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

**EIP 2025–2026 — Technical Track**
Final-year engineering project (started in 4th year) developed by 4 students from **Epitech Paris**.

The goal of VSTProfiler is to provide guitarists with a **free, open-source, and highly customizable** tool capable of “copying” the tone and feel of real amplifiers, with audio quality comparable to commercial solutions.

### Key Features
- Impulse Response capture and loading (custom capture tools)
- Non-linear tube modeling (12AX7, EL34, etc.)
- Speaker cabinet and speaker emulation
- Oversampling up to **16×** for zero aliasing
- Modern, skinnable JUCE-based UI
- Cross-platform support (Windows, macOS Apple Silicon, Linux — target)

<p align="right">(<a href="#readme-top">back to top</a>)</p>



### Built With

* [**C++20**](https://en.cppreference.com/w/cpp/20)
* [**JUCE 8.0+**](https://juce.com/)
* [**spdlog**](https://github.com/gabime/spdlog)

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

### Prerequisites

**for windows:**
```
cmake
...
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

choco install pkgconfiglite

choco install curl
```

**for linux**
```
cmake
...
to be documented
```


<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Installation

1. Clone the repository
  ```sh
  git clone https://github.com/QuentinStoll/Profiler.git
  ```
2. use the install script
  >windows:
  ```batch
  .\install.bat
  ```

  >linux:
  ```sh
  ./install.sh
  ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- USAGE -->
## Usage

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTRIBUTING -->
## Contributing
see [CONTRIBUTING.md](CONTRIBUTING.md)
<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- LICENSE -->
## License
see [LICENSE.md](LICENSE.md)
<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ROADMAP -->
