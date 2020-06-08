#include <multiboot.hpp>
#include <serial.hpp>
#include <stddef.h>
#include <stdint.h>

namespace mboot {
    static size_t mbisize;
    static mb_tag_t *tag;

    void initialise(uintptr_t mbaddr) {
        mbisize = *(size_t*) mbaddr;
        tag = (mb_tag_t*) (mbaddr + 8);

        while(tag->type != MULTIBOOT_TAG_TYPE_END) {
            switch(tag->type) {
                case MULTIBOOT_TAG_TYPE_CMDLINE:
                    serial::printf("[mboot2] tag: MULTIBOOT_TAG_TYPE_CMDLINE, size: %d bytes\n", tag->size);
                    serial::printf("         command line: %s\n\n", ((mb_tag_string_t*) tag)->string);
                    break;
                case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                    serial::printf("[mboot2] tag: MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME, size: %d bytes\n", tag->size);
                    serial::printf("         bootloader name: %s\n\n", ((mb_tag_string_t*) tag)->string);
                    break;
                case MULTIBOOT_TAG_TYPE_MODULE:
                    serial::printf("[mboot2] tag: MULTIBOOT_TAG_TYPE_MODULE, size: %d bytes\n", tag->size);
                    serial::printf("         module loaded at: 0x%lx - 0x%lx\n", ((mb_tag_module_t*) tag)->start, ((mb_tag_module_t*) tag)->end);
                    serial::printf("         command line: %s\n\n", ((mb_tag_module_t*) tag)->cmdline);
                    break;
                case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                    serial::printf("[mboot2] tag: MULTIBOOT_TAG_TYPE_BASIC_MEMINFO, size: %d bytes\n", tag->size);
                    serial::printf("         lower: %uKB, higher: %uKB\n\n", ((mb_tag_basicmem_t*) tag)->lower, ((mb_tag_basicmem_t*) tag)->upper);
                    break;
                case MULTIBOOT_TAG_TYPE_BOOTDEV:
                    serial::printf("[mboot2] tag: MULTIBOOT_TAG_TYPE_BOOTDEV, size: %d bytes\n", tag->size);
                    serial::printf("         address/slice/part: 0x%lx/0x%lx/0x%lx\n\n", ((mb_tag_bootdev_t*) tag)->biosdev, ((mb_tag_bootdev_t*) tag)->slice, ((mb_tag_bootdev_t*) tag)->part);
                    break;
                case MULTIBOOT_TAG_TYPE_MMAP:
                    serial::printf("[mboot2] tag: MULTIBOOT_TAG_TYPE_MMAP, size: %d bytes\n", tag->size);
                    for(mb_mmap_entry_t *entry = ((mb_tag_mmap_t*) tag)->entries; (uintptr_t) entry < (uintptr_t) (tag + tag->size) && entry->type < MULTIBOOT_MEMORY_BADRAM; entry = (mb_mmap_entry_t*) ((uintptr_t) entry + ((mb_tag_mmap_t*) tag)->entrysize)) {
                        serial::printf("         mmap entry: %p, %p -> ", entry->addr, entry->len);
                        switch(entry->type) {
                            case MULTIBOOT_MEMORY_AVAILABLE:
                                serial::write("AVAILABLE\n");
                                break;
                            case MULTIBOOT_MEMORY_RESERVED:
                                serial::write("RESERVED\n");
                                break;
                            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                                serial::write("ACPI RECLAIMABLE\n");
                                break;
                            case MULTIBOOT_MEMORY_NVS:
                                serial::write("ACPI NVS\n");
                                break;
                            case MULTIBOOT_MEMORY_BADRAM:
                                serial::write("BAD MEMORY\n");
                                break;
                            default:
                                break;
                        }
                    }
                    serial::printf("\n");
                    break;
                case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
                    serial::printf("[mboot2] tag: MULTIBOOT_TAG_TYPE_FRAMEBUFFER, size: %d bytes\n", tag->size);
                    serial::printf("         framebuffer: %p\n\n", ((mb_tag_framebuffer_t*) tag)->common.framebuffer);
                    break;
                default:
                    break;
            }

            tag = (mb_tag_t*) ((uintptr_t) tag + ((tag->size + 7) & ~7));
        }
    }
}
