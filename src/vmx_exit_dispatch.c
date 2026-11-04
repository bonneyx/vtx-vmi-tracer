/**
 * @file vmx_exit_dispatch.c
 * @brief Reference VM-Exit dispatcher for EPT Violations and Monitor Trap Flag (MTF) step-over.
 */

#include "../include/vmx_arch.h"
#include "../include/ept_vmi.h"

typedef struct _vcpu_vmi_context {
    vmi_watchpoint_t watchpoints[32];
    vmi_watchpoint_t *pending_mtf_wp;
    uint64_t         event_count;
    vmi_trace_event_t last_event;
} vcpu_vmi_context_t;

/* Mock/abstract VMCS read/write hooks for standalone compilation & unit testing */
extern uint64_t vmcs_read64(enum vmcs_field_encoding field);
extern void     vmcs_write64(enum vmcs_field_encoding field, uint64_t value);

static void enable_monitor_trap_flag(bool enable)
{
    uint64_t proc_ctls = vmcs_read64(VMCS_CTRL_CPU_BASED);
    if (enable) {
        proc_ctls |= CPU_BASED_MONITOR_TRAP_FLAG;
    } else {
        proc_ctls &= ~CPU_BASED_MONITOR_TRAP_FLAG;
    }
    vmcs_write64(VMCS_CTRL_CPU_BASED, proc_ctls);
}

bool vmi_dispatch_vmexit(vcpu_vmi_context_t *ctx, vmi_guest_registers_t *regs)
{
    (void)regs;
    uint32_t exit_reason = (uint32_t)(vmcs_read64(VMCS_RO_VM_EXIT_REASON) & 0xFFFFU);

    switch (exit_reason) {
    case EXIT_REASON_EPT_VIOLATION: {
        ept_violation_qualification_t qual;
        qual.raw = vmcs_read64(VMCS_RO_EXIT_QUALIFICATION);

        uint64_t gpa       = vmcs_read64(VMCS_RO_GUEST_PHYSICAL_ADDR);
        uint64_t gla       = vmcs_read64(VMCS_RO_GUEST_LINEAR_ADDR);
        uint64_t guest_cr3 = vmcs_read64(VMCS_GUEST_CR3);
        uint64_t guest_rip = vmcs_read64(VMCS_GUEST_RIP);

        if (ept_vmi_handle_violation(
                ctx->watchpoints,
                32U,
                gpa,
                gla,
                guest_cr3,
                guest_rip,
                qual,
                &ctx->last_event)) {
            ctx->event_count++;
            /* Find matching watchpoint to re-arm on next MTF trap */
            uint64_t aligned_gpa = gpa & ~(EPT_PAGE_SIZE - 1ULL);
            for (uint32_t i = 0; i < 32U; ++i) {
                if (ctx->watchpoints[i].active &&
                    ctx->watchpoints[i].guest_physical_page == aligned_gpa) {
                    ctx->pending_mtf_wp = &ctx->watchpoints[i];
                    enable_monitor_trap_flag(true);
                    break;
                }
            }
            return true;
        }
        return false;
    }

    case EXIT_REASON_MTF: {
        /* Re-arm the EPT permission hook after single-stepping the guest instruction */
        if (ctx->pending_mtf_wp && ctx->pending_mtf_wp->bound_pte) {
            vmi_watchpoint_t *wp = ctx->pending_mtf_wp;
            wp->bound_pte->fields.read_access  = (wp->enforced_permissions & EPT_ACCESS_READ) ? 1 : 0;
            wp->bound_pte->fields.write_access = (wp->enforced_permissions & EPT_ACCESS_WRITE) ? 1 : 0;
            wp->bound_pte->fields.exec_access  = (wp->enforced_permissions & EPT_ACCESS_EXEC) ? 1 : 0;
            ctx->pending_mtf_wp = 0;
        }
        enable_monitor_trap_flag(false);
        return true;
    }

    default:
        return false;
    }
}

