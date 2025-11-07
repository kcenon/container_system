# GitHub Actions Workflow Template

This directory contains workflow templates that cannot be pushed directly by GitHub Apps due to permission restrictions.

## Automated Memory Pool Performance Benchmarks

**File:** `memory-pool-benchmarks.yml`

### Features
- Multi-platform benchmarks (Ubuntu 22.04, macOS ARM64)
- Automatic performance tracking and documentation updates
- JSON result parsing and analysis
- 90-day artifact retention
- PR comment integration

### Installation

To activate the workflow, simply copy it to the `.github/workflows/` directory:

```bash
# Create workflows directory if it doesn't exist
mkdir -p .github/workflows

# Copy the workflow file
cp .github-workflows-template/memory-pool-benchmarks.yml .github/workflows/

# Commit and push
git add .github/workflows/memory-pool-benchmarks.yml
git commit -m "ci: Add automated memory pool performance benchmarks workflow"
git push
```

### Manual Installation (Alternative)

If you prefer to review the workflow first:

```bash
# View the workflow content
cat .github-workflows-template/memory-pool-benchmarks.yml

# Copy when ready
cp .github-workflows-template/memory-pool-benchmarks.yml .github/workflows/
```

### What It Does

1. **Triggers automatically on:**
   - Push to `main` branch (when memory pool files change)
   - Pull requests to `main`
   - Manual workflow dispatch

2. **Runs benchmarks:**
   - Ubuntu 22.04 (x86_64, Clang)
   - macOS latest (ARM64, Apple Silicon)
   - 5 repetitions for statistical confidence

3. **Generates results:**
   - JSON format (Google Benchmark)
   - Performance metrics and speedup factors
   - Comparison with standard allocator

4. **Auto-updates documentation:**
   - Parses JSON results with Python
   - Calculates performance metrics
   - Updates `docs/MEMORY_POOL_PERFORMANCE.md`
   - Auto-commits with `[skip ci]` tag

5. **Stores artifacts:**
   - 90-day retention period
   - Historical comparison capability

### First Run

After installation, trigger the workflow manually:

1. Go to **Actions** → **Memory Pool Performance Benchmarks**
2. Click **Run workflow**
3. Select your branch
4. Click **Run workflow**

The results will be automatically added to `docs/MEMORY_POOL_PERFORMANCE.md`.

### Documentation

For complete system documentation, see:
- `docs/AUTOMATED_PERFORMANCE_TRACKING.md` - System architecture and usage
- `docs/MEMORY_POOL_PERFORMANCE.md` - Auto-updated performance results
- `docs/TESTING.md` - Testing infrastructure guide

---

**한국어 안내:**

워크플로우를 활성화하려면 다음 명령어를 실행하세요:

```bash
mkdir -p .github/workflows
cp .github-workflows-template/memory-pool-benchmarks.yml .github/workflows/
git add .github/workflows/memory-pool-benchmarks.yml
git commit -m "ci: Add automated memory pool performance benchmarks workflow"
git push
```

설치 후 GitHub Actions에서 수동으로 워크플로우를 실행하면 성능 결과가 자동으로 문서에 업데이트됩니다.
