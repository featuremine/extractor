/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file fmtron_sys.cpp
 * @author Andrus Suvalau
 * @date 13 Apr 2020
 * @brief File contains C++ definitions of the fmtron operator
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "fmtron_sys.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
#include "fmtron.h"
}
#include "fmc++/time.hpp"

#include <algorithm>
#include <list>
#include <string>
#include <vector>
using namespace std;

struct fmtron_sys_cl {
  fmtron_sys_cl() : sys(nullptr), command_op(0) {}
  fmtron_sys_cl(const string &h, const string &u, fmtron_sys *s,
                fm_call_handle_t hdl)
      : host(h), user(u), sys(s), command_op(hdl) {}
  ~fmtron_sys_cl() {
    if (sys != nullptr)
      fmtron_sys_del(sys);
  }
  string host;
  string user;
  fmtron_sys *sys;
  fm_call_handle_t command_op;
};

struct fm_comp_fmtron_closure {
  list<fmtron_sys_cl> sys_list;
};

static fm_comp_fmtron_closure comp_closure;
const fm_comp_def_t fm_comp_fmtron = {"fmtron", &fm_comp_fmtron_gen,
                                      &fm_comp_fmtron_destroy, &comp_closure};

struct fmtron_info {
  fmtron_info(const char *h, const char *u, const char *sn, const char *n,
              const vector<pair<short, string>> &infos)
      : host(h), user(u), srvname(sn), name(n), flds_info(infos) {}
  string host;
  string user;
  string srvname;
  string name;
  vector<pair<short, string>> flds_info;
};

struct fmtron_field_info {
  fm_field_t index;
  size_t len = 0; // used only for strings
};

fmtron_type to_fmtron_type(fm_type_decl_cp type) {
  if (fm_type_is_base(type)) {
    auto fm_type = fm_type_base_enum(type);
    switch (fm_type) {
    case FM_TYPE_INT64:
      return FMTRON_INT;
    case FM_TYPE_UINT64:
      return FMTRON_UINT;
    case FM_TYPE_FLOAT32:
      return FMTRON_FLOAT;
    case FM_TYPE_FLOAT64:
      return FMTRON_REAL;
    case FM_TYPE_TIME64:
      return FMTRON_TIME;
    default:
      return FMTRON_LAST;
    }
  } else if (fm_type_is_array(type)) {
    auto *arr_type = fm_type_array_of(type);
    if (fm_type_is_base(arr_type)) {
      switch (fm_type_base_enum(arr_type)) {
      case FM_TYPE_CHAR:
        return FMTRON_RMTES;
      default:
        return FMTRON_LAST;
      }
    }
  }
  return FMTRON_LAST;
}

struct fmtron_exec_cl {
  fmtron_exec_cl(fmtron_info *info, auto *process_msg, fm_frame_t *f,
                 fm_call_ctx_t *context, fmtron_sys_cl *sys_closure)
      : flds(info->flds_info.size()), flds_info(info->flds_info.size()),
        cln(nullptr), frame(f), ctx(context), updated(false),
        sys_cl(sys_closure) {
    for (size_t i = 0; i < info->flds_info.size(); ++i) {
      flds_info[i].index = fm_frame_field(f, info->flds_info[i].second.c_str());
      auto *type = fm_frame_field_type(f, info->flds_info[i].second.c_str());
      if (fm_type_is_array(type)) {
        flds_info[i].len = fm_type_array_size(type);
      }
      flds[i].id = info->flds_info[i].first;
      flds[i].type = to_fmtron_type(type);
      flds[i].data = nullptr;
    }

    flds.push_back({});
    cln = fmtron_sys_register_client(sys_cl->sys, info->srvname.c_str(),
                                     info->name.c_str(), flds.data(),
                                     process_msg, this);
  }
  ~fmtron_exec_cl() {
    if (sys_cl != nullptr and cln != nullptr) {
      fmtron_sys_unregister_client(sys_cl->sys, cln);
      if (fmtron_sys_client_count(sys_cl->sys) == 0) {
        auto &list = comp_closure.sys_list;
        auto it = find_if(list.begin(), list.end(), [this](const auto &elem) {
          return sys_cl->sys == elem.sys;
        });
        if (it != list.end()) {
          list.erase(it);
        }
      }
    }
  }
  vector<fmtron_field> flds;
  vector<fmtron_field_info> flds_info;
  fmtron_client *cln;
  fm_frame_t *frame;
  fm_call_ctx_t *ctx;
  bool updated;
  fmtron_sys_cl *sys_cl;
};

