from distutils.core import setup
from distutils.extension import Extension
import commands

def pkgconfig(*packages, **kw):
    flag_map = {'-I': 'include_dirs', '-L': 'library_dirs', '-l': 'libraries'}
    for token in commands.getoutput("pkg-config --libs --cflags %s" % ' '.join(packages)).split():
        kw.setdefault(flag_map.get(token[:2]), []).append(token[2:])
    return kw

module1 = Extension('libmtag', ['libmtagmodule.c'], **pkgconfig('libmtag'))

setup (name = 'MTag',
       version = '0.0.0',
       description = 'This is a demo package',
       ext_modules = [module1])
