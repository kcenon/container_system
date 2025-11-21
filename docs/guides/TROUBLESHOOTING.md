# Container System – Troubleshooting Guide

This guide consolidates the most common issues reported while using the Container System and explains how to interpret `Result<T>` failures.

## 1. Serialization or Deserialization Fails

**Symptoms**
- `Result<std::string>` or `Result<value_container>` returns `is_err() == true`.
- Error message similar to `missing required field` or `type mismatch`.

**Checklist**
1. Confirm the builder configured every required field before calling `.build()`.
2. Inspect the error via `const auto& err = result.error();` to retrieve `err.code` and `err.message`.
3. Compare `err.code` with the error table in `docs/guides/FAQ.md` to determine if the issue is schema-related.

## 2. SIMD Optimization Disabled

**Symptoms**
- Benchmarks are slower than documented numbers.
- Build logs show `SIMD support disabled`.

**Checklist**
1. Ensure the compiler flags include the relevant ISA (e.g., `-march=native` for GCC/Clang).
2. Re-run `cmake` after cleaning the build directory so the feature-detection scripts re-evaluate hardware support.
3. Validate at runtime with `container::simd::is_enabled()` (if available) before filing a bug.

## 3. Integration Failures

**Symptoms**
- Linking errors or undefined references when mixing Container System with other modules.

**Checklist**
1. Verify the consuming project links against the same version of `common_system`.
2. Ensure the integration follows the steps in `docs/guides/INTEGRATION.md`.
3. When in doubt, run the provided examples under `examples/` to confirm the toolkit builds correctly in isolation.

---

For additional issues, file a ticket with the failing `Result` payload (`err.code`, `err.message`, `err.module`) so maintainers can reproduce the scenario quickly.
