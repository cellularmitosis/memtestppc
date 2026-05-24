typedef unsigned long ofw_cell_t;
typedef int (*ofw_entry_t)(void *);

extern ofw_cell_t _ofw_r3, _ofw_r4, _ofw_r5, _ofw_r6, _ofw_r7;

struct ofw_args {
    const char *service;
    int nargs;
    int nret;
    ofw_cell_t args[10];
};

static int finddevice(ofw_entry_t e, const char *path)
{
    struct ofw_args a;
    a.service = "finddevice";
    a.nargs = 1;
    a.nret = 1;
    a.args[0] = (ofw_cell_t)path;
    a.args[1] = 0;
    e(&a);
    return (int)a.args[1];
}

static int getprop(ofw_entry_t e, int ph, const char *name, void *buf, int len)
{
    struct ofw_args a;
    a.service = "getprop";
    a.nargs = 4;
    a.nret = 1;
    a.args[0] = (ofw_cell_t)ph;
    a.args[1] = (ofw_cell_t)name;
    a.args[2] = (ofw_cell_t)buf;
    a.args[3] = (ofw_cell_t)len;
    a.args[4] = 0;
    e(&a);
    return (int)a.args[4];
}

static void write_str(ofw_entry_t e, int ih, const char *s)
{
    struct ofw_args a;
    int len = 0;
    const char *p = s;
    while (*p++) len++;
    a.service = "write";
    a.nargs = 3;
    a.nret = 1;
    a.args[0] = (ofw_cell_t)ih;
    a.args[1] = (ofw_cell_t)s;
    a.args[2] = (ofw_cell_t)len;
    a.args[3] = 0;
    e(&a);
}

static void ofw_exit(ofw_entry_t e)
{
    struct ofw_args a;
    a.service = "exit";
    a.nargs = 0;
    a.nret = 0;
    e(&a);
}

static int try_cif(ofw_entry_t e, const char *label)
{
    int chosen, stdout_ih;

    if ((ofw_cell_t)e == 0) return 0;

    chosen = finddevice(e, "/chosen");
    if (chosen == -1) return 0;

    if (getprop(e, chosen, "stdout", &stdout_ih, sizeof(stdout_ih)) < 0)
        return 0;

    write_str(e, stdout_ih, label);
    write_str(e, stdout_ih, " works!\r\n");
    return 1;
}

int main(void)
{
    ofw_entry_t e;
    int found = 0;

    e = (ofw_entry_t)_ofw_r5;
    if (try_cif(e, "r5")) { found = 1; ofw_exit(e); }

    e = (ofw_entry_t)_ofw_r3;
    if (try_cif(e, "r3")) { found = 1; ofw_exit(e); }

    e = (ofw_entry_t)_ofw_r4;
    if (try_cif(e, "r4")) { found = 1; ofw_exit(e); }

    e = (ofw_entry_t)_ofw_r6;
    if (try_cif(e, "r6")) { found = 1; ofw_exit(e); }

    e = (ofw_entry_t)_ofw_r7;
    if (try_cif(e, "r7")) { found = 1; ofw_exit(e); }

    for (;;) ;
    return 0;
}
