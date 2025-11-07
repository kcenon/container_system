#!/bin/bash
# Install GitHub Actions workflow for automated memory pool benchmarks

set -e

echo "Installing automated memory pool performance benchmarks workflow..."

# Create workflows directory if it doesn't exist
mkdir -p .github/workflows

# Copy the workflow file
cp .github-workflows-template/memory-pool-benchmarks.yml .github/workflows/

echo "✓ Workflow file copied to .github/workflows/"

# Check if we're in a git repository
if [ -d .git ]; then
    echo ""
    echo "To commit and push the workflow:"
    echo "  git add .github/workflows/memory-pool-benchmarks.yml"
    echo "  git commit -m \"ci: Add automated memory pool performance benchmarks workflow\""
    echo "  git push"
    echo ""
    echo "Would you like to commit now? (y/n)"
    read -r response

    if [[ "$response" =~ ^[Yy]$ ]]; then
        git add .github/workflows/memory-pool-benchmarks.yml
        git commit -m "ci: Add automated memory pool performance benchmarks workflow"
        echo ""
        echo "✓ Committed. Run 'git push' to push to remote."
    fi
else
    echo "✓ Installation complete!"
fi

echo ""
echo "Next steps:"
echo "1. Push the workflow to your repository"
echo "2. Go to Actions → Memory Pool Performance Benchmarks"
echo "3. Click 'Run workflow' to trigger the first benchmark run"
echo ""
echo "Results will be automatically updated in docs/MEMORY_POOL_PERFORMANCE.md"
