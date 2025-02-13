# Configuration
#
# Hay: Hay Ain't YAML

#### use bin
use
echo status=$?
use z
echo status=$?

use bin
echo bin status=$?
use bin sed grep
echo bin status=$?

## STDOUT:
status=2
status=2
bin status=0
bin status=0
## END

#### use dialect
shopt --set parse_brace

use dialect
echo status=$?

use dialect ninja
echo status=$?

shvar _DIALECT=oops {
  use dialect ninja
  echo status=$?
}

shvar _DIALECT=ninja {
  use dialect ninja
  echo status=$?
}

## STDOUT:
status=2
status=1
status=1
status=0
## END

#### hay builtin usage
shopt --set parse_brace

hay define
echo status=$?

hay define -- package user
echo status=$?

hay pp defs | wc -l | read n
echo read $?
test $n -gt 0
echo greater $?

## STDOUT:
status=2
status=0
read 0
greater 0
## END

#### haynode builtin can define nodes
shopt --set parse_paren parse_brace parse_equals

# It prints JSON by default?  What about the code blocks?
# Or should there be a --json flag?

haynode parent alice {
  age = '50'
  
  haynode child bob {
    age = '10'
  }

  haynode child carol {
    age = '20'
  }

  other = 'str'
}

# _hay_result() is mutated regsiter
var result = _hay_result()

#= result
write -- 'level 0 children' $len(result['children'])
write -- 'level 1 children' $len(result['children'][0]['children']) 

hay clear result

setvar result = _hay_result()
write -- 'level 0 children' $len(result['children'])

## STDOUT:
level 0 children
1
level 1 children
2
level 0 children
0
## END


#### haynode: node name is required
shopt --set parse_brace parse_equals

haynode package
echo status=$?

haynode package {
  version = '1.0'
}
echo status=$?

hay define package

package
echo status=$?

package {
  version = '1.0'
}
echo status=$?

## STDOUT:
status=2
status=2
status=2
status=2
## END

#### haynode: shell nodes require block args; attribute nodes don't

shopt --set parse_brace parse_equals

hay define package TASK

package glibc > /dev/null
echo status=$?

TASK build
echo status=$?

## STDOUT:
status=0
status=2
## END



#### hay define --under
set -o errexit
shopt --set parse_brace parse_equals

hay define package user TASK

hay define --under package -- license

hay pp defs > /dev/null

# shvar PATH=
# shopt --unset allow_side_effects

package cppunit
echo status=$?

package unzip {
  version = '1.0'

  license {
    echo hi
  }
}
echo status=$?

user bob
echo status=$?

TASK build {
  configure
}
echo status=$?

hay pp defs

hay clear defs

hay pp defs

# TODO: Test printing it to JSON?
# Problem: we can't retroactively do hay_eval?

## STDOUT:
status=0
status=0
status=0
status=0
## END


#### parse_hay() 

const config_path = "$REPO_ROOT/spec/testdata/config/ci.oil"
const block = parse_hay(config_path)

# Are blocks opaque?
{
  = block
} | wc -l | read n

# Just make sure we got more than one line?
if test "$n" -gt 1; then
  echo "OK"
fi

## STDOUT:
OK
## END


#### parse_hay() then shvar _DIALECT= { eval_hay() }
shopt --set parse_brace

hay define TASK

const config_path = "$REPO_ROOT/spec/testdata/config/ci.oil"
const block = parse_hay(config_path)

shvar _DIALECT=sourcehut {
  const d = eval_hay(block)
}

const children = d['children']
write 'level 0 children' $len(children) ---
write 'child 0' $[children[0]->type] $[children[0]->name] ---
write 'child 1' $[children[1]->type] $[children[1]->name] ---

#= d 

#pp cell d

## STDOUT:
level 0 children
2
---
child 0
TASK
cpp
---
child 1
TASK
publish-html
---
## END


#### Attribute Blocks

# first words has to be dynamic I think?
#
# push-proc package user {
#   const config = _vm_eval('spec/testdata/config/package-manger.oil')
# }
#
# Implement with ctx_Proc().  Yeah that needs to be a stack just ilke the
# option stack!
#
# Or
#
# push --proc package --proc user --no-external {
#
# }

const path = "$REPO_ROOT/spec/testdata/config/package-manager.oil"
#ls $path

const block = parse_hay(path)

hay define package
const d = eval_hay(block)
write 'level 0 children' $len(d['children'])
write 'level 1 children' $len(d['children'][1]['children'])

## STDOUT:
level 0 children
3
level 1 children
0
## END

#### Turn off external binaries with shvar PATH='' {}
shopt --set parse_brace

echo hi > file

# Note: this CACHES the lookup, so shvar has to clear cache when modifying it
cp -v file /tmp >&2
echo status=$?

# TODO: implement this, and call it whenever shvar mutates PATH?
# what about when PATH is mutated?   No leave it out for now.

# hash -r  # clear the cache, no longer necessary

shvar PATH='' {
  cp -v file /tmp
  echo status=$?

  # this also doesn't work
  command cp -v file /tmp
  echo status=$?
}

# Now it's available again
cp -v file /tmp >&2
echo status=$?

## STDOUT:
status=0
status=127
status=127
status=0
## END

#### More shvar PATH=''
shopt --set parse_brace command_sub_errexit

shvar PATH='' {
  ( cp -v file /tmp >&2 )
  echo status=$?

  forkwait {
    cp -v file /tmp >&2
  }
  echo status=$?

  try {
    true $(cp -v file /tmp >&2)
  }
  echo _status $_status
}

## STDOUT:
status=127
status=127
_status 127
## END

#### Block param binding
shopt --set parse_brace

proc package(name, b Block) {
  = b

  var d = eval_hay(b)

  # NAME and TYPE?
  setvar d->name = name
  setvar d->type = 'package'

  # Now where does d go?


  # Every time you do eval_hay, it clears _config?

  # Another option: HAY_CONFIG

  if ('package_list' not in _config) {
    setvar _config->package_list = []
  }
  _ append(_config->package_list, d)
}

# push-vars (_config) { ?  So it's not global?
# Or make it a register.

package unzip {
  version = 1
}

## STDOUT:
## END


#### Proc that doesn't take a block
shopt --set parse_brace

proc task(name) {
  echo "task name=$name"
}

task foo {
  echo 'running task foo'
}
# This should be an error
echo status=$?

## STDOUT:
status=1
## END
