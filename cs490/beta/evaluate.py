# This attempts to execute a student's python function against any number of
# test cases.
# We attempt to catch common mistakes by static anlysis of student source code
# that fails to compile, and then further static analysis of an AST given a
# successful compile but unsuccessful result.
# We also attempt to (minimally) guard against malicious code by stripping
# the global environment and enforcing only a single function definition.
# This also includes disallowing the user to (easily) output over stdout, which
# might allow student code to effectively publish their own grade to the caller.
# Of course, this is not foolproof as os.write(1, 'str') is hard to guard
# against if the student code does manage to get access to __import__.

import argparse, ast, json, sys, keyword, math

# Constants.
global_whitelist = ['__doc__', '__package__']
# Mostly a copy of all __builtins__.__dict__'s keys.
# Normally present values that are removed are denoted by a comment
# NOTE: Keep this list in alphabetical order
# LAST UPDATED FOR VERSION 3.5.2
builtins_whitelist = [
  'abs',
  'all',
  'any',
  'ArithmeticError',
  'ascii',
  'AssertionError',
  'AttributeError',
  'BaseException',
  'bin',
  'BlockingIOError',
  'bool',
  'BrokenPipeError',
  'BufferError',
  '__build_class__',
  'bytearray',
  'bytes',
  'BytesWarning',
  'callable',
  'ChildProcessError',
  'chr',
  'classmethod',
  #'compile',    # Avoid new code creation.
  'complex',
  'ConnectionAbortedError',
  'ConnectionError',
  'ConnectionRefusedError',
  'ConnectionResetError',
  #'copyright',  # Extraneous constant from module site.
  #'credits',    # ibid module site.
  #'__debug__',  # No reason to use debug mode on student code.
  'delattr',
  'DeprecationWarning',
  'dict',
  'dir',
  'divmod',
  '__doc__',
  'Ellipsis',
  'enumerate',
  'EnvironmentError',
  'EOFError',
  #'eval',       # ibid code creation.
  'Exception',
  #'exec',       # ibid code creation.
  #'exit',       # ibid module site.
  'False',
  'FileExistsError',
  'FileNotFoundError',
  'filter',
  'float',
  'FloatingPointError',
  'format',
  'frozenset',
  'FutureWarning',
  'GeneratorExit',
  'getatter',
  'globals',
  'hasattr',
  'hash',
  #'help',       # ibid module site.
  'hex',
  'id',
  #'__import__', # ibid code creation.
  'ImportError',
  'ImportWarning',
  'IndentationError',
  'IndexError',
  'input',
  'int',
  'InterruptedError',
  'IOError',
  'IsADirectoryError',
  'isinstance',
  'issubclass',
  'iter',
  'KeyboardInterrupt',
  'KeyError',
  'len',
  #'license',    # ibid module site.
  'list',
  #'__loader__', # ibid code creation.
  'locals',
  'LookupError',
  'map',
  'max',
  'MemoryError',
  'memoryview',
  'min',
  '__name__',
  'NameError',
  'next',
  'None',
  'NotADirectoryError',
  'NotImplemented',
  'NotImplementedError',
  'object',
  'oct',
  #'open',      # Avoid file i/o.
  'ord',
  'OSError',
  'OverflowError',
  '__package__',
  'PendingDeprecationWarning',
  'PermissionError',
  'pow',
  'print',
  'ProcessLookupError',
  'property',
  #'quit',      # ibid module site.
  'range',
  'RecursionError',
  'ReferenceError',
  'repr',
  'ResourceWarning',
  'reversed',
  'round',
  'RuntimeError',
  'RuntimeWarning',
  'set',
  'setattr',
  'slice',
  'sorted',
  #'__spec__',  # ibid code creation.
  'staticmethod',
  'StopAsyncIteration',
  'StopIteration',
  'str',
  'sum',
  'super',
  'SyntaxError',
  'SyntaxWarning',
  'SystemExit',
  'TabError',
  'TimeoutError',
  'True',
  'tuple',
  'type',
  'TypeError',
  'UnboundLocalError',
  'UnicodeEncodeError',
  'UnicodeWarning',
  'UserWarning',
  'ValueError',
  'vars',
  'Warning',
  'ZeroDivisionError',
  'zip',
]

