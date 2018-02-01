#ifndef user_def_fun_h
#define user_def_fun_h

#include "bui_pkg_fun.h"

typedef enum app_user_def{
  APP_USER_DEF_SUCCESS,
  NO_USER_FUN_FAIL,
  APP_TIME_PROC_FAIL,
  APP_REMOTER_CTRL_FAIL,
	APP_HANGXUN_CTRL_FAIL
}app_user_def_t;

app_status_t user_def_proc(app_pkg_t *,uint8_t *,uint8_t *);       


app_user_def_t time_proc(app_pkg_t *,uint8_t *, uint8_t *);
app_user_def_t no_user_fun(app_pkg_t *,uint8_t *, uint8_t *);

#endif

