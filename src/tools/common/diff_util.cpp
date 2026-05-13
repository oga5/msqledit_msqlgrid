/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#include "stdafx.h"

#include <vector>
#include "diff_util.h"
#include "GridData.h"

struct _diff_path_st {
	int		pre;
	int		x;
	int		y;
};
typedef std::vector<struct _diff_path_st> path_v;

struct _diff_list_st {
	int		x;
	int		y;
};
typedef std::vector<struct _diff_list_st> list_v;

static bool data_equal(CGridData *a, int a_idx, CGridData *b, int b_idx)
{
	if(a->Get_ColCnt() != b->Get_ColCnt()) return false;

	int		col_cnt = a->Get_ColCnt();
	int		c;
	for(c = 0; c < col_cnt; c++) {
		if(_tcscmp(a->Get_ColData(a_idx, c), b->Get_ColData(b_idx, c)) != 0) return false;
	}

	return true;
}

static int cnt = 0;

static void snake(int_v *fp, int_v *lst, path_v *path, int k, 
	CGridData *a, CGridData *b, int m, int n, int a_start, int b_start)
{
	cnt++;

	int y = (*fp)[(size_t)k - 1] + 1;
	int pre;

	if(y > (*fp)[(size_t)k + 1]) {
		pre = (*lst)[(size_t)k - 1];
	} else {
		y = (*fp)[(size_t)k + 1];
		pre = (*lst)[(size_t)k + 1];
	}

    int x = y - (k - (m + 1));
    while(x < m && y < n && data_equal(a, x + a_start, b, y + b_start)) {
        x++;
        y++;
    }

    (*fp)[k] = y;
	int pt = (int)path->size();
	struct _diff_path_st path_data = { pre, x, y };
	path->push_back(path_data);
    (*lst)[k] = pt;
}


/* path座標をこの件数まで保持し、超えた場合は途中結果を確定して再開する */
#define MAX_COORDINATES_SIZE	2000
/* 再開境界の揺らぎを吸収するため、直近の操作をこの件数だけ再計算対象に戻す */
#define RESUME_ROLLBACK_OPS	128
/* 再開時に最低1ステップは前進を残し、同一点再開によるループを防ぐ */
#define MIN_RESUME_PROGRESS	1

static void rollback_result_tail(diff_result_v *result,
	int *x0, int *y0, int *diff_cnt,
	enum diff_result_status del_cmd, enum diff_result_status ins_cmd,
	int a_start, int b_start)
{
	int rollback_cnt = 0;
	/* x/y のどちらか一方のみ進んだ区間（insert/delete連続）も巻き戻せるよう OR 条件にする */
	while (!result->empty() && rollback_cnt < RESUME_ROLLBACK_OPS
		&& (*x0 > a_start || *y0 > b_start)
		&& ((*x0 - a_start) + (*y0 - b_start) > MIN_RESUME_PROGRESS))
	{
		struct _diff_result_st tail = result->back();
		result->pop_back();

		/* 各座標は個別の境界チェック付きで減算し、a_start/b_start 未満にしない */
		if (tail.cmd == DIFF_COMMON) {
			if (*x0 > a_start) (*x0)--;
			if (*y0 > b_start) (*y0)--;
		} else if (tail.cmd == del_cmd) {
			if (*x0 > a_start) (*x0)--;
			if (*diff_cnt > 0) (*diff_cnt)--;
		} else if (tail.cmd == ins_cmd) {
			if (*y0 > b_start) (*y0)--;
			if (*diff_cnt > 0) (*diff_cnt)--;
		}

		rollback_cnt++;
	}
}

static diff_result_v *diff_onp(CGridData *a, CGridData *b, 
	enum diff_result_status del_cmd, enum diff_result_status ins_cmd,
	int *diff_cnt, bool swap_flg)
{
    int m = a->Get_RowCnt();
    int n = b->Get_RowCnt();
	
	int a_start = 0;
	int b_start = 0;
	int delta = n - m;

	*diff_cnt = 0;

	int_v	fp_vec((size_t)n + m + 3), lst_vec((size_t)n + m + 3);
	path_v	path_vec;
	path_vec.reserve((size_t)n + m);

	int_v	*fp = &fp_vec;
	int_v	*lst = &lst_vec;
	path_v	*path = &path_vec;

	diff_result_v *result = new diff_result_v;
	int	i, p, k;

ONP:
	for(i = 0; i <= n + 1 + m + 1; i++) {
        (*fp)[i] = -1;
        (*lst)[i] = -1;
    }

    for(p = 0; p <= m; p++) {
        for(k = -p; k < delta; k++) {
            snake(fp, lst, path, k + m + 1, a, b, m, n, a_start, b_start);
        }
        for(k = delta + p; k > delta; k--) {
            snake(fp, lst, path, k + m + 1, a, b, m, n, a_start, b_start);
        }
        snake(fp, lst, path, delta + m + 1, a, b, m, n, a_start, b_start);

		bool end_flg = ((*fp)[(size_t)delta + m + 1] >= n);

        if(end_flg || path->size() > MAX_COORDINATES_SIZE) {
            int pt = (*lst)[(size_t)delta + m + 1];
			list_v	list;

            while(pt >= 0) {
				struct _diff_list_st list_data = { (*path)[pt].x + a_start, (*path)[pt].y + b_start };
				list.push_back(list_data);
                pt = (*path)[pt].pre;
            }

            int x0 = a_start;
            int y0 = b_start;
			struct _diff_result_st result_data;
			
            for(i = (int)(list.size() - 1); i >= 0; i--) {
                int x1 = list[i].x;
                int y1 = list[i].y;

                while(x0 < x1 || y0 < y1) {
					if(swap_flg) {
						result_data.row1 = y0;
						result_data.row2 = x0;
					} else {
						result_data.row1 = x0;
						result_data.row2 = y0;
					}

                    if(y1 - x1 > y0 - x0) {
						result_data.cmd = ins_cmd;
						result->push_back(result_data);
						y0++;

						(*diff_cnt)++;
                    } else if(y1 - x1 < y0 - x0) {
						result_data.cmd = del_cmd;
						result->push_back(result_data);
						x0++;

						(*diff_cnt)++;
                    } else {
						result_data.cmd = DIFF_COMMON;
						result->push_back(result_data);

						x0++;
                        y0++;
                    }
                }
            }
            if(end_flg) return result;

			rollback_result_tail(result, &x0, &y0, diff_cnt, del_cmd, ins_cmd,
				a_start, b_start);

			a_start = x0;
			b_start = y0;
			m = a->Get_RowCnt() - a_start;
			n = b->Get_RowCnt() - b_start;
			delta = n - m;
			path->resize(0);

			goto ONP;
		}
	}

	delete result;
	return NULL;
}

diff_result_v *grid_diff(CGridData *a, CGridData *b, int *diff_cnt)
{
    if(a->Get_RowCnt() <= b->Get_RowCnt()) {
        return diff_onp(a, b, DIFF_DELETE, DIFF_INSERT, diff_cnt, false);
    } else {
        return diff_onp(b, a, DIFF_INSERT, DIFF_DELETE, diff_cnt, true);
    }
}
