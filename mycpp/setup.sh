#!/bin/bash
#
# Toil build steps.
#
# TODO: Rename to toil-tasks.sh?
#
# Usage:
#   ./setup.sh <function name>

set -o nounset
set -o pipefail
set -o errexit

readonly THIS_DIR=$(dirname $(readlink -f $0))
readonly REPO_ROOT=$THIS_DIR/..
readonly MYPY_REPO=$REPO_ROOT/_clone/mypy

clone() {
  local out=$MYPY_REPO
  mkdir -p $out
  git clone --recursive --depth=50 --branch=release-0.730 \
    https://github.com/python/mypy $out
}

# From run.sh
deps() {
  export MYPY_REPO  # mypy-deps function uses this

  pushd $THIS_DIR

  ./run.sh create-venv

  set +o nounset
  set +o pipefail
  set +o errexit
  source _tmp/mycpp-venv/bin/activate

  ./run.sh mypy-deps      # install deps in virtual env

  popd
}

build() {
  export MYPY_REPO  # build/mycpp.sh uses this

  build/dev.sh oil-cpp
}

all-examples() {
  # mycpp_main.py needs to find it
  export MYPY_REPO
  # Don't use clang for benchmarks.
  export CXX=c++

  cd $THIS_DIR
  ./configure.py

  set +o errexit
  ninja
  local status=$?
  set -o errexit

  find _ninja -type f > _ninja/index.txt
  echo 'mycpp/setup.sh travis done'

  # Now we want to zip up
  return $status
}

travis() {
  # invoked by services/toil-worker.sh
  all-examples
}

run-for-release() {
  # invoked by devtools/release.sh

  rm --verbose -r -f _ninja
  all-examples

  # TODO: harness.sh benchmark-all creates ../_tmp/mycpp-examples/raw/times.tsv
  # It compares C++ and Python.
  #
  # So we need to do the same with 'cat' and some magic.
}

"$@"
