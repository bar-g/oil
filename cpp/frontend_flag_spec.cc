// frontend_arg_def.cc

#include "frontend_flag_spec.h"
#include "arg_types.h"

#ifndef CPP_UNIT_TEST
#include "osh_eval.h"  // args::Reader, etc.
#endif

namespace flag_spec {

using arg_types::kFlagSpecs;
using runtime_asdl::value__Bool;
using runtime_asdl::value__Undef;
using runtime_asdl::value_t;

// "Inflate" the static C data into a heap-allocated ASDL data structure.
//
// TODO: Make a GLOBAL CACHE?  It could be shared between subinterpreters even?
runtime_asdl::FlagSpec_* CreateSpec(FlagSpec_c* in) {
  auto out = new runtime_asdl::FlagSpec_();

  if (in->arity0) {
    int i = 0;
    while (true) {
      const char* s = in->arity0[i];
      if (!s) {
        break;
      }
      // log("a0 %s", s);
      out->arity0->append(new Str(s));
      ++i;
    }
  }

  if (in->arity1) {
    int i = 0;
    while (true) {
      SetToArg_c* p = &(in->arity1[i]);
      if (!p->name) {
        break;
      }
      // log("a1 %s", p->name);
      ++i;
    }
  }

  if (in->options) {
    int i = 0;
    while (true) {
      const char* s = in->options[i];
      if (!s) {
        break;
      }
      // log("option %s", s);
      out->options->append(new Str(s));
      ++i;
    }
  }

  if (in->defaults) {
    int i = 0;
    while (true) {
      DefaultPair_c* pair = &(in->defaults[i]);
      if (!pair->name) {
        break;
      }
      // log("default %s", d->name);
      value_t* val;
      switch (pair->default_val) {
      case Default_c::Undef:
        val = new value__Undef();
        break;
      case Default_c::False:
        val = new value__Bool(false);
        break;
      case Default_c::True:
        val = new value__Bool(true);
        break;
      default:
        assert(0);
      }
      out->defaults->set(new Str(pair->name), val);
      ++i;
    }
  }

  return out;
}

runtime_asdl::FlagSpec_* LookupFlagSpec(Str* spec_name) {
  int i = 0;
  while (true) {
    const char* name = kFlagSpecs[i].name;
    if (name == nullptr) {
      break;
    }
    if (str_equals0(name, spec_name)) {
      // log("%s found", spec_name->data_);
      return CreateSpec(&kFlagSpecs[i]);
    }

    i++;
  }
  // log("%s not found", spec_name->data_);
  return nullptr;
}

args::_Attributes* Parse(Str* spec_name, args::Reader* arg_r) {
  runtime_asdl::FlagSpec_* spec = LookupFlagSpec(spec_name);
  assert(spec);  // should always be found

#ifdef CPP_UNIT_TEST
  // hack because we don't want to depend on a translation of args.py
  return nullptr;
#else
  return args::Parse(spec, arg_r);
#endif
}

Tuple2<args::_Attributes*, args::Reader*> ParseCmdVal(
    Str* spec_name, runtime_asdl::cmd_value__Argv* cmd_val) {
#ifdef CPP_UNIT_TEST
  return Tuple2<args::_Attributes*, args::Reader*>(nullptr, nullptr);
#else
  auto arg_r = new args::Reader(cmd_val->argv, cmd_val->arg_spids);
  arg_r->Next();  // move past the builtin name

  runtime_asdl::FlagSpec_* spec = LookupFlagSpec(spec_name);
  assert(spec);  // should always be found
  return Tuple2<args::_Attributes*, args::Reader*>(args::Parse(spec, arg_r),
                                                   arg_r);
#endif
}

Tuple2<args::_Attributes*, args::Reader*> ParseLikeEcho(
    Str* spec_name, runtime_asdl::cmd_value__Argv* cmd_val) {
#ifdef CPP_UNIT_TEST
  return Tuple2<args::_Attributes*, args::Reader*>(nullptr, nullptr);
#else
  auto arg_r = new args::Reader(cmd_val->argv, cmd_val->arg_spids);
  arg_r->Next();  // move past the builtin name

  runtime_asdl::FlagSpec_* spec = LookupFlagSpec(spec_name);
  assert(spec);  // should always be found
  return Tuple2<args::_Attributes*, args::Reader*>(
      args::ParseLikeEcho(spec, arg_r), arg_r);
#endif
}

args::_Attributes* ParseMore(Str* spec_name, args::Reader* arg_r) {
#ifdef CPP_UNIT_TEST
  return nullptr;
#else
  // TODO: Fill this in from constant data!
  flag_spec::_FlagSpecAndMore* spec = nullptr;
  // assert(spec);  // should always be found
  return args::ParseMore(spec, arg_r);
#endif
}

}  // namespace flag_spec
