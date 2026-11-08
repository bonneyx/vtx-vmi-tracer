# Security Policy

## Security Maintainer

- **Lead Security Maintainer:** [@bonneyx](https://github.com/bonneyx)
- **Scope:** Hypervisor VMCS/EPT introspection core (`src/ept_monitor.c`, `src/vmx_exit_dispatch.c`) and guest memory isolation boundaries.

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 0.2.x   | :white_check_mark: |
| < 0.2.0 | :x:                |

## Reporting a Vulnerability

If you discover a guest-to-host privilege escalation vector, EPT page-splitting race condition, or VMCS state corruption issue in `vtx-vmi-tracer`, please report it privately via [GitHub Security Advisories](https://github.com/bonneyx/vtx-vmi-tracer/security/advisories/new).
