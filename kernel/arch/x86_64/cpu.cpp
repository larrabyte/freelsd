#include <errors.hpp>
#include <cpu.hpp>
#include <cpuid.h>

namespace cpu {
    const char *exceptionstr[] = {
        "Divide-by-zero exception.",
        "Debug exception.",
        "Non-maskable interrupt.",
        "Breakpoint exception.",
        "Overflow exception.",
        "Bound range exceeded.",
        "Invalid opcode.",
        "Device not available.",
        "Double fault exception.",
        "Reserved exception.",
        "Invalid task state segment.",
        "Segment not present.",
        "Stack segment fault.",
        "General protection fault.",
        "Page fault.",
        "Reserved exception.",
        "x87 floating-point exception.",
        "Alignment check exception.",
        "Machine check exception.",
        "SIMD floating-point exception.",
        "Virtualisation exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Reserved exception.",
        "Security exception.",
        "Reserved exception."
    };

    void handler(idt::regs64_t *regs) {
        ctxpanic(regs, exceptionstr[regs->isr]);
    }
}