## OUTPUT FORMAT
# The output of this script will be written to stdout, and will take the form:
# {
#   'score': dd,
#   'deductions': [
#     { 'points': dd, 'reason': str }, ...
#   ]
# }

def output_json(points, deductions):
  # Write out the proper json output to stdout.
  score = points - sum(d['points'] for d in deductions)
  if score < 0: score = 0
  print(json.dumps({'score': score, 'deductions': deductions}))

def dock_points(deductions, points, reason):
  deductions.append({'points': points, 'reason': reason})

def fix_syntax_err(code, err):
  # Try to fix the syntax error found in the code. Returns fixed code or None.
  # TODO: Come up with some rules for fixing syntax errors.
  return None

def grade(code_obj, name, points, test_case_objs, vlevel = 0):
  # Grades the given implementation of name, code_obj, with a maximum score
  # of points on each test_case_obj. Returns a list of deductions.
  deductions = []  # Reasons we took off points.
  points_per_case = points // len(test_case_objs)

  # Instrument globals and locals. This is not leak-proof by anymeans, but
  # helps to limit the attack surface.
  instr_globals = {
    k: globals()[k]
    for k in global_whitelist
    if k in globals()
  }
  instr_globals['__name__'] = name  # Module shouldn't run with name as __main__
  instr_globals['__builtins__'] = {
    k: __builtins__.__dict__[k]
    for k in builtins_whitelist
    if k in __builtins__.__dict__
  }
  instr_locals = {}

  # TODO: instrument stdin, stdout, and stderr for these testcases.
  # TODO: if output is captured from stdout, consider comparing it against desired result

  # Run the function declaration from the student.
  try:
    exec(code_obj, instr_globals, instr_locals)
  except BaseException as e:
    if vlevel >= 1: print(repr(e), file=sys.stderr)
    # We'll consider any exception while defining the function to be a 0.
    dock_points(deductions, points, 'unable to execute function')
    return deductions

  for i, test_case_obj in enumerate(test_case_objs):
    try:
      result = eval(test_case_obj, instr_globals, instr_locals)
      if not result:
        dock_points(deductions, points_per_case, 'failed test case %d' % i)
    except BaseException as e:
      if vlevel >= 1: print(repr(e), file=sys.stderr)
      # We'll consider any exception while executing a test case to be wrong.
      dock_points(deductions, points_per_case, 'exception during test case %d' % i)

  return deductions


