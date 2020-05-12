#!/usr/bin/env python2
"""
front_end_test.py: Tests for front_end.py
"""
from __future__ import print_function

import cStringIO
import unittest

from asdl import front_end  # module under test
from asdl import asdl_


class FrontEndTest(unittest.TestCase):

  def testLoadSchema(self):
    with open('asdl/typed_demo.asdl') as f:
      schema_ast, type_lookup = front_end.LoadSchema(f, {}, verbose=True)
    #print(type_lookup)

    # Test fully-qualified name
    self.assertTrue('bool_expr__LogicalNot' in type_lookup)
    self.assertTrue('op_id__Plus' in type_lookup)

  def testSharedVariant(self):
    with open('asdl/shared_variant.asdl') as f:
      schema_ast, type_lookup = front_end.LoadSchema(f, {}, verbose=False)
    #print(type_lookup)

  def testSharedVariantCode(self):
    # generated by build/dev.sh minimal
    from _devbuild.gen.shared_variant_asdl import (
        double_quoted, expr, expr_e, word_part, word_part_e
    )
    print(double_quoted)

    print(expr)
    print(expr_e)

    print(word_part)
    print(word_part_e)

    # These have the same value!
    self.assertEqual(1001, expr_e.DoubleQuoted)
    self.assertEqual(expr_e.DoubleQuoted, word_part_e.DoubleQuoted)

    d = double_quoted(5, ['foo', 'bar'])
    d.PrettyPrint()
    print()

    b = expr.Binary(d, d)
    b.PrettyPrint()

  def _assertParse(self, code_str):
    f = cStringIO.StringIO(code_str)
    p = front_end.ASDLParser()
    schema_ast = p.parse(f)
    print(schema_ast)
    # For now just test its type
    self.assert_(isinstance(schema_ast, asdl_.Module))

  def testParse(self):
    self._assertParse("""
module foo {
  point = (int? x, int* y)
  action = Foo | Bar(point z)
  foo = (array[int] span_ids)
  bar = (map[string, int] options)
}
""")

  def _assertParseError(self, code_str):
    f = cStringIO.StringIO(code_str)
    p = front_end.ASDLParser()
    try:
      schema_ast = p.parse(f)
    except front_end.ASDLSyntaxError as e:
      print(e)
    else:
      self.fail("Expected parse failure: %r" % code_str)

  def testParseErrors(self):
    # Need field name
    self._assertParseError('module foo { t = (int) }')

    # Need []
    self._assertParseError('module foo { t = (array foo) }')

    # Shouldn't have []
    self._assertParseError('module foo { t = (string[string] a) }')

    # Not enough params
    self._assertParseError('module foo { t = (map[] a) }')
    self._assertParseError('module foo { t = (map[string] a) }')
    self._assertParseError('module foo { t = (map[string, ] a) }')

  def testAstNodes(self):
    # maybe[string]
    n1 = asdl_.TypeExpr('string')
    print(n1)

    n2 = asdl_.TypeExpr('maybe', [n1])
    print(n2)

    n3 = asdl_.TypeExpr('map', [n1, asdl_.TypeExpr('int')])
    print(n3)



if __name__ == '__main__':
  unittest.main()
