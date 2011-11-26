/*
 * Will be started by init via "on property:..." triggers to
 * change the symbolic links of the camera shutter sound to
 * either /dev/null or to the actual location. See
 * get_property_status below for the properties and common.h
 * for the definition of paths and filenames.
 */

#define LOG_TAG "toggleshutter"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <cutils/log.h>
#include <cutils/properties.h>

#include "common.h"

#define error(...) do { \
    LOGE(__VA_ARGS__); \
    exit(1); \
  } while(0)

#define errno_error(fmt, args...) error(fmt ": %s", ##args, strerror(errno))
#define info LOGI
#define warn LOGW

#define safe_snprintf(str, size, format...) do { \
    if (snprintf(str, size, ##format) >= (int)size) \
        error("snprintf at %s:%d would be truncated", __FILE__, __LINE__); \
  } while(0)

#define safe_readlink(path, buf, bufsiz) do_safe_readlink(__FILE__, __LINE__, \
    path, buf, bufsiz)

#define DEVNULL "/dev/null"

typedef enum status_t {
    STATUS_NONE,
    STATUS_UNKNOWN,
    STATUS_ENABLED,  /* sounds enabled */
    STATUS_DISABLED  /* muted */
} status_t;

ssize_t do_safe_readlink(char *file, int line, const char *path, char *buf, size_t bufsiz) {
    ssize_t ret;
    if ((ret = readlink(path, buf, bufsiz)) >= (ssize_t)bufsiz)
        error("readlink at %s:%d would be truncated", file, line);
    return ret;
}

status_t get_link_status(void) {
    struct stat sb;
    status_t status = STATUS_NONE, newstatus;
    int i;

    if (stat(NEWPATH, &sb) == -1) {
        if (errno != ENOENT)
            errno_error("Unable to stat %s", NEWPATH);
        if (mkdir(NEWPATH, 0755) == -1)
            errno_error("Unable to mkdir %s", NEWPATH);
        info("Directory %s created", NEWPATH);
    } else if (!S_ISDIR(sb.st_mode))
        error("%s exists but is not a directory", NEWPATH);


    for (i = 0; i < (int)(sizeof(filenames)/sizeof(filenames[0])); i++) {
        char srcpath[PATH_MAX], dstpath[PATH_MAX], linkdst[PATH_MAX];
        ssize_t ret;

        safe_snprintf(srcpath, sizeof(srcpath), "%s%s", NEWPATH, filenames[i]);
        safe_snprintf(dstpath, sizeof(dstpath), "%s%s", OLDPATH, filenames[i]);

        if ((ret = safe_readlink(srcpath, linkdst, sizeof(linkdst))) == -1) {
            if (errno == ENOENT) {
                if (status != STATUS_NONE) {
                    warn("Inconistent state in %s", NEWPATH);
                }
                return STATUS_UNKNOWN;
            } else
                error("Unable to readlink %s", srcpath);
        }

        newstatus = (strcmp(linkdst, dstpath) == 0) ? STATUS_ENABLED  :
                    (strcmp(linkdst, DEVNULL) == 0) ? STATUS_DISABLED :
                    STATUS_UNKNOWN;
        if (newstatus == STATUS_UNKNOWN)
            warn("%s points to unknown file %s", srcpath, linkdst);

        if (status != STATUS_NONE && status != newstatus) {
                warn("Inconistent state in %s", NEWPATH);
                return STATUS_UNKNOWN;
        }

        status = newstatus;
    }

    return (status == STATUS_NONE) ? STATUS_UNKNOWN : status;
}

int get_boolean_property(const char *key, int default_value) {
    char propval[PROPERTY_VALUE_MAX];
    if (property_get(key, propval, default_value?"1":"0") < 0)
        error("property_get for property %s failed "
            "(is ANDROID_PROPERTY_WORKSPACE set?)", key);
    if (propval[0] == '0' && !propval[1])
        return 0;
    else if (propval[0] == '1' && !propval[1])
        return 1;
    else
        error("value \"%s\" for property %s is neither 0 nor 1",
            propval, key);

   assert(0); return -1; /* unreached */
}

status_t get_property_status(void) {
    return (get_boolean_property("ro.camera.sound.disabled", 0) ||
            get_boolean_property("persist.sys.camera-mute", 0))
            ? STATUS_DISABLED : STATUS_ENABLED;
}

void set_status(status_t status) {
    char srcpath[PATH_MAX], *dstpath, dstpathbuf[PATH_MAX];
    int i;

    assert(status == STATUS_ENABLED || status == STATUS_DISABLED);
    dstpath = (status == STATUS_ENABLED) ? dstpathbuf : DEVNULL;

    for (i = 0; i < (int)(sizeof(filenames)/sizeof(filenames[0])); i++) {
        int retry = 0; /* whether symlink() already failed once with EEXIST */

        safe_snprintf(srcpath, sizeof(srcpath), "%s%s", NEWPATH, filenames[i]);

        /* dstpath points to either "/dev/null" or to dstpathbuf - status
         * won't change during the loop anyway */
        if (status == STATUS_ENABLED)
            safe_snprintf(dstpathbuf, sizeof(dstpathbuf), "%s%s", OLDPATH,
                filenames[i]);

        do {
            if (symlink(dstpath, srcpath) == -1) {
                if (errno == EEXIST && !retry) {
                    if (unlink(srcpath) == -1)
                        errno_error("Unable to unlink %s", srcpath);
                    retry = 1;
                } else {
                    errno_error("Unable to make %s a symlink pointing to %s",
                        srcpath, dstpath);
                }
             } else {
                retry = 0;
            }
         } while(retry);
    }

    info("Camera shutter sound %s", (status == STATUS_ENABLED)
        ? "enabled" : "disabled");
}

int main(void) {
    status_t ls = get_link_status();
    status_t ps = get_property_status();
    assert(ps == STATUS_ENABLED || ps == STATUS_DISABLED);
    if (ls == STATUS_UNKNOWN || ls != ps)
        set_status(ps);

    return 0;
}
