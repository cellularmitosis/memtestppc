#ifndef OFW_H
#define OFW_H

typedef unsigned long ofw_cell_t;
typedef int ofw_handle_t;
typedef int ofw_ihandle_t;

extern ofw_cell_t _ofw_cif_store;

int ofw_call(const char *service, int nargs, int nret, ...);
ofw_handle_t ofw_finddevice(const char *path);
int ofw_getprop(ofw_handle_t phandle, const char *name, void *buf, int buflen);
ofw_ihandle_t ofw_open(const char *path);
int ofw_call_method_1(const char *method, ofw_ihandle_t ihandle, int arg1);
void ofw_exit(void);
void ofw_reset(void);
int ofw_write(ofw_ihandle_t ihandle, const void *buf, int len);
void ofw_puts(const char *s);

/* Memory */
void *ofw_claim(void *addr, unsigned int size, unsigned int align);

/* Keyboard */
ofw_ihandle_t ofw_get_stdin(void);
int ofw_read(ofw_ihandle_t ihandle, void *buf, int len);

/* Framebuffer info */
struct ofw_fb_info {
    unsigned long addr;
    int width;
    int height;
    int depth;
    int linesize;
};

int ofw_get_fb_info(struct ofw_fb_info *info);
unsigned long ofw_get_timebase_freq(void);

#endif
