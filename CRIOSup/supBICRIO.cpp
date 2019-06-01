
#include <devSup.h>
#include <biRecord.h>
#include <iostream>
#include <stdexcept>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <epicsExport.h>
#include "errlog.h"

#include <CrioLinux.h>

static long crio_bi_init_rec(biRecord *BaseRecord);
static long crio_bi_read(biRecord *BaseRecord);
static long crio_bi_init_dev(int param);

typedef struct {
    long count;
    DEVSUPFUN report;
    DEVSUPFUN init;
    DEVSUPFUN init_record;
    DEVSUPFUN getIointInfo;
    DEVSUPFUN read;
} BIDeviceSupport;

BIDeviceSupport devBICRIO = {
    5,
    NULL,
    (DEVSUPFUN) crio_bi_init_dev,
    (DEVSUPFUN) crio_bi_init_rec,
    NULL,
    (DEVSUPFUN) crio_bi_read,
};

extern "C" { epicsExportAddress(dset, devBICRIO); }

extern struct crio_context *ctx;

static long crio_bi_init_dev(int parm)
{
    return 0;
}

static long crio_bi_init_rec(biRecord *BaseRecord) {
    if (ctx == NULL)
    {
        errlogPrintf("%s ctx global was also null.\n", __func__);
        return -1;
    }
    BaseRecord->dpvt = (void *) ctx;
    return 0;
}

static long crio_bi_read(biRecord *BaseRecord) {
    bool item;
    struct crio_context *ctx;

    ctx = (struct crio_context *)BaseRecord->dpvt;
    auto name = BaseRecord->inp.value.instio.string;

    try{
       crioGetBIArrayItemByName(ctx, &item, name);
    }
    catch (CrioLibException &e) {
        errlogPrintf("Error on BI read - %s \n", e.error_text);
        return -1;
    }
    BaseRecord->rval = item;

    return 0;
}


