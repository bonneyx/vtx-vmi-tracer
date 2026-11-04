/**
 * @file ept_vmi.h
 * @brief Extended Page Table (EPT) structures and VMI watchpoint descriptors.
 */

#ifndef VTX_VMI_EPT_VMI_H
#define VTX_VMI_EPT_VMI_H

#include <stdint.h>
#include <stdbool.h>

#define EPT_PAGE_SHIFT          12ULL
#define EPT_PAGE_SIZE           (1ULL << EPT_PAGE_SHIFT)
#define EPT_TABLE_ENTRIES       512U

/* EPT Access Permission Bits */
#define EPT_ACCESS_NONE         0x0ULL
#define EPT_ACCESS_READ         (1ULL << 0)
#define EPT_ACCESS_WRITE        (1ULL << 1)
#define EPT_ACCESS_EXEC         (1ULL << 2)
#define EPT_ACCESS_RW           (EPT_ACCESS_READ | EPT_ACCESS_WRITE)
#define EPT_ACCESS_RWX          (EPT_ACCESS_READ | EPT_ACCESS_WRITE | EPT_ACCESS_EXEC)

/* EPT Memory Types */
#define EPT_MEMORY_TYPE_UC      0ULL
#define EPT_MEMORY_TYPE_WB      6ULL

/* EPT Violation Exit Qualification Bits (Intel SDM Vol. 3C, Table 28-7) */
typedef union _ept_violation_qualification {
    uint64_t raw;
    struct {
        uint64_t read_access        : 1; /* Bit 0: Violation caused by data read */
        uint64_t write_access       : 1; /* Bit 1: Violation caused by data write */
        uint64_t exec_access        : 1; /* Bit 2: Violation caused by instruction fetch */
        uint64_t readable           : 1; /* Bit 3: EPT entry allows read */
        uint64_t writable           : 1; /* Bit 4: EPT entry allows write */
        uint64_t executable         : 1; /* Bit 5: EPT entry allows execute */
        uint64_t executable_user    : 1; /* Bit 6: Mode-based execute control */
        uint64_t valid_guest_linear : 1; /* Bit 7: Guest linear address valid */
        uint64_t translation_fault  : 1; /* Bit 8: Page walk vs final GPA access */
        uint64_t reserved           : 55;
    } bits;
} ept_violation_qualification_t;

typedef union _ept_pte {
    uint64_t raw;
    struct {
        uint64_t read_access        : 1;
        uint64_t write_access       : 1;
        uint64_t exec_access        : 1;
        uint64_t memory_type        : 3;
        uint64_t ignore_pat         : 1;
        uint64_t reserved0          : 1;
        uint64_t accessed           : 1;
        uint64_t dirty              : 1;
        uint64_t exec_user          : 1;
        uint64_t ignored1           : 1;
        uint64_t page_frame_number  : 36;
        uint64_t reserved1          : 4;
        uint64_t ignored2           : 11;
        uint64_t suppress_ve        : 1;
    } fields;
} ept_pte_t;

/* Introspection telemetry record emitted on watched EPT events */
typedef struct _vmi_trace_event {
    uint64_t timestamp_tsc;
    uint64_t guest_cr3;
    uint64_t guest_rip;
    uint64_t guest_linear_addr;
    uint64_t guest_physical_addr;
    uint32_t access_mask;
    uint32_t vcpu_id;
} vmi_trace_event_t;

typedef struct _vmi_watchpoint {
    bool     active;
    uint64_t target_cr3;            /* 0 = monitor all address spaces */
    uint64_t guest_physical_page;   /* 4KB-aligned GPA */
    uint8_t  enforced_permissions;  /* Restricted EPT permissions */
    uint8_t  original_permissions;  /* Saved permissions for MTF step-over */
    ept_pte_t *bound_pte;
} vmi_watchpoint_t;

bool ept_vmi_install_watchpoint(vmi_watchpoint_t *wp, ept_pte_t *pte, uint64_t gpa, uint64_t target_cr3, uint8_t restricted_perms);
void ept_vmi_restore_watchpoint(vmi_watchpoint_t *wp);
bool ept_vmi_handle_violation(vmi_watchpoint_t *table, uint32_t count, uint64_t gpa, uint64_t gla, uint64_t guest_cr3, uint64_t guest_rip, ept_violation_qualification_t qual, vmi_trace_event_t *out_event);

#endif /* VTX_VMI_EPT_VMI_H */

