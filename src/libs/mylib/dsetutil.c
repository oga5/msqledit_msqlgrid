/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

/*
 * dsetutil.c — CSV / TSV dataset export.
 *              MySQL equivalent of pglib's dsetutil.c.
 */

#include <stdio.h>
#include <stdlib.h>

#include "localdef.h"
#include "myapi.h"
#include "mymsg.h"

/* =========================================================================
   csv_fputs — write one field enclosed in double-quotes, doubled internally.
   Appends the separator character after the closing quote.
   ========================================================================= */
static int csv_fputs(FILE *stream, const TCHAR *string, TCHAR sepa)
{
    if (string == NULL) return EOF;

    if (putwc(_T('\"'), stream) == EOF) return EOF;
    for (; *string != _T('\0'); string++) {
        if (putwc(*string, stream) == EOF) return EOF;
        if (*string == _T('\"')) {
            if (putwc(_T('\"'), stream) == EOF) return EOF;
        }
    }
    if (putwc(_T('\"'), stream) == EOF) return EOF;

    putwc(sepa, stream);
    return 1;
}

/* =========================================================================
   Internal helpers
   ========================================================================= */
static int my_save_dataset_main_fp(FILE *fp, HMyDataset dataset,
    int put_colname, TCHAR *msg_buf, TCHAR sepa)
{
    int r, c;

    (void)msg_buf; /* currently unused */

    if (put_colname == 1) {
        for (c = 0; c < dataset->col_cnt; c++) {
            csv_fputs(fp, my_dataset_get_colname(dataset, c), sepa);
        }
        _fputts(_T("\n"), fp);
    }

    for (r = 0; r < dataset->row_cnt; r++) {
        for (c = 0; c < dataset->col_cnt; c++) {
            csv_fputs(fp, my_dataset_data(dataset, r, c), sepa);
        }
        _fputts(_T("\n"), fp);
    }

    return 0;
}

static int my_save_dataset_main(const TCHAR *path, HMyDataset dataset,
    int put_colname, TCHAR *msg_buf, TCHAR sepa)
{
    FILE *fp;
    int   ret_v;

    if ((fp = _tfopen(path, _T("w"))) == NULL) {
        if (msg_buf != NULL) _stprintf(msg_buf, MYERR_OPEN_FILE_MSG, path);
        return MYERR_OPEN_FILE;
    }

    ret_v = my_save_dataset_main_fp(fp, dataset, put_colname, msg_buf, sepa);
    if (ret_v != 0) {
        fclose(fp);
        return ret_v;
    }

    if (fclose(fp) != 0) {
        if (msg_buf != NULL) _tcscpy(msg_buf, MYERR_CLOSE_FILE_MSG);
        return MYERR_CLOSE_FILE;
    }

    return 0;
}

/* =========================================================================
   Public API
   ========================================================================= */
int my_save_dataset_csv(const TCHAR *path, HMyDataset dataset,
    int put_colname, TCHAR *msg_buf)
{
    return my_save_dataset_main(path, dataset, put_colname, msg_buf, _T(','));
}

int my_save_dataset_csv_fp(FILE *fp, HMyDataset dataset,
    int put_colname, TCHAR *msg_buf)
{
    return my_save_dataset_main_fp(fp, dataset, put_colname, msg_buf, _T(','));
}

int my_save_dataset_tsv(const TCHAR *path, HMyDataset dataset,
    int put_colname, TCHAR *msg_buf)
{
    return my_save_dataset_main(path, dataset, put_colname, msg_buf, _T('\t'));
}

int my_save_dataset_ex(const TCHAR *path, HMyDataset dataset,
    int put_colname, TCHAR *msg_buf, int col_cnt, TCHAR sepa)
{
    int ret_v;
    int org_col_cnt   = dataset->col_cnt;
    dataset->col_cnt  = col_cnt;
    ret_v             = my_save_dataset_main(path, dataset, put_colname,
                            msg_buf, sepa);
    dataset->col_cnt  = org_col_cnt;
    return ret_v;
}