void process_msg(fmtron_msg *msg, void *closure) {
  auto *exec_cl = (fmtron_exec_cl *)closure;
  auto *s_ctx = (fm_stream_ctx *)exec_cl->ctx->exec;
  auto *frame = exec_cl->frame;
  auto **upds = msg->upds;
  auto *beg = exec_cl->flds.data();

  if (upds == nullptr || *upds == nullptr)
    return;

  for (; *upds != nullptr; ++upds) {
    auto *upd = *upds;
    auto dist = (upd - beg);
    const auto &fld = exec_cl->flds[dist];
    const auto &fld_info = exec_cl->flds_info[dist];
    switch (upd->type) {
    case FMTRON_INT:
      *(int64_t *)fm_frame_get_ptr1(frame, fld_info.index, 0) =
          *(int64_t *)upd->data;
      break;
    case FMTRON_UINT:
      *(uint64_t *)fm_frame_get_ptr1(frame, fld_info.index, 0) =
          *(uint64_t *)upd->data;
      break;
    case FMTRON_FLOAT:
      *(float *)fm_frame_get_ptr1(frame, fld_info.index, 0) =
          *(float *)upd->data;
      break;
    case FMTRON_DOUBLE:
    case FMTRON_REAL:
      *(double *)fm_frame_get_ptr1(frame, fld_info.index, 0) =
          *(double *)upd->data;
      break;
    case FMTRON_ASCII:
    case FMTRON_RMTES: {
      void *data = fm_frame_get_ptr1(frame, fld_info.index, 0);
      memset(data, '\0', fld_info.len * sizeof(char));
      strncpy((char *)data, (const char *)upd->data, fld_info.len);
    } break;
    case FMTRON_TIME:
      *(fmc_time64_t *)fm_frame_get_ptr1(frame, fld_info.index, 0) =
          fmc_time64_from_nanos(*(int64_t *)upd->data);
      break;
    default:
      break;
    }
  }

  exec_cl->updated = true;
  auto handle = exec_cl->ctx->handle;
  if (exec_cl->sys_cl->command_op != handle) {
    auto now = fm_stream_ctx_now(s_ctx);
    fm_stream_ctx_schedule(s_ctx, handle, now);
  }
}

bool fm_comp_fmtron_stream_init(fm_frame_t *result, size_t args,
                                const fm_frame_t *const argv[],
                                fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto *info = (fmtron_info *)ctx->comp;
  auto *exec_ctx = (fm_exec_ctx *)ctx->exec;
  auto size = info->flds_info.size();

  auto &list = comp_closure.sys_list;
  auto it = find_if(list.begin(), list.end(), [info](const auto &elem) {
    return info->host == elem.host && info->user == elem.user;
  });

  fmtron_sys_cl *sys_closure = nullptr;

  if (it == list.end()) {
    auto *fmtrnsys = fmtron_sys_new(info->host.c_str(), info->user.c_str());
    auto fmtrnerr = [fmtrnsys, exec_ctx]() {
      auto *err = fmtron_sys_err_get(fmtrnsys);
      fm_exec_ctx_error_set(exec_ctx, err->msg);
      fmtron_sys_del(fmtrnsys);
      return false;
    };

    if (fmtron_sys_err_has(fmtrnsys))
      return fmtrnerr();

    auto *s_ctx = (fm_stream_ctx *)ctx->exec;
    fm_stream_ctx_queue(s_ctx, ctx->handle);

    list.emplace_back(info->host, info->user, fmtrnsys, ctx->handle);
    sys_closure = &(*list.rbegin());
  } else
    sys_closure = &(*it);

  auto *exec_cl =
      new fmtron_exec_cl(info, &process_msg, result, ctx, sys_closure);

  if (fmtron_sys_err_has(sys_closure->sys)) {
    auto *err = fmtron_sys_err_get(sys_closure->sys);
    fm_exec_ctx_error_set(exec_ctx, err->msg);
    delete exec_cl;
    return false;
  }

  *cl = exec_cl;

  return true;
}

