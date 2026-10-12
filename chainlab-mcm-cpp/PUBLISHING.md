# Publish the C++ project to your GitHub account

This C++ version is a terminal application. GitHub Pages cannot run the compiled terminal UI as an interactive website, so publish the source repository and README; the included GitHub Actions workflow will build the program and run its tests automatically.

## 1. Create a repository

1. Sign in to GitHub and open <https://github.com/new>.
2. Name the repository `chainlab-mcm-cpp`.
3. Choose the visibility required by your course.
4. Create it empty (do not add a README, license, or `.gitignore`, as these are already included here).

## 2. Push the source

Extract the project ZIP and open a terminal in the extracted `chainlab-mcm-cpp` directory:

```bash
git init
git add .
git commit -m "Add C++ Matrix Chain Multiplication logic animator"
git branch -M main
git remote add origin https://github.com/YOUR-USERNAME/chainlab-mcm-cpp.git
git push -u origin main
```

Replace `YOUR-USERNAME` with your own GitHub username. Complete GitHub's sign-in prompt if asked.

## 3. Verify the automated build

Open the repository's **Actions** tab. The `C++ Build and Tests` workflow uses CMake to build the application and runs CTest. Wait for the workflow to finish successfully. You can run the same checks locally:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 4. Put the real repository link in the paper

The report currently contains this placeholder:

`https://github.com/<YOUR-USERNAME>/chainlab-mcm-cpp`

Replace it with your real URL, for example `https://github.com/your-actual-account/chainlab-mcm-cpp`. Do not submit the placeholder. Also fill in your author name, registration number, and course code.

## 5. Final pre-submission check

Make sure the repository includes `src/`, `tests/`, `CMakeLists.txt`, `README.md`, and `LICENSE`. Verify that the README commands work on your computer and that the repository is visible to whoever will grade it.
