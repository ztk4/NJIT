# First Day

- Use kvm to virtualize the kernel for hw.
- Install lxr for kernel cross referencing.

# Memory

- logical: seg (GDT [Global Descriptor Table])
- linear: flat 4G in 32-bit (dir/table/page)
- physical: machine (mem cntlr)

The kernel primarily works in the linear memory space.

### e820-memmap

- e820 is the memory address to the BIOS routine (which reports the memory map).
- Kernel pushes this mapping into memblocks (abstraction of physical memory map).
- e820 -> memblocks -> pages -> node 0 -> zone -> mem type

#### Pages

A list of 1M page structs (idx [0, 1M)) which point to the linear address regions  associated with each page.
This totals to 4G of memory.

#### Nodes

On a motherboard without SMP, there is only one node.

#### Zones

The list of pages is split into multiple zones, such as DMA zone and normal zone.
Each zone is a consecutive range of page frame numbers (pfm).

#### Memory Types

Each zone is split up into regions of different memory types.

#### Flat Memory (32 bit)

- dir is a list of 1024 entries pointing to a page table
- page table is a list of 1024 pages
- Each page is 4K.

### Allocation

1. get\_free\_pages (gets full pages)
1. kmalloc (gets just a certain number of bytes for kernel use)
1. vmalloc (non-consecutive block of memory [usually large allocations])

## Addressing

### Logical (48 bits)
- \[15 segment selector 0]\[31 offset 0]

- segment selector -> offset to GDTR

- GDTR -> base of GDT

- GDT has 32 entries of 64-bits each
    - In modern computers the segment is usually 0, always pointing to a singular valid entry in the GDT

### Linear (32 bits)

- \[31 dir 22]\[21 page 12]\[11 offset 0]

- dir and page together reference a physical page
- the offset indicates where in that page.

- The base pointer of the page directory table is in register cr3
    - dir is relative to this pointer

- Each entry of the PDT points to the base of a page table
    - page is relative to this pointer

- Each entry of a PT points to the base of a 4K page
    - offset is relative to this pointer (refers to a single byte).

### BIOS e820 Map

```c
/* The following is pseudo-code-ish */
/* Type semantics and pointer semantics are not really observed */
struct e820map {
    struct e820_entry e820_map[128];
} boot_params;

struct e820_entry {
    addr;
    size;
    type;
};

/* global */
struct e820map e820 = { nr_map, e820_entry, map };

struct memblock {
    current_limit;
    memory_size;
    memblock_type memory;    /* Regions are all usable */
    memblock_type reserved;  /* Regions are all reserved */
};

struct memblock_type {
    cnt;
    max;
    struct region regions[N];
};

struct region {
    base;
    size;
    nid = 0;  /* node id */
};
```

#### Boot Flow

- detect\_memory
- start\_kernel
    - setup\_arch
        - setup\_memory\_map (Works on boot_params e820_map)
    - memblock\_x86\_fill (Works on global e820 map)
    - init\_mem\_mapping
    - initmem\_init (base, size, nid)
    - pagetable\_init
        - zonesizes\_init