def main():
  ## INPUT ARGUMENTS
  parser = argparse.ArgumentParser(description="Grade student code against "
          "given test cases. Results will be written over stdout in JSON")
  parser.add_argument('-n', '--name', required=True,
          help="The name of the function the student was supposed to implement")
  parser.add_argument('-p', '--points', type=int, default=0,
          help="The number of points this question is worth. This argument is "
               "only used when -c is passed, and defaults to 0")
  parser.add_argument('-c', '--code',
          help="The student's code submission. Make sure to carefully escape "
               "this argument as a single string. If this argument is ommitted "
               "then this program just checks the validity of the test cases. "
               "The exit status indicates the validity of the cases")
  parser.add_argument('-t', '--test_case', required=True, action='append',
          help="The test cases to run the students code against. Each test case "
               "must take the form of of a function call without the function "
               "name followed by a comparison to a return value. For example "
               "``(1, 2) == 3'' or ``(1, 2, (3, 4), *[5, 6], last=8) == None''")
  parser.add_argument('-v', '--verbose',  action='count', default=0,
          help="Specifies verbositiy level. Each time this flag is specified, "
               "the count goes up by one. Level 1 or greater outputs additional "
               "information about exceptions that occur")


  # Process input arguments.
  args = parser.parse_args()
  vlevel = args.verbose

  # Ensure that name is a valid identifier (not a keyword).
  name = args.name
  if not name.isidentifier() or keyword.iskeyword(name):
    raise ValueError('Function name is not a valid identifier')

  test_case_objs = []
  # Handle test cases (make sure they are valid)
  for i, test_case in enumerate(args.test_case):
    # Prepend the function name to create a valid expression comparing the result
    # of calling the function with a value.
    test_case = name + test_case
    # Try parsing this into a single expression ast.
    try:
      expr = ast.parse(test_case, mode='eval')
    except BaseException as e:
      if vlevel >= 1: print(repr(e), file=sys.stderr)
      expr = None

    if not expr:
      raise ValueError('Failed to parse test case %d' % i)

    # Validate format of test case.
    # For now we will be pretty strict, we expect exactly:
    # One comparison expression:
    #   where left is a single function call.
    #   and there is only one comparator
    #     which must be a simple literal:
    #       Dict, Set, Num, Str, Bytes, NameConstant, List, Tuple.
    #     or simple operators applied to the above:
    #       BinOp, UnaryOp.
    valid = False
    if type(expr) == ast.Expression:
      comp = expr.body
      if type(comp) == ast.Compare and len(comp.ops) == len(comp.comparators) == 1:
        # We have a valid comparison between exactly two exprs.
        left = comp.left
        right = comp.comparators[0]

        # Validate left side of comparison.
        left_valid = False
        if type(left) == ast.Call:
          if type(left.func) == ast.Name and left.func.id == name:
            # We appear to be calling the right function, TODO: walk args.
            left_valid = True

        # Validate right side of comparison (only if left side was okay).
        if left_valid:
          # TODO: walk nested exprs as needed.
          if type(right) in [ast.Num, ast.Str, ast.Bytes, ast.NameConstant,
                             ast.Dict, ast.Set, ast.List, ast.Tuple,
                             ast.BinOp, ast.UnaryOp]:
            valid = True

    if not valid:
      raise ValueError('Invalid formatting for test case %d' % i)

    # Finally, we just have to try compiling the test case.
    try:
      obj = compile(expr, '<unknown>', 'eval')
    except BaseException as e:
      if vlevel >= 1: print(repr(e), file=sys.stderr)
      obj = None

    if not obj:
      raise ValueError('Cannot compile test case %d' % i)

    test_case_objs.append(obj)

  # Handle student code.
  code = args.code
  if not code:
    # We're just supposed to test the validity of the test cases when code is None.
    # TODO: Consider trying to evaluate the test cases to ensure that in addition to
    #       compiling, they actually evaluate without throwing exceptions.
    return

  # Bookkeeping for score.
  deductions = []

  # Try to construct an AST.
  tree = None
  while not tree:
    try:
      tree = ast.parse(code)
    except SyntaxError as se:
      fixed = fix_syntax_err(code, se)
      if not fixed:
        if vlevel >= 1: print(repr(se), file=sys.stderr)
        dock_points(deductions, args.points, 'syntax error')
        break  # Couldn't fix error.
      code = fixed
    except BaseException as e:
      if vlevel >= 1: print(repr(e), file=sys.stderr)
      dock_points(deductions, args.points, 'failed to parse code')
      break    # Unknown exception.

  if not tree:
    output_json(args.points, deductions)
    return

  # Validate the tree. Should be a single module containing a single function.
  valid = False
  if type(tree) == ast.Module and len(tree.body) == 1:
    fdef = tree.body[0]
    if type(fdef) in [ast.FunctionDef, ast.AsyncFunctionDef]:
      if fdef.name != name:
        fdef.name = name
        # TODO: Solidify point values.
        dock_points(deductions, 1, 'misnamed function')
      # TODO: Consider checking number of arguments and stuff.
      valid = True

  if not valid:
    dock_points(deductions, args.points, 'not just a single function definition')
    output_json(args.points, deductions)
    return

  # TODO: Consider doing more work on the AST to help catch common errors.
  # TODO: Consider multiple alterations to tree to fix common errors.

  # Attempt to compile current code tree.
  try:
    code_obj = compile(tree, '<unknown>', 'exec')
  except BaseException as e:
    if vlevel >= 1: print(repr(e), file=sys.stderr)
    code_obj = None

  # Currently no way to recover from failed compile if tree parsed okay.
  # Full marks off.
  if not code_obj:
    dock_points(deductions, args.points, 'failed to compile code')
    output_json(args.points, deductions)
    return

  # Actually grade the students submission!
  deductions += grade(code_obj, name, args.points, test_case_objs, vlevel)
  output_json(args.points, deductions)

if __name__ == '__main__':
  try:
    main()
  except Exception as e:
    sys.exit(str(e))
