/**
 * @file ept_monitor.c
 * @brief EPT permission watchpoint management and violation triage for sandbox introspection.
 */

#include "../include/ept_vmi.h"
#include "../include/vmx_arch.h"

static inline uint64_t read_tsc_counter(void) {
#if defined(_MSC_VER)
    return __rdtsc();
#elif defined(__GNUC__) || defined(__clang__)
    uint32_t lo = 0, hi = 0;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#else
    return 0;
#endif
}

bool ept_vmi_install_watchpoint(
    vmi_watchpoint_t *wp,
    ept_pte_t *pte,
    uint64_t gpa,
    uint64_t target_cr3,
    uint8_t restricted_perms)
{
    if (!wp || !pte) {
        return false;
    }

    wp->original_permissions = (uint8_t)(pte->raw & EPT_ACCESS_RWX);
    wp->enforced_permissions = restricted_perms & EPT_ACCESS_RWX;
    wp->guest_physical_page  = gpa & ~(EPT_PAGE_SIZE - 1ULL);
    wp->target_cr3           = target_cr3;
    wp->bound_pte            = pte;
    wp->active               = true;

    /* Apply restricted access mask while preserving PFN and memory type */
    pte->fields.read_access  = (wp->enforced_permissions & EPT_ACCESS_READ) ? 1 : 0;
    pte->fields.write_access = (wp->enforced_permissions & EPT_ACCESS_WRITE) ? 1 : 0;
    pte->fields.exec_access  = (wp->enforced_permissions & EPT_ACCESS_EXEC) ? 1 : 0;

    return true;
}

void ept_vmi_restore_watchpoint(vmi_watchpoint_t *wp)
{
    if (!wp || !wp->active || !wp->bound_pte) {
        return;
    }

    wp->bound_pte->fields.read_access  = (wp->original_permissions & EPT_ACCESS_READ) ? 1 : 0;
    wp->bound_pte->fields.write_access = (wp->original_permissions & EPT_ACCESS_WRITE) ? 1 : 0;
    wp->bound_pte->fields.exec_access  = (wp->original_permissions & EPT_ACCESS_EXEC) ? 1 : 0;
    wp->active = false;
}

bool ept_vmi_handle_violation(
    vmi_watchpoint_t *table,
    uint32_t count,
    uint64_t gpa,
    uint64_t gla,
    uint64_t guest_cr3,
    uint64_t guest_rip,
    ept_violation_qualification_t qual,
    vmi_trace_event_t *out_event)
{
    uint64_t aligned_gpa = gpa & ~(EPT_PAGE_SIZE - 1ULL);

    for (uint32_t i = 0; i < count; ++i) {
        vmi_watchpoint_t *wp = &table[i];
        if (!wp->active || wp->guest_physical_page != aligned_gpa) {
            continue;
        }

        /* Temporarily restore original permissions to allow single-step via MTF */
        wp->bound_pte->fields.read_access  = (wp->original_permissions & EPT_ACCESS_READ) ? 1 : 0;
        wp->bound_pte->fields.write_access = (wp->original_permissions & EPT_ACCESS_WRITE) ? 1 : 0;
        wp->bound_pte->fields.exec_access  = (wp->original_permissions & EPT_ACCESS_EXEC) ? 1 : 0;

        /* Only emit introspection event if CR3 matches (or wildcard 0) */
        if (wp->target_cr3 == 0 || wp->target_cr3 == guest_cr3) {
            if (out_event) {
                out_event->timestamp_tsc       = read_tsc_counter();
                out_event->guest_cr3           = guest_cr3;
                out_event->guest_rip           = guest_rip;
                out_event->guest_linear_addr   = gla;
                out_event->guest_physical_addr = gpa;
                out_event->access_mask         = (uint32_t)(qual.raw & EPT_ACCESS_RWX);
                out_event->vcpu_id             = 0;
            }
        }
        return true;
    }

    return false;
}

