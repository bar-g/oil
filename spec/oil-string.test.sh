#### single quoted -- implicit and explicit raw
var x = 'foo bar'
echo $x
setvar x = r'foo bar'  # Same string
echo $x
setvar x = r'\t\n'  # This is raw
echo $x
## STDOUT:
foo bar
foo bar
\t\n
## END

#### Implicit raw single quote with backslash is a syntax error
var x = '\t\n'
echo $x
## status: 2
## stdout-json: ""

#### single quoted C strings: $'foo\n'

# expression mode
var x = $'foo\nbar'
echo "$x"

# command mode
if test "$x" = $'foo\nbar'; then
  echo equal
fi

## STDOUT:
foo
bar
equal
## END

#### Double Quoted
var name = 'World'
var g = "Hello $name"

echo "Hello $name"
echo $g
## STDOUT:
Hello World
Hello World
## END

#### Multiline strings with '' and ""

var single = '
  single
'

var x = 42
var double = "
  double $x
"

echo $single
echo $double

## STDOUT:

  single


  double 42

## END

#### C strings in %() array literals
shopt -s oil:upgrade

var lines=%($'aa\tbb' $'cc\tdd')
write @lines

## STDOUT:
aa	bb
cc	dd
## END

#### shopt parse_raw_string

# Ignored prefix
echo r'\'

# space
write r '' end

# These use shell rules!
echo ra'\'
echo raw'\'

echo r"\\"

# Now it's a regular r
shopt --unset parse_raw_string
write unset r'\'

## STDOUT:
\
r

end
ra\
raw\
r\
unset
r\
## END

#### Triple Double Quotes, Expression Mode

var line1 = """line1"""
echo line1=$line1
var line2 = """
line2"""
echo line2=$line2

var two = 2
var three = 3
var x = """
  one "
  two = $two ""
   three = $three
  """
echo "[$x]"

var x = """
  good
 bad
  """
echo "[$x]"

## STDOUT:
line1=line1
line2=line2
[one "
two = 2 ""
 three = 3
]
[good
 bad
]
## END

#### Triple Single Quotes, Expression Mode

var two = 2
var three = 2

var x = ''' 
  two = $two '
  three = $three ''
   \u{61}
  '''
echo "[$x]"

var x = $''' 
  two = $two '
  three = $three ''
   \u{61}
  '''
echo "[$x]"

## STDOUT:
[two = $two '
three = $three ''
 \u{61}
]
[two = $two '
three = $three ''
 a
]
## END


#### Triple Double Quotes, Command Mode

var two=2
var three=3

echo ""a  # test lookahead

echo --
echo """
  one "
  two = $two ""
  three = $three
  """

echo --
tac <<< """
  one "
  two = $two ""
  three = $three
  """

shopt --unset parse_triple_quote

echo --
echo """
  one
  two = $two
  three = $three
  """


## STDOUT:
a
--
one "
two = 2 ""
three = 3

--

three = 3
two = 2 ""
one "
--

  one
  two = 2
  three = 3
  
## END

#### raw strings and triple quotes

echo r'''a'''

shopt --unset parse_raw_string

echo r'''a'''

## STDOUT:
a
ra
## END


#### Triple Single Quotes, Command Mode

echo ''a  # make sure lookahead doesn't mess up

echo --
echo '''
  two = $two
  '
  '' '
  \u{61}
  '''
## STDOUT:
a
--
two = $two
'
'' '
\u{61}

## END

#### Triple Single Quotes, Here Doc

tac <<< '''
  two = $two
  '
  '' '
  \u{61}
  '''

## STDOUT:

\u{61}
'' '
'
two = $two
## END

#### Triple Single Quotes, disabled

shopt --unset parse_triple_quote

echo '''
  two = $two
  \u{61}
  '''

## STDOUT:

  two = $two
  \u{61}
  
## END


#### $''' in command mode

echo $'''
  two = $two
  '
  '' '
  \u{61}
  '''

## STDOUT:
two = $two
'
'' '
a

## END

#### here doc with quotes

# This has 3 right double quotes

cat <<EOF
"hello"
""
"""
EOF


## STDOUT:
"hello"
""
"""
## END

#### triple quoted and implicit concatenation

# Should we allow this?  Or I think it's possible to make it a syntax error

echo '''
single
'''zz

echo """
double
"""zz
## status: 2
## stdout-json: ""
