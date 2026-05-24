/*
 * Open Firmware client interface for memtestppc+
 */

#include "ofw.h"

typedef int (*ofw_entry_t)(void *);

/* Defined in head.S — filled in at entry */
extern ofw_cell_t _ofw_cif_store;

struct ofw_args {
    const char *service;
    int nargs;
    int nret;
    ofw_cell_t args[10];
};

static int ofw_call_raw(struct ofw_args *args)
{
    ofw_entry_t entry = (ofw_entry_t)_ofw_cif_store;
    return entry(args);
}

ofw_handle_t ofw_finddevice(const char *path)
{
    struct ofw_args args;
    args.service = "finddevice";
    args.nargs = 1;
    args.nret = 1;
    args.args[0] = (ofw_cell_t)path;
    args.args[1] = 0;
    ofw_call_raw(&args);
    return (ofw_handle_t)args.args[1];
}

int ofw_getprop(ofw_handle_t phandle, const char *name, void *buf, int buflen)
{
    struct ofw_args args;
    args.service = "getprop";
    args.nargs = 4;
    args.nret = 1;
    args.args[0] = (ofw_cell_t)phandle;
    args.args[1] = (ofw_cell_t)name;
    args.args[2] = (ofw_cell_t)buf;
    args.args[3] = (ofw_cell_t)buflen;
    args.args[4] = 0;
    ofw_call_raw(&args);
    return (int)args.args[4];
}

ofw_ihandle_t ofw_open(const char *path)
{
    struct ofw_args args;
    args.service = "open";
    args.nargs = 1;
    args.nret = 1;
    args.args[0] = (ofw_cell_t)path;
    args.args[1] = 0;
    ofw_call_raw(&args);
    return (ofw_ihandle_t)args.args[1];
}

void *ofw_claim(void *addr, unsigned int size, unsigned int align)
{
    struct ofw_args args;
    args.service = "claim";
    args.nargs = 3;
    args.nret = 1;
    args.args[0] = (ofw_cell_t)addr;
    args.args[1] = (ofw_cell_t)size;
    args.args[2] = (ofw_cell_t)align;
    args.args[3] = 0;
    ofw_call_raw(&args);
    return (void *)args.args[3];
}

void ofw_exit(void)
{
    struct ofw_args args;
    args.service = "exit";
    args.nargs = 0;
    args.nret = 0;
    ofw_call_raw(&args);
}

void ofw_reset(void)
{
    struct ofw_args args;
    /* Try interpret "reset-all" as a Forth word */
    args.service = "interpret";
    args.nargs = 1;
    args.nret = 1;
    args.args[0] = (ofw_cell_t)"reset-all";
    args.args[1] = 0;
    ofw_call_raw(&args);
}

int ofw_write(ofw_ihandle_t ihandle, const void *buf, int len)
{
    struct ofw_args args;
    args.service = "write";
    args.nargs = 3;
    args.nret = 1;
    args.args[0] = (ofw_cell_t)ihandle;
    args.args[1] = (ofw_cell_t)buf;
    args.args[2] = (ofw_cell_t)len;
    args.args[3] = 0;
    ofw_call_raw(&args);
    return (int)args.args[3];
}

void ofw_puts(const char *s)
{
    ofw_handle_t chosen;
    ofw_ihandle_t stdout_ih;
    int len = 0;
    const char *p = s;

    chosen = ofw_finddevice("/chosen");
    if (chosen == -1) return;
    if (ofw_getprop(chosen, "stdout", &stdout_ih, sizeof(stdout_ih)) < 0)
        return;

    while (*p++) len++;
    ofw_write(stdout_ih, s, len);
}

ofw_ihandle_t ofw_get_stdin(void)
{
    ofw_handle_t chosen;
    ofw_ihandle_t stdin_ih;

    chosen = ofw_finddevice("/chosen");
    if (chosen == -1) return -1;

    if (ofw_getprop(chosen, "stdin", &stdin_ih, sizeof(stdin_ih)) < 0)
        return -1;

    return stdin_ih;
}

