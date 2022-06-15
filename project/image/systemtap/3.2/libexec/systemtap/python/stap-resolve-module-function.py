from __future__ import print_function

import ast
import imp
import os.path
import re
import sys
from glob import glob


_verbose = 0


# We want everything that isn't actual results to go to
# stderr. _eprint() is just like print(), but all output automatically
# goes to stderr.
def _eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def _remove_ext(fullname):
    return os.path.splitext(fullname)[0]

_alphanum = frozenset(
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")
_regexp_chars = frozenset("*?[]")


def _stapre_escape(pattern):
    "Escape all unallowed wildcard characters in pattern."
    s = list(pattern)
    alphanum = _alphanum
    for i, c in enumerate(pattern):
        if c not in alphanum and c not in _regexp_chars:
            if c == "\000":
                s[i] = "\\000"
            else:
                s[i] = "\\" + c
    for i, c in enumerate(pattern):
        # Convert '*' into '.*', a real regexp.
        if c == '*':
            s[i] = '[^.]*'
        # Convert '?' into '.', a real regexp.
        elif c == '?':
            s[i] = '.'
    return pattern[:0].join(s)


def _stapre_name_has_wildcard(pattern):
    for i, c in enumerate(pattern):
        if c in _regexp_chars:
            return True
    return False


def _get_default_search_path():
    path = sys.path
    # The first element of sys.path is the current directory of this
    # script, not the user's current directory. So, insert the empty
    # string (standing in for the current directory) at the start of
    # the list.
    path.insert(0, '')
    return path


def _find_wildcarded_modules(modpattern, path=None):
    """Version of imp.find_module() that handles hierarchical module
    names and wildcards.

    Try to find the module name. If path is omitted or None, the
    default search path is used. Otherwise, path must be a list of
    directory names; each directory is searched for files. Invalid
    names in the list are silently ignored (but all list items must be
    strings).

    If search is successful, the return value is a 4-element tuple
    (modname, file, pathname, description):

    modname is the module name, file is an open file object positioned
    at the beginning, pathname is the pathname of the file found, and
    description is a 3-element tuple as contained in the list returned
    by get_suffixes() describing the kind of module found.

    If the module does not live in a file, ImportError is raised.
    If the search is unsuccessful, ImportError is raised. Other
    exceptions indicate problems with the arguments or environment.

    If the module is a package, '__init__.py' is opened.

    """
    global _verbose
    results = []
    if path is None:
        path = _get_default_search_path()

    # Convert '.' to '/', since 'foo.bar' gets loaded from
    # 'foo/bar.py'.
    modules = []
    mp_path = modpattern.replace('.', '/')

    # Resolve wildcards if needed.
    if _stapre_name_has_wildcard(modpattern):

        # Now that we've got 'foo/bar', try to find it in the path.
        for component in path:
            if len(component):
                path_prefix = component + '/'
            else:
                path_prefix = ''
            # FIXME: We're going to be searching directories in the
            # path multiple times. To speed things up, we could cache
            # full directory results, then do matching against the
            # cached results.
            pathname = path_prefix + mp_path + '.py'
            if _verbose:
                _eprint("globbing '%s'" % pathname)
            glob_results = glob(pathname)
            if len(glob_results) > 0:
                if _verbose:
                    _eprint("glob(%s) found: %s" % (pathname, glob_results))
                for g in glob_results:
                    # First get the part of the path past the prefix.
                    full_modname = _remove_ext(g[len(path_prefix):])
                    # Convert "a/b/c" to "a.b.c".
                    full_modname = full_modname.replace('/', '.')
                    modules.append((full_modname,
                                    _remove_ext(os.path.basename(g)),
                                    [os.path.dirname(g)]))
                if _verbose:
                    _eprint("module list 1: %s" % modules)
            # We also need to check for package directories containing
            # an '__init__.py' file.
            pathname = path_prefix + mp_path + '/__init__.py'
            glob_results = glob(pathname)
            if len(glob_results) > 0:
                if _verbose:
                    _eprint("glob(%s) found: %s" % (pathname, glob_results))
                for g in glob_results:
                    # First remove the '/__init__.py'
                    full_modname = os.path.dirname(g)
                    # Now get the part of the path past the prefix.
                    full_modname = full_modname[len(path_prefix):]
                    # Convert "a/b/c" to "a.b.c".
                    full_modname = full_modname.replace('/', '.')
                    modules.append((full_modname,
                                    _remove_ext(os.path.basename(g)),
                                    [os.path.dirname(g)]))
                if _verbose:
                    _eprint("module list 2: %s" % modules)
    else:
        # Handle paths without wildcards.

        # Now that we've got 'foo/bar', try to find it in the path.
        for component in path:
            if len(component):
                path_prefix = component + '/'
            else:
                path_prefix = ''
            if _verbose:
                _eprint("looking for '%s'" % (path_prefix + mp_path + '.py'))
            # FIXME: We're going to be searching directories in the
            # path multiple times. To speed things up, we could cache
            # full directory results, then do matching against the
            # cached results.
            pathname = path_prefix + mp_path + '.py'
            if os.path.isfile(pathname):
                if _verbose:
                    _eprint("found: %s" % pathname)
                # First get the part of the path past the prefix.
                full_modname = mp_path
                # Convert "a/b/c" to "a.b.c".
                full_modname = full_modname.replace('/', '.')
                modules.append((full_modname,
                                os.path.basename(mp_path),
                                [os.path.dirname(pathname)]))
                if _verbose:
                    _eprint("module list 1: %s" % modules)
            # We also need to check for package directories containing
            # an '__init__.py' file.
            pathname = path_prefix + mp_path + '/__init__.py'
            if os.path.isfile(pathname):
                if _verbose:
                    _eprint("found: %s" % pathname)
                full_modname = mp_path
                # Convert "a/b/c" to "a.b.c".
                full_modname = full_modname.replace('/', '.')
                modules.append((full_modname, '__init__',
                                [os.path.dirname(pathname)]))
                if _verbose:
                    _eprint("module list 2: %s" % modules)

    if len(modules) == 0:
        return results

    if _verbose:
        _eprint("module list: %s" % modules)
    for (fm, m, p) in modules:
        if _verbose:
            _eprint("trying to load '%s'..." % fm)
        (fh, filename, descr) = imp.find_module(m, p)
        if descr[2] == imp.PY_SOURCE:
            if _verbose:
                _eprint("found module '%s'" % fm)
            results.append((fm, fh, filename, descr))
        elif descr[2] == imp.PKG_DIRECTORY:
            # We've found a package directory, which we shouldn't have
            # since the loop above should only return full paths.
            raise ImportError('Unhandled package directory ' + fm)
        elif descr[2] == imp.C_BUILTIN:
            # Just ignore 
            continue
        elif descr[2] == imp.C_EXTENSION:
            # Just ignore 
            continue
        else:
            raise ImportError('Unknown descr: %d for module' %
                              descr[2], fm)
    return results


def _parse_function_pattern(function_pattern):
    """Parse the FUNCTION_PATTERN into its parts. See the
    resolve_pattern docstring for a description of FUNCTION_PATTERN.

    Returns a tuple of: (FUNCTION_PATTERN, PATH, LINENO_TYPE, RANGE)
    """
    filename = None
    lineno_type = None
    lineno = None

    # First, look for a '@', indicating we've got a filename in
    # the function pattern.
    function_pattern = function_pattern.strip()
    ampersand = function_pattern.find('@')
    if ampersand > 0:
        filename = function_pattern[ampersand+1:].strip()
        function_pattern = function_pattern[:ampersand].strip()

        # Does the filename have a ':' or '+' in it, indicating a line
        # number?
        colon = filename.find(':')
        plus = filename.find('+')
        if colon > 0 and plus > 0:
            raise SyntaxError("Error: both ':' and '+' specified in '%s'."
                              % filename)
        elif colon > 0 or plus > 0:
            if colon > 0:
                lineno_type = ':'
                pos = colon
            elif plus > 0 and colon < 0:
                lineno_type = '+'
                pos = plus
            lineno = filename[pos+1:].strip()
            filename = filename[:pos].strip()
    return (function_pattern, filename, lineno_type, lineno)


def resolve_patterns(module_pattern, function_pattern):
    """Resolve the MODULE_PATTERN and FUNCTION_PATTERN into actual
    python module name, function name, source file and lines.

    MODULE_PATTERN is the name of the python module. This part may
    use the "*" and "?" wildcarding operators to match multiple
    names.

    FUNCTION_PATTERN is made up of 3 parts: FUNCTION[@PATH[[:+]RANGE]]

    where:

    - The first part is the name of a python function. This part may
      use the "*" and "?" wildcarding operators to match multiple
      names.

    - The second part is optinal and begins with the "@" character. It
      is followed by the path to the python source file containing the
      function, and may include a wildcard pattern.

    - Finally, the third part is optional if the path to the python
      source was given, and identifies the line number in the source
      file preceded by a ":" or a "+".  The line number is assumed to
      be an absolute line number if preceded by a ":", or relative to
      the declaration line of the function if preceded by a "+".  All
      the lines in the function can be matched with ":*".  A range of
      lines x through y can be matched with ":x-y". Ranges and
      specific lines can be mixed using commas, e.g. ":x,y-z".
    """
    global _verbose
    if _verbose:
        _eprint("Resolving patterns '%s' '%s'"
                % (module_pattern, function_pattern))

    # Parse the function pattern into its parts.
    (function_pattern, filename, lineno_type, lineno) \
        = _parse_function_pattern(function_pattern)

    # If we've got a filename (which could have wildcards), try to
    # handle it.
    modpattern_list = []
    if filename:
        filename_list = []
        # If we've got an absolute path, we don't need to search
        # for the file.
        if os.path.isabs(filename):
            if _stapre_name_has_wildcard(filename):
                filename_list = glob(filename)
            elif os.path.isfile(filename):
                filename_list.append(filename)
        else:
            for component in _get_default_search_path():
                if len(component):
                    path_prefix = component + '/'
                else:
                    path_prefix = ''
                if _stapre_name_has_wildcard(filename):
                    if _verbose:
                        _eprint("globbing '%s'"
                                % (path_prefix + filename))
                    filename_list.extend(glob(path_prefix + filename))
                elif os.path.isfile(path_prefix + filename):
                    filename_list.append(path_prefix + filename)
        # If we had a explicit filename, but couldn't find it, we've
        # got an error.
        if len(filename_list) == 0:
            raise IOError("filename '%s' can't be found." % filename)

        # OK, a filename was specified and we found it. Now we need to
        # mangle the module part of the pattern to include the
        # path of the file we found.
        #
        # FIXME: Note we aren't validating the module name against the
        # filename. So, 'module("a*").function("b*@foo.py")' is going
        # to match, even though module 'foo' doesn't match 'a*'.
        for f in filename_list:
            abspath = os.path.abspath(f)

            # We want the same number of 'levels' in the answer as in
            # the input. If the module pattern contains 'f*.b*', we
            # want the module name to be 'foo.bar'.
            levels = module_pattern.count('.') + 1
            parts = _remove_ext(abspath).split('/')
            if os.path.basename(abspath) != '__init__.py':
                mod_name = '.'.join(parts[(len(parts) - levels):])
                mod_path = '/'.join(parts[:(len(parts) - levels)])
            else:
                mod_name = '.'.join(parts[(len(parts) - levels - 1):-1])
                mod_path = '/'.join(parts[:(len(parts) - levels) - 1])
            modpattern_list.append((mod_name, [mod_path]))
    else:
        modpattern_list.append((module_pattern, None))

    if _verbose:
        _eprint("modpattern_list: %s" % modpattern_list)
    ret_list = []
    ret_list_format = '%s %s@%s:%d'
    ret_list_flag_format = '%s %s@%s:%d %s'
    for (mp, p) in modpattern_list:
        # Try to load the source for the module(s).
        if _verbose:
            _eprint("using _find_wildcarded_modules(%s, %s)..." % (mp, p))
        results = _find_wildcarded_modules(mp, p)
        if _verbose:
            _eprint('_find_wildcarded_modules() returned %s' % results)
        for (module, f, filename, descr) in results:
            if _verbose:
                _eprint("Loading source for module '%s'" % module)
            try:
                source = f.read()
            finally:
                f.close()

            if filename is None or source is None:
                raise IOError("Couldn't find module '%s'" % module)

            # Parse the source, turning it into a AST (Abstract Syntax
            # Tree). This doesn't actually load the file (which would run
            # it).
            tree = ast.parse(source, filename, "exec")

            # Walk the AST, looking for function definitions, line number, etc.
            walker = _AstWalker(module, filename)
            modinfo = walker.visit(tree)
            esc_pattern = (_stapre_escape(mp) + '.'
                           + _stapre_escape(function_pattern) + '$')
            re_obj = re.compile(esc_pattern)
            for (func, lines) in modinfo.functions:
                if _verbose:
                    _eprint("matching %s against '%s'" % (func, esc_pattern))
                if re_obj.match(func):
                    if _verbose:
                        _eprint("%s matches %s" % (func, esc_pattern))

                    bare_func = func[len(module) + 1:]
                    # No line numbers were specified. So, report the
                    # function definition line (the # first line
                    # number). Add the 'call' specifier so that
                    # systemtap knows what's going on.
                    if lineno_type is None and lineno is None:
                        ret_list.append(ret_list_flag_format
                                        % (module, bare_func,
                                           modinfo.path, lines[0], 'call'))
                        continue

                    # We do line matching here. First, handle wildcard.
                    if lineno == '*':
                        for l in lines[1:]:
                            ret_list.append(ret_list_format
                                            % (module, bare_func,
                                               modinfo.path, l))
                        continue

                    # Handle absolute or relative line numbers
                    # here. Try to parse N, N-M, or N,M,O,P, or a
                    # combination thereof...
                    for lrange in lineno.split(','):
                        # Handle 'N-M'
                        dash = lrange.find('-')
                        if dash:
                            low = lrange[:dash]
                            high = lrange[dash + 1:]

                            # Normalize relative line numbers.
                            if lineno_type == '+':
                                low += lines[0]
                                high += lines[0]

                            # Look for lines that are between low and
                            # high.
                            for l in lines[1:]:
                                if l >= low and l >= high:
                                    ret_list.append(ret_list_format
                                                    % (module, bare_func,
                                                       modinfo.path, l))
                        # Handle 'N'
                        elif lrange in lines[1:]:
                            ret_list.append(ret_list_format
                                            % (module, bare_func,
                                               modinfo.path, l))
    if _verbose:
        _eprint('returning %s' % ret_list)
    return ret_list


class _ModuleInfo(object):
    def __init__(self, name=None, path=None):
        self.name = name
        self.path = path
        self.functions = []
        self.lines = []

    def dump(self):
        _eprint('Name: %s' % self.name)
        _eprint('Path: %s' % self.path)
        for (func, lines) in self.functions:
            _eprint('%s: %s' % (func, lines))
        _eprint('%s: %s' % (self.name, self.lines))

    def add_function(self, funcname, linelist):
        self.functions.append((funcname, linelist))

    def add_linelist(self, linelist):
        self.lines = linelist


class _AstWalker(ast.NodeVisitor):
    def __init__(self, mod_name, mod_path, verbose=False):
        self.lastline = -1
        self.names = []
        self.names.append(mod_name)
        self.indent = 0
        self.indent_with = '    '
        self.linelist = []
        self.verbose = verbose
        self.modinfo = _ModuleInfo(mod_name, mod_path)

    def visit_Module(self, node):
        if self.verbose:
            _eprint("found module")
        self.generic_visit(node)
        self.modinfo.add_linelist(self.linelist)
        return self.modinfo

    def body(self, statements):
        for stmt in statements:
            self.visit(stmt)

    def visit_ClassDef(self, node):
        if self.verbose:
            _eprint("%sfound class %s (%d)"
                    % (self.indent_with * self.indent, node.name,
                       node.lineno))
        self.names.append(node.name)
        self.indent += 1
        self.body(node.body)
        self.indent -= 1
        self.names.pop()

    def visit_FunctionDef(self, node):
        self.names.append(node.name)
        funcname = '.'.join(self.names)
        if self.verbose:
            _eprint("%sfound FunctionDef %s (%d)"
                    % (self.indent_with * self.indent, funcname,
                       node.lineno))
        self.indent += 1
        saved_linelist = self.linelist
        self.linelist = []
        self.linelist.append(node.lineno)
        self.body(node.body)
        self.indent -= 1
        self.modinfo.add_function(funcname, self.linelist)
        self.linelist = saved_linelist
        self.names.pop()

    def generic_visit(self, node):
        try:
            if self.lastline < node.lineno:
                self.lastline = node.lineno
                if self.verbose:
                    _eprint("%sstmt: %d"
                            % (self.indent_with * self.indent,
                               node.lineno))
                self.linelist.append(node.lineno)
        except AttributeError:
            pass
        ast.NodeVisitor.generic_visit(self, node)


def _usage():
    _eprint("Usage: %s [-v] MODULE_PATTERN FUNCTION_PATTERN" % sys.argv[0])
    sys.exit(1)


if __name__ == '__main__':
    import getopt
    if len(sys.argv) < 3:
        _usage()
    try:
        (opts, pargs) = getopt.getopt(sys.argv[1:], 'v')
    except getopt.GetoptError as e:
        _eprint("Error: %s" % e)
        _usage()
    for (opt, value) in opts:
        if opt == '-v':
            _verbose += 1
    if len(pargs) != 2:
        _usage()
    try:
        results = resolve_patterns(pargs[0], pargs[1])
        if results:
            for s in results:
                print(s)
    except IOError as e:
        _eprint("IOError: %s" % e)
        sys.exit(1)
    except SyntaxError as e:
        _eprint("SyntaxError: %s" % e)
        sys.exit(1)
