# systemtap python module
# Copyright (C) 2016 Red Hat Inc.
#
# This file is part of systemtap, and is free software.  You can
# redistribute it and/or modify it under the terms of the GNU General
# Public License (GPL); either version 2, or (at your option) any
# later version.

# Note that we'd like for this module to be the same between python 2
# and python 3, but the 'print' function has changed quite
# dramatically between the two versions. You'd think we could use the
# '__future__' module's 'print' function so we'd be able to use python
# 3's 'print' function style everywhere. However, this causes python 2
# scripts that get loaded to have to use python 3 print syntax, since
# the '__future__' module's print function "leaks" through to the
# python 2 script. There's probably a way to fix that, but we'll just
# punt and use 'sys.stdout.write()', which is a little less
# convenient, but works the same on both python 2 and python 3.
import cmd
import os.path
import sys

if sys.version_info[0] == 2:
    import _HelperSDT
else:
    from . import _HelperSDT

class _Breakpoint:
    def __init__(self, index, filename, funcname, lineno, flags, key):
        self.index = index
        self.filename = filename
        self.funcname = funcname
        self.lineno = lineno
        self.key = key

        # Decode flags.
        self.callp = False
        self.returnp = False
        if flags & 0x2:
            self.callp = True
        if flags & 0x1:
            self.returnp = True

    def dump(self, out=None):
        if out is None:
            out = sys.stdout
        out.write("%s\n" % self.bpformat())

    def bpformat(self):
        if self.returnp:
            disp = '%s.return:%d' % (self.funcname, self.lineno)
        elif self.callp:
            disp = '%s.call:%d' % (self.funcname, self.lineno)
        else:
            disp = '%s:%d' % (self.funcname, self.lineno)
        return '%-4dbreakpoint at %s:%s %d' % (self.index, self.filename,
                                               disp, self.key)


class _BreakpointList:
    def __init__(self):
        self._index = 1
        self._bynumber = []      # breakpoints indexed by number

        # N.B. To make sure we're inside the right function, and not
        # just executing a 'def' statement, we need the function name
        # for line number breakpoints.
        #
        # The 'by line' and 'by function' lists are indexed by a (file,
        # function, lineno) tuple. The 'by function return' list is
        # indexed by a (file, function) tuple.
        self._byline = {}
        self._byfunc = {}
        self._byfuncret = {}

        # Let's keep a cache of the os.path.abspath() results, to
        # avoid looking up the same paths over and over again.
        self._abspath_cache = {}

    def _abspath(self, path):
        if path in self._abspath_cache:
            return self._abspath_cache[path]
        abspath = os.path.abspath(path)
        self._abspath_cache[path] = abspath
        return abspath
        
    def add(self, filename, funcname, lineno, flags, key):
        # Ensure we've got a full absolute path here.
        filename = self._abspath(filename)

        # We get passed the full function name, like
        # "class.method". When the breakpoint hits, we only see
        # 'method'. So, cut the function name down to the last bit.
        period = funcname.rfind('.')
        if period >= 0:
            funcname = funcname[period + 1:]

        bp = _Breakpoint(self._index, filename, funcname, lineno,
                         flags, key)
        self._index += 1
        self._bynumber.append(bp)

        if bp.returnp:
            if (filename, funcname) in self._byfuncret:
                self._byfuncret[filename, funcname].append(bp)
            else:
                self._byfuncret[filename, funcname] = [bp]
        elif bp.callp:
            if (filename, funcname, lineno) in self._byfunc:
                self._byfunc[filename, funcname, lineno].append(bp)
            else:
                self._byfunc[filename, funcname, lineno] = [bp]
        else:
            if (filename, funcname, lineno) in self._byline:
                self._byline[filename, funcname, lineno].append(bp)
            else:
                self._byline[filename, funcname, lineno] = [bp]

    def dump(self, out=None):
        if out is None:
            out = sys.stdout
        for bp in self._bynumber:
            bp.dump(out)

    def break_here(self, frame, event):
        # Depending on how the script name was passed to python,
        # relative or absolute, we might or might not get a full
        # pathname here. So, we'll make sure we've got an absolute
        # path.
        filename = self._abspath(frame.f_code.co_filename)

        funcname = frame.f_code.co_name
        lineno = frame.f_lineno
        if event == 'call':
            if (filename, funcname, lineno) in self._byfunc:
                return self._byfunc[filename, funcname, lineno]
        elif event == 'line':
            if (filename, funcname, lineno) in self._byline:
                return self._byline[filename, funcname, lineno]
        elif event == 'return':
            if (filename, funcname) in self._byfuncret:
                return self._byfuncret[filename, funcname]
        return None


