# Test out Oil's JSON support.

#### json write STRING
shopt --set parse_proc

json write ('foo')
var s = 'foo'
json write (s)
## STDOUT:
"foo"
"foo"
## END

#### json write ARRAY
json write (%(foo.cc foo.h))
json write --indent 0 (['foo.cc', 'foo.h'])
## STDOUT:
[
  "foo.cc",
  "foo.h"
]
[
"foo.cc",
"foo.h"
]
## END

#### json write compact format
shopt --set parse_proc

# TODO: ORDER of keys should be PRESERVED
var mydict = {name: "bob", age: 30}

json write --pretty=0 (mydict)
# ignored
json write --pretty=F --indent 4 (mydict)
## STDOUT:
{"age":30,"name":"bob"}
{"age":30,"name":"bob"}
## END

#### json write in command sub
shopt -s oil:all  # for echo
var mydict = {name: "bob", age: 30}
json write (mydict)
var x = $(json write (mydict))
echo $x
## STDOUT:
{
  "age": 30,
  "name": "bob"
}
{
  "age": 30,
  "name": "bob"
}
## END

#### json read passed invalid args
json read
echo status=$?
json read 'z z'
echo status=$?
json read a b c
echo status=$?
## STDOUT:
status=2
status=2
status=2
## END


#### json read with redirect
echo '{"age": 42}'  > $TMP/foo.txt
json read :x < $TMP/foo.txt
pp cell :x
## STDOUT:
x = (cell exported:F readonly:F nameref:F val:(value.Obj obj:{'age': 42}))
## END

#### json read at end of pipeline (relies on lastpipe)
echo '{"age": 43}' | json read :y
pp cell y
## STDOUT:
y = (cell exported:F readonly:F nameref:F val:(value.Obj obj:{'age': 43}))
## END

#### invalid JSON
echo '{' | json read :y
echo pipeline status = $?
pp cell y
## status: 1
## STDOUT:
pipeline status = 1
## END

#### json write expression
json write --pretty=0 ([1,2,3])
echo status=$?

json write (5, 6)  # to many args
echo status=$?
## STDOUT:
[1,2,3]
status=0
status=2
## END

#### json write evaluation error

#var block = ^(echo hi)
#json write (block) 
#echo status=$?

# undefined var
json write (a) 
echo status=$?

## status: 1
## STDOUT:
## END
