# vtx-vmi-tracer

A lightweight **Intel VT-x / EPT Virtual Machine Introspection (VMI)** research framework designed for isolated dynamic binary analysis and kernel-level execution tracing.

## Overview

`vtx-vmi-tracer` is a clean-room hypervisor research prototype focused on **defensive security monitoring** and **sandbox introspection**. By leveraging Intel Virtualization Technology (VT-x) and Extended Page Tables (EPT), it enables transparent monitoring of guest memory accesses and control-register transitions without modifying guest kernel structures.

### Key Defensive Use Cases
- **Dynamic Malware Analysis:** Observing unpacking routines and page-level execution transitions (`W^X` violations) via EPT violation handlers.
- **Kernel Vulnerability Triage:** Capturing precise architectural register state (`GUEST_RIP`, `GUEST_RSP`, `CR3`, `DR0-DR7`) upon targeted memory access events.
- **Anti-Tamper Sandbox Research:** Studying hardware-assisted isolation mechanisms for security analysis tools.

## Architecture

```
+-------------------------------------------------------------------+
|                     Guest OS / Target Binary                      |
|   [CR3 Context]  --->  [Tracked Virtual Page]  --->  #VE / VMExit |
+-------------------------------------------------------------------+
                                   |
                         VM-Exit (Reason 48: EPT Violation)
                                   v
+-------------------------------------------------------------------+
|                   vtx-vmi-tracer (VMX Root Mode)                  |
|  +------------------+   +-------------------+   +--------------+  |
|  | VMCS State Audit |-->| EPT Permission    |-->| VMI Event    |  |
|  | (CR3 / RIP / RFLAGS) | Page-Split Engine |   | Ring Buffer  |  |
|  +------------------+   +-------------------+   +--------------+  |
+-------------------------------------------------------------------+
```

## Repository Structure

```text
vtx-vmi-tracer/
├── include/
│   ├── vmx_arch.h        # Intel SDM Vol. 3 VMCS & MSR architectural definitions
│   └── ept_vmi.h         # EPT 4-level paging & introspection event structures
├── src/
│   ├── ept_monitor.c     # EPT page permission management & violation triage
│   └── vmx_exit_dispatch.c # VM-Exit handler for EPT violations & CR3 tracking
├── CMakeLists.txt
└── README.md
```

## Technical Notes

- **Page Splitting (2MB to 4KB):** When placing an execution or write watchpoint on a specific guest physical address (GPA), `ept_split_large_page()` dynamically decomposes the target 2MB PDE into 512 individual 4KB PTEs to minimize EPT violation noise across unrelated pages.
- **Single-Stepping via Monitor Trap Flag (MTF):** Upon triggering an EPT violation on a watched page, the handler temporarily restores page permissions, sets the VMCS **Monitor Trap Flag (`CPU_BASED_MONITOR_TRAP_FLAG`)**, and re-applies the EPT hook on the subsequent `EXIT_REASON_MTF` event.

> **Disclaimer:** This repository contains research prototypes and reference implementations extracted from my local virtualization lab for defensive binary analysis and VMI research. External pull requests are not actively tracked.

## License

MIT License