class Dispatcher(cmd.Cmd):
    def __init__(self, pyfile):
        # Note that using the cmd class here is overkill, but it would
        # allow us to add more commands very easily.
        cmd.Cmd.__init__(self)
        self._bplist = _BreakpointList()

        # Note that the filename of breakpoint info has the python
        # major version present.
        bpFileBase = '_stp_python%d_probes' % sys.version_info[0]

        # Read the breakpoints from a file.
        lines = []
        if 'SYSTEMTAP_MODULE' in os.environ:
            self.envModule = os.environ['SYSTEMTAP_MODULE']
            bpFileName = "/proc/systemtap/%s/%s" % (self.envModule,
                                                    bpFileBase)
            try:
                bpFile = open(bpFileName)
            except IOError:
                sys.stderr.write("Error: the '%s' file could not be opened\n"
                                 % bpFileName)
                sys.exit(1)
            else:
                lines = bpFile.readlines()
                bpFile.close()
        else:
            sys.stderr.write("Error: the 'SYSTEMTAP_MODULE' environment"
                             " variable does not exist\n")
            sys.exit(1)

        # Now handle each command
        for line in lines:
            line = line[:-1]
            if len(line) > 0 and line[0] != '#':
                self.onecmd(line)

    def pytrace_dispatch(self, frame, event, arg):
        if event == 'call':
            bplist = self._bplist.break_here(frame, event)
            if bplist:
                for bp in bplist:
                    _HelperSDT.trace_callback(_HelperSDT.PyTrace_CALL,
                                              frame, arg, self.envModule,
                                              bp.key)
            return self.pytrace_dispatch
        elif event == 'line':
            bplist = self._bplist.break_here(frame, event)
            if bplist:
                for bp in bplist:
                    _HelperSDT.trace_callback(_HelperSDT.PyTrace_LINE,
                                              frame, arg, self.envModule,
                                              bp.key)
            return self.pytrace_dispatch
        elif event == 'return':
            bplist = self._bplist.break_here(frame, event)
            if bplist:
                for bp in bplist:
                    _HelperSDT.trace_callback(_HelperSDT.PyTrace_RETURN,
                                              frame, arg, self.envModule,
                                              bp.key)
            return self.pytrace_dispatch
        return self.pytrace_dispatch

    #
    # cmd class commands
    #

    def do_b(self, arg):
        # Breakpoint command:
        #   b [ MODULE|FUNCTION@FILENAME:LINENO|FLAGS|KEY ]
        if not arg:
            self._bplist.dump()
            return

        # Parse argument.
        # FIXME: 'module' needed?
        #  module = None
        funcname = None
        filename = None
        lineno = None
        flags = None
        key = None
        parts = arg.split('|')
        if len(parts) != 4:
            sys.stderr.write("Invalid breakpoint format: %s\n" % arg)
            sys.stderr.write("Wrong number of major parts (%d vs. 4)\n" %
                             len(parts))
            return
        #  module = parts[0]
        filename_arg = parts[1]
        try:
            flags = int(parts[2])
        except:
            sys.stderr.write("Invalid breakpoint format: %s\n" % arg)
            sys.stderr.write("Invalid flags value (%s)\n" % parts[2])
            return
        try:
            key = int(parts[3])
        except:
            sys.stderr.write("Invalid breakpoint format: %s\n" % arg)
            sys.stderr.write("Invalid key value (%s)\n" % parts[3])
            return
        parts = filename_arg.split('@')
        if len(parts) != 2:
            sys.stderr.write("Invalid breakpoint format: %s\n" % arg)
            sys.stderr.write("Wrong number of filename parts (%d vs. 2)\n" %
                             len(parts))
            return
        funcname = parts[0]
        lineno_arg = parts[1]
        parts = lineno_arg.split(':')
        if len(parts) != 2:
            sys.stderr.write("Invalid breakpoint format: %s\n" % arg)
            sys.stderr.write("Wrong number of line number parts (%d vs. 2)\n" %
                             len(parts))
            return
        filename = parts[0]
        try:
            lineno = int(parts[1])
        except:
            sys.stderr.write("Invalid breakpoint format: %s\n" % arg)
            sys.stderr.write("Invalid line number value (%s)\n" % parts[1])
            return

        # Actually add the breakpoint.
        self._bplist.add(filename, funcname, lineno, flags, key)


def run():
    # Now that we're attached, run the real python file.
    mainpyfile = sys.argv[1]
    if not os.path.exists(mainpyfile):
        sys.stderr.write("Error: '%s' does not exist\n" % mainpyfile)
        sys.exit(1)

    del sys.argv[0]         # Hide this module from the argument list

    # Start tracing.
    dispatcher = Dispatcher(mainpyfile)
    sys.settrace(dispatcher.pytrace_dispatch)

    # The script we're about to run has to run in __main__ namespace
    # (or imports from __main__ will break).
    #
    # So we clear up the __main__ namespace and set several special
    # variables (this gets rid of our globals).
    import __main__
    __main__.__dict__.clear()
    __main__.__dict__.update({"__name__": "__main__",
                              "__file__": mainpyfile,
                              "__builtins__": __builtins__})

    # Run the real file. When it finishes, remove tracing.
    try:
        # execfile(mainpyfile, __main__.__dict__, __main__.__dict__)
        exec(compile(open(mainpyfile).read(), mainpyfile, 'exec'),
             __main__.__dict__, __main__.__dict__)
    finally:
        sys.settrace(None)
