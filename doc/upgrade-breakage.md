---
default_highlighter: oil-sh
---

What Breaks When You Upgrade to Oil
===================================

Only a few things break when you put this at the top of a shell script:

    shopt --set oil:upgrade

This doc enumerates and explains them.

<div id="toc">
</div>

## Reasons for Upgrading

First, let's emphasize the **good** things that happen when you upgrade:

- You can write `if (x > 0)` instead of `if [ "$x" -gt 0 ]`.
- You can pass blocks to commands, like `cd /tmp { echo $PWD }`
- [Simple Word Evaluation](simple-word-eval.html): You can write `$var` instead
  of `"$var"`, and splice arrays with `@myarray`.
- [Reliable Error Handling](error-handling.html) becomes the default.
- ... and more

You can also use `bin/osh` indefinitely, in which case you **don't** need to
read this doc.  [OSH]($xref:osh-language) is a highly compatible Unix shell.

## Syntax Changes

Now onto the breakages.  Most of them are **unlikely**, but worth noting.

### `if ( )` and `while ( )` take expressions, not subshell commands

Code like `if ( ls /tmp )` is valid shell, but it's almost always a **misuse**
of the language.  Parentheses mean **subshell**, not grouping as in C or
Python.

In Oil:

- Use `if (x > 0)` for true/false expressions
- Use the `forkwait` builtin for subshells, which are uncommon.  
  - It's like a sequence of the `fork` builtin (replacing `&`) and the `wait`
    builtin.

No:

    ( not_changed=foo )
    echo $not_changed

Yes:

    forkwait {
      setvar not_changed = 'foo'
    }
    echo $not_changed

Note that the idiom of running commands in a different dir no longer requires
a subshell:

No:

    ( cd /tmp; echo $PWD )
    echo $PWD  # still the original value

Yes:

    cd /tmp {
      echo $PWD 
    }
    echo $PWD  # restored


(Option `parse_paren` is part of group `oil:upgrade`.)

### `@()` is spliced command sub, not extended glob 

Oil doesn't have implicit word splitting, so we want `@(seq 3)` to be
consistent with `$(hostname)`.  They're related in the same way that `@myarray`
and `$mystr` are.

This means that `@()` is no longer extended glob, and `,()` is an alias.

No:

    echo @(*.cc|*.h)

Use this Oil alias instead:

    echo ,(*.cc|*.h)

(Option `parse_at` is part of group `oil:upgrade`.)

### `r'c:\Users\'` is a raw string, not joined strings

The meaning of `\` within string literals can be confusing, so Oil
distinguishes them like this:

- `$'foo\n'` 
  - The `$` prefix means that C-style backslash escapes are respected.
- `r'c:\Users\'` 
  - The `r` prefix means the backslashes are literal.
  - In shell this is written `'c:\Users\'`.  Oil accepts this in command mode
    for compatibility, but not expression mode.

The prefix **changes** the meaning of commands like:

    echo r'foo'
    # => foo in Oil
    # => rfoo in shell, because of implicit joining

Instead, write `'rfoo'` if that's what you mean.

(Option `parse_raw_string` is part of group `oil:upgrade`.)

## Unsupported Syntax

### No Extended Globs in Word Evaluation, but eggex

Like regular globs, the extended glob syntax is used in two ways:

1. Pattern matching 
   - `case` 
   - Bash boolean expressions like `[[ x == !(*.cc|*.h) ]]`
2. Word Evaluation
   - commands like `cp !(*.cc|*.h) /tmp`
   - arrays like `local -a myarray=( !(*.cc|*.h) )`
   - Shell-style `for` loops

Extended globs are **not** supported in [Simple Word
Evaluation](simple-word-eval.html), so you can't use them in the second way
after upgrading.

You may want to use the `find` command or [Egg expressions](eggex.html)
instead.

(Option `simple_word_eval` is part of group `oil:upgrade`.)

## Parsed as Oil Syntax, if not quoted (rarely a problem)

### New first-word keywords (`proc`, `var` etc.)

Oil has new keywords like `proc`, `const`, `var`, and `setvar`.  To use them
as command names, quote them like `'proc'`.

### `=foo` as first-word (too similar to `= foo`)

To avoid confusion with Oil's `=` operator, words starting with `=` can't be the first word in a command.
To invoke such commands, quote them like `'=foo'`.

There is very little reason to use commands like `'proc'` an, `'=x'`, so you
will likely never run into this!

### `@foo` as argument (splices)

Option `parse_at`.  In Oil, `@` is used to splice arrays.  To pass a string
`@foo` to a command, quote it like `'@foo'`.

### `{` as argument (blocks)

Option `parse_brace`.  Braces after commands start block arguments.  To change
to a directory named `{`, quote it like `cd '{'`.

### `=` as argument (bare assignments)

Option `parse_equals`.  A statement like `x = 42` is a "bare assignment" or
attribute.  To pass `=` to a command `x`, quote it like `x '='`.



## Summary

This concludes the list of features that's broken when you upgrade from OSH to
Oil.  We tried to keep this list as small as possible.

There are other features that are **discouraged**, like `$(( x + 1 ))`, `(( i++
))`, `[[ $s =~ $pat ]]`, and `${s%%prefix}`.  These have better alternatives in
the Oil expression language, but they can still be used.  See [Oil vs. Shell
Idioms](idioms.html).

Also related: [Known Differences Between OSH and Other
Shells](known-differences.html).