bool fm_comp_fmtron_stream_exec(fm_frame_t *result, size_t,
                                const fm_frame_t *const argv[],
                                fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *exec_cl = (fmtron_exec_cl *)cl;
  auto *s_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exec_ctx = (fm_exec_ctx *)ctx->exec;

  if (exec_cl->sys_cl->command_op == ctx->handle) {
    auto *sys = exec_cl->sys_cl->sys;
    auto res = fmtron_sys_proc_one(sys);
    if (fmtron_sys_err_has(sys)) {
      auto *err = fmtron_sys_err_get(sys);
      fm_exec_ctx_error_set(exec_ctx, err->msg);
      return false;
    }
    auto now = fm_stream_ctx_now(s_ctx);
    fm_stream_ctx_schedule(s_ctx, ctx->handle, now);
  }

  if (exec_cl->updated) {
    exec_cl->updated = false;
    return true;
  }

  return false;
}

void fm_comp_fmtron_stream_destroy(fm_call_exec_cl cl) {
  auto *exec_cl = (fmtron_exec_cl *)cl;
  if (exec_cl)
    delete exec_cl;
}

fm_call_def *fm_comp_fmtron_stream_call(fm_comp_def_cl comp_cl,
                                        const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_fmtron_stream_init);
  fm_call_def_destroy_set(def, fm_comp_fmtron_stream_destroy);
  fm_call_def_exec_set(def, fm_comp_fmtron_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_fmtron_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                 unsigned argc, fm_type_decl_cp argv[],
                                 fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 0) {
    fm_type_sys_err_set(sys, FM_TYPE_ERROR_ARGS);
    return nullptr;
  }

  auto error = [&](const char *errstr) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 5)
    return error("expect a host, user, service name, name and a tuple of field "
                 "descriptions");

  auto *param1 = fm_type_tuple_arg(ptype, 0);
  if (!fm_type_is_cstring(param1))
    return error("expect the 1st parameter to be a host");
  const char *host = STACK_POP(plist, const char *);

  auto *param2 = fm_type_tuple_arg(ptype, 1);
  if (!fm_type_is_cstring(param2))
    return error("expect the 2nd parameter to be a user");
  const char *user = STACK_POP(plist, const char *);

  auto *param3 = fm_type_tuple_arg(ptype, 2);
  if (!fm_type_is_cstring(param3))
    return error("expect the 3rd parameter to be a service name");
  const char *srvname = STACK_POP(plist, const char *);

  auto *param4 = fm_type_tuple_arg(ptype, 3);
  if (!fm_type_is_cstring(param4))
    return error("expect the 4th parameter to be a name");
  const char *name = STACK_POP(plist, const char *);

  auto *fld_descs = fm_type_tuple_arg(ptype, 4);
  if (!fm_type_is_tuple(fld_descs))
    return error(
        "expect the 5th parameter to be a tuple of field descriptions");
  auto size = fm_type_tuple_size(fld_descs);

  auto *str_t = fm_cstring_type_get(sys);
  auto *type_t = fm_type_type_get(sys);
  auto *i64_t = fm_base_type_get(sys, FM_TYPE_INT64);
  auto *fld_desc_t = fm_tuple_type_get(sys, 3, str_t, i64_t, type_t);

  vector<pair<short, string>> flds_info(size);
  const char *names[size];
  fm_type_decl_cp types[size];
  int dims[1] = {1};

  for (unsigned i = 0; i < size; ++i) {
    auto *fld_desc = fm_type_tuple_arg(fld_descs, i);
    if (fm_type_equal(fld_desc, fld_desc_t)) {
      names[i] = STACK_POP(plist, const char *);
      flds_info[i] = make_pair((short)STACK_POP(plist, INT64), names[i]);
      types[i] = STACK_POP(plist, fm_type_decl_cp);
      auto type = to_fmtron_type(types[i]);
      if (type == FMTRON_LAST)
        return error("invalid field type");
    } else
      return error("each field description must contain field name, field id "
                   "and field type");
  }

  auto *type = fm_frame_type_get1(sys, size, names, types, 1, dims);
  if (!type)
    return error("unable to create result frame type");

  auto *ctx_cl = new fmtron_info(host, user, srvname, name, flds_info);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_fmtron_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_fmtron_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (fmtron_info *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
