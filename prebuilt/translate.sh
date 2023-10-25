#!/usr/bin/env bash
#
# Translate parts of Oil with mycpp, to work around circular deps issue.
#
# Usage:
#   prebuilt/translate.sh <function name>

set -o nounset
set -o pipefail
set -o errexit

REPO_ROOT=$(cd "$(dirname $0)/.."; pwd)

source mycpp/common.sh       # MYPY_REPO
source devtools/run-task.sh  # run-task
source build/ninja-rules-cpp.sh

readonly TEMP_DIR=_build/tmp

oil-part() {
  ### Translate ASDL deps for unit tests

  local out_prefix=$1
  local raw_header=$2
  local guard=$3
  local more_include=$4
  shift 4

  local name=asdl_runtime
  local raw=$TEMP_DIR/${name}_raw.cc 

  mkdir -p $TEMP_DIR

  local mypypath=$REPO_ROOT

  local mycpp=_bin/shwrap/mycpp_main

  ninja $mycpp
  $mycpp \
    $mypypath $raw \
    --header-out $raw_header \
    "$@"

  { 
    echo "// $out_prefix.h: GENERATED by mycpp"
    echo
    echo "#ifndef $guard"
    echo "#define $guard"
    echo
    echo '#include "_gen/asdl/hnode.asdl.h"'
    echo '#include "cpp/qsn.h"'
    echo '#include "mycpp/runtime.h"'
    echo "$more_include"

    cat $raw_header

    echo "#endif  // $guard"

  } > $out_prefix.h

  { cat <<EOF
// $out_prefix.cc: GENERATED by mycpp

#include "$out_prefix.h"
EOF
    cat $raw

  } > $out_prefix.cc
}

readonly -a ASDL_FILES=(
  $REPO_ROOT/{asdl/runtime,asdl/format,core/ansi,pylib/cgi,data_lang/qsn}.py \
)

asdl-runtime() {
  mkdir -p prebuilt/asdl $TEMP_DIR/asdl
  oil-part \
    prebuilt/asdl/runtime.mycpp \
    $TEMP_DIR/asdl/runtime_raw.mycpp.h \
    ASDL_RUNTIME_MYCPP_H \
    '' \
    --to-header asdl.runtime \
    --to-header asdl.format \
    "${ASDL_FILES[@]}"
}

core-error() {
  # Depends on frontend/syntax_asdl

  mkdir -p prebuilt/core $TEMP_DIR/core
  oil-part \
    prebuilt/core/error.mycpp \
    $TEMP_DIR/core/error.mycpp.h \
    CORE_ERROR_MYCPP_H \
    '
#include "_gen/core/runtime.asdl.h"
#include "_gen/core/value.asdl.h"
#include "_gen/frontend/syntax.asdl.h"' \
    --to-header core.error \
    core/error.py
}

frontend-args() {
  # Depends on core/runtime_asdl

  mkdir -p prebuilt/frontend $TEMP_DIR/frontend
  oil-part \
    prebuilt/frontend/args.mycpp \
    $TEMP_DIR/frontend/args_raw.mycpp.h \
    FRONTEND_ARGS_MYCPP_H \
    '
#include "_gen/core/runtime.asdl.h"
#include "_gen/core/value.asdl.h"
#include "_gen/frontend/syntax.asdl.h"
#include "cpp/frontend_flag_spec.h"' \
    --to-header asdl.runtime \
    --to-header asdl.format \
    --to-header frontend.args \
    "${ASDL_FILES[@]}" \
    core/error.py \
    frontend/args.py
}

all() {
  asdl-runtime
  core-error
  frontend-args
}

deps() {
  PYTHONPATH='.:vendor' \
    python2 -c 'import sys; from frontend import args; print(sys.modules.keys())'

  PYTHONPATH='.:vendor' \
    python2 -c 'import sys; from core import error; print(sys.modules.keys())'
}

run-task "$@"
