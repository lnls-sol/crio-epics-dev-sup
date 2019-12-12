/*###############################################################################
#
# This software is distributed under the following ISC license:
#
# Copyright © 2019 BRAZILIAN SYNCHROTRON LIGHT SOURCE <sol@lnls.br>
#   Dawood Alnajjar <dawood.alnajjar@lnls.br>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# Description:
# EPICS Waveform device support
#
###############################################################################*/

#include <devSup.h>
#include <waveformRecord.h>
#include <iostream>
#include <stdexcept>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <epicsExport.h>
#include "errlog.h"
#include "dbAccess.h"
#include <CrioLinux.h>

static long crio_waveform_init_rec(waveformRecord *BaseRecord);
static long crio_waveform_read(waveformRecord *BaseRecord);
static long crio_waveform_init_dev(int param);

typedef struct  { /* waveform dset */
        long            number;
        DEVSUPFUN       dev_report;
        DEVSUPFUN       init;
        DEVSUPFUN       init_record; /*returns: (-1,0)=>(failure,success)*/
        DEVSUPFUN       get_ioint_info;
        DEVSUPFUN       read_wf; /*returns: (-1,0)=>(failure,success)*/
} waveFormDeviceSupport;


waveFormDeviceSupport devWAVEFORMCRIO = {
    5,
    NULL,
    (DEVSUPFUN) crio_waveform_init_dev,
    (DEVSUPFUN) crio_waveform_init_rec,
    NULL,
    (DEVSUPFUN) crio_waveform_read,
};

extern "C" { epicsExportAddress(dset, devWAVEFORMCRIO); }

extern struct crio_context *ctx;

static long crio_waveform_init_dev(int parm)
{
    return 0;
}

static long crio_waveform_init_rec(waveformRecord *BaseRecord) {
    if (ctx == NULL)
    {
        errlogPrintf("%s ctx global was also null.\n", __func__);
        return -1;
    }
    BaseRecord->dpvt = (void *) ctx;
    return 0;
}

static long crio_waveform_read(waveformRecord *BaseRecord) {
    struct crio_context *ctx;

    ctx = (struct crio_context *)BaseRecord->dpvt;
    auto name = BaseRecord->inp.value.instio.string;
    try{
        uint64_t max_size = BaseRecord->nelm * dbValueSize(BaseRecord->ftvl);
        crioGetWaveformItem(ctx, name, BaseRecord->bptr, &(BaseRecord->nord), max_size);
    }
    catch (CrioLibException &e) {
        errlogPrintf("Error on Waveform read - %s \n", e.error_text);
        return -1;
    }
    return 0;

}


