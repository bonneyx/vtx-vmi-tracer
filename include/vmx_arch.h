/**
 * @file vmx_arch.h
 * @brief Intel VT-x (VMX) architectural constants and VMCS field encodings
 *        based on Intel 64 and IA-32 Architectures Software Developer's Manual, Vol. 3C.
 */

#ifndef VTX_VMI_VMX_ARCH_H
#define VTX_VMI_VMX_ARCH_H

#include <stdint.h>

/* IA32 MSR encodings for VMX capability reporting */
#define IA32_FEATURE_CONTROL_MSR            0x0000003AU
#define IA32_VMX_BASIC_MSR                  0x00000480U
#define IA32_VMX_PINBASED_CTLS_MSR          0x00000481U
#define IA32_VMX_PROCBASED_CTLS_MSR         0x00000482U
#define IA32_VMX_EXIT_CTLS_MSR              0x00000483U
#define IA32_VMX_ENTRY_CTLS_MSR             0x00000484U
#define IA32_VMX_PROCBASED_CTLS2_MSR        0x0000048BU
#define IA32_VMX_EPT_VPID_CAP_MSR           0x0000048CU

/* Primary Processor-Based VM-Execution Controls */
#define CPU_BASED_MONITOR_TRAP_FLAG         (1UL << 27)
#define CPU_BASED_ACTIVATE_MSR_BITMAP       (1UL << 28)
#define CPU_BASED_ACTIVATE_SECONDARY_CTLS   (1UL << 31)

/* Secondary Processor-Based VM-Execution Controls */
#define CPU_BASED_CTL2_ENABLE_EPT           (1UL << 1)
#define CPU_BASED_CTL2_ENABLE_RDTSCP        (1UL << 3)
#define CPU_BASED_CTL2_ENABLE_VPID          (1UL << 5)
#define CPU_BASED_CTL2_ENABLE_INVPCID       (1UL << 12)
#define CPU_BASED_CTL2_ENABLE_XSAVES        (1UL << 20)

/* Selected VM-Exit Reasons (Intel SDM Vol. 3C, Appendix C) */
#define EXIT_REASON_EXCEPTION_NMI           0U
#define EXIT_REASON_CPUID                   10U
#define EXIT_REASON_VMCALL                  18U
#define EXIT_REASON_CR_ACCESS               28U
#define EXIT_REASON_MSR_READ                31U
#define EXIT_REASON_MSR_WRITE               32U
#define EXIT_REASON_MTF                     37U
#define EXIT_REASON_EPT_VIOLATION           48U
#define EXIT_REASON_EPT_MISCONFIG           49U

/* Selected VMCS Field Encodings */
enum vmcs_field_encoding {
    VMCS_CTRL_PIN_BASED                     = 0x00004000,
    VMCS_CTRL_CPU_BASED                     = 0x00004002,
    VMCS_CTRL_EXCEPTION_BITMAP              = 0x00004004,
    VMCS_CTRL_SECONDARY_CPU_BASED           = 0x0000401E,
    VMCS_CTRL_EPT_POINTER                   = 0x0000201A,

    VMCS_RO_VM_EXIT_REASON                  = 0x00004402,
    VMCS_RO_EXIT_QUALIFICATION              = 0x00006400,
    VMCS_RO_GUEST_LINEAR_ADDR               = 0x0000640A,
    VMCS_RO_GUEST_PHYSICAL_ADDR             = 0x00002400,

    VMCS_GUEST_CR0                          = 0x00006800,
    VMCS_GUEST_CR3                          = 0x00006802,
    VMCS_GUEST_CR4                          = 0x00006804,
    VMCS_GUEST_DR7                          = 0x0000681A,
    VMCS_GUEST_RSP                          = 0x0000681C,
    VMCS_GUEST_RIP                          = 0x0000681E,
    VMCS_GUEST_RFLAGS                       = 0x00006820
};

/* Guest General-Purpose Register Context */
typedef struct _vmi_guest_registers {
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbx;
    uint64_t rsp;
    uint64_t rbp;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
} vmi_guest_registers_t;

#endif /* VTX_VMI_VMX_ARCH_H */

