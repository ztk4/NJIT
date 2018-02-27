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

import argparse, ast, json

def fix_syntax_err(code, err):
  # Try to fix the syntax error found in the code. Returns fixed code or None.
  # TODO: Come up with some rules for fixing syntax errors.
  return None

def output_json(points, deductions):
  # Write out the proper json output to stdout.
  score = points - sum(d['points'] for d in deductions)
  if score < 0: score = 0
  print(json.dumps({'score': score, 'deductions': deductions}))

## OUTPUT FORMAT
# The output of this script will be written to stdout, and will take the form:
# {
#   'score': dd,
#   'deductions': [
#     { 'points': dd, 'reason': str }, ...
#   ]
# }

def main():
  ## INPUT ARGUMENTS
  parser = argparse.ArgumentParser(description="Grade student code against "
          "given test cases. Results will be written over stdout in JSON")
  parser.add_argument('-n', '--name', required=True,
          help="The name of the function the student was supposed to implement")
  parser.add_argument('-p', '--points', type=int, required=True,
          help="The number of points this question is worth")
  parser.add_argument('-c', '--code', required=True,
          help="The student's code submission. Make sure to carefully escape "
               "this argument as a single string")
  parser.add_argument('-t', '--test_case', required=True, action='append',
          help="The test cases to run the students code against. Each test case "
               "must take the form of of a function call without the function "
               "name followed by a comparison to a return value. For example "
               "``(1, 2) == 3'' or ``(1, 2, (3, 4), *[5, 6], last=8) == None''")

  # Process input arguments.
  args = parser.parse_args()
  code = args.code

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
        deductions.append({'points': args.points, 'reason': 'syntax error'})
        break  # Couldn't fix error.
      code = fixed
    except:
      deductions.append({'points': args.points, 'reason': 'failed to compile'})
      break    # Unknown exception.

  if not tree:
    output_json(args.points, deductions)
    return

  # Validate the tree. Should be a single module containing a single function.
  valid = False
  if type(tree) == ast.Module and len(tree.body) == 1:
    fdef = tree.body[0]
    if type(fdef) in [ast.FunctionDef, ast.AsyncFunctionDef]:
      if fdef.name != args.name:
        fdef.name = args.name
        # TODO: Solidify point values.
        deductions.append({'points': 1, 'reason': 'misnamed function'})
      # TODO: Consider checking number of arguments and stuff.
      valid = True

  if not valid:
    deductions.append({'points': args.points,
                       'reason': 'not just a single function definition'})

  # TODO: Consider doing more work on the AST to help catch common errors.

  # TODO: grade

  output_json(args.points, deductions)

if __name__ == '__main__':
  main()