int ofw_read(ofw_ihandle_t ihandle, void *buf, int len)
{
    struct ofw_args args;
    args.service = "read";
    args.nargs = 3;
    args.nret = 1;
    args.args[0] = (ofw_cell_t)ihandle;
    args.args[1] = (ofw_cell_t)buf;
    args.args[2] = (ofw_cell_t)len;
    args.args[3] = 0;
    ofw_call_raw(&args);
    return (int)args.args[3];
}

int ofw_get_fb_info(struct ofw_fb_info *info)
{
    ofw_handle_t screen_ph;
    ofw_handle_t chosen;
    ofw_ihandle_t stdout_ih;
    unsigned int val;

    /* Try /chosen/stdout first, then fall back to /screen */
    chosen = ofw_finddevice("/chosen");
    if (chosen != -1) {
        if (ofw_getprop(chosen, "stdout", &stdout_ih, sizeof(stdout_ih)) >= 0) {
            /* Try to get the screen phandle from stdout */
        }
    }

    /* Find the screen device */
    screen_ph = ofw_finddevice("screen");
    if (screen_ph == -1) {
        screen_ph = ofw_finddevice("/pci/ATY,RageM3_p@10");
        if (screen_ph == -1) {
            screen_ph = ofw_finddevice("/bandit/ATY,mach64_3DU@10");
            if (screen_ph == -1) {
                screen_ph = ofw_finddevice("/pci@f2000000/ATY,RageM3_p@10");
                if (screen_ph == -1) {
                    screen_ph = ofw_finddevice("/pci@f0000000/ATY,Rage128Pro@10");
                    if (screen_ph == -1) {
                        screen_ph = ofw_finddevice("/pci/vga");
                        if (screen_ph == -1) {
                            screen_ph = ofw_finddevice("/pci@f0000000/display");
                        }
                    }
                }
            }
        }
    }
    if (screen_ph == -1)
        return -1;

    if (ofw_getprop(screen_ph, "address", &val, sizeof(val)) >= 0) {
        info->addr = val;
    } else {
        /* Try assigned-addresses */
        unsigned int addrs[20];
        int len = ofw_getprop(screen_ph, "assigned-addresses", addrs, sizeof(addrs));
        if (len >= 20) {
            info->addr = addrs[2]; /* PCI BAR value */
        } else {
            return -1;
        }
    }

    if (ofw_getprop(screen_ph, "width", &val, sizeof(val)) >= 0)
        info->width = val;
    else
        info->width = 640;

    if (ofw_getprop(screen_ph, "height", &val, sizeof(val)) >= 0)
        info->height = val;
    else
        info->height = 480;

    if (ofw_getprop(screen_ph, "depth", &val, sizeof(val)) >= 0)
        info->depth = val;
    else
        info->depth = 8;

    if (ofw_getprop(screen_ph, "linebytes", &val, sizeof(val)) >= 0)
        info->linesize = val;
    else
        info->linesize = info->width * (info->depth / 8);

    return 0;
}

unsigned long ofw_get_timebase_freq(void)
{
    ofw_handle_t cpus, cpu;
    unsigned long freq = 0;

    cpus = ofw_finddevice("/cpus");
    if (cpus == -1) {
        /* Try getting it from /cpus/@0 or just /PowerPC,G3@0 */
        cpu = ofw_finddevice("/cpus/PowerPC,G3@0");
        if (cpu == -1) {
            cpu = ofw_finddevice("/cpus/PowerPC,750@0");
            if (cpu == -1) {
                cpu = ofw_finddevice("/cpus/cpu@0");
            }
        }
    } else {
        cpu = cpus;
        /* Try child nodes */
        cpu = ofw_finddevice("/cpus/PowerPC,G3@0");
        if (cpu == -1) {
            cpu = ofw_finddevice("/cpus/PowerPC,750@0");
            if (cpu == -1) {
                cpu = ofw_finddevice("/cpus/cpu@0");
            }
        }
    }

    if (cpu != -1) {
        ofw_getprop(cpu, "timebase-frequency", &freq, sizeof(freq));
    }

    if (freq == 0) freq = 25000000; /* 25 MHz fallback */
    return freq;
}
