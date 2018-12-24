# Threaded Wikipedia Importance Analyzer

[![GitHub Issues](https://img.shields.io/github/issues/David-Bauman/Multi-Threaded-Wikipiedia-Importance.svg)](https://github.com/David-Bauman/Multi-Threaded-Wikipiedia-Importance/issues)
![Contributions welcome](https://img.shields.io/badge/contributions-welcome-green.svg)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

## What is it?

This analyzer is a C++, multi-threaded implementation of [Wikipedia Importance Analyzer](https://github.com/David-Bauman/Wikipedia-Importance). It currently does the same analysis as the Python version, has the same capabilities (such as pausing), and includes a file to make the data human readable. The major difference is the addition of a compile step.

## Compiling

Both programs, `creatingimportance` and `makeimportanceusable`, are C++ files. Use your favorite compiler and be sure to link the thread and curl libraries! Included as the first line in each file is a suggested compilation command.

## Contributing

If you like the project, I'd love to have your help improving it. Contributions don't have to just be code though. It could also take the form of

- Opening issues on bugs you find or new features you'd like to see
- Joining discussion on issues and pull requests
- Discussing cool extensions of the data the current program gathers or interesting new ways to analyze it

#### Workflow

0. (Optional) Open an issue and describe what you will be trying to fix: a specific bug or a new feature.
1. Clone the repo to your local machine.
2. Create a new branch for your feature, commit changes to it, and push it to origin.
3. Open a pull request with a clear description of the changes.

## To Do

- Handle more threads
- Allow for user choice of starting pages (read from specific file for instance)
- Optimize code for those micro second gains!

