from distutils.core import setup, Extension

module = Extension('python_hacks',
                    sources = ['PythonHacks.cpp'],
                    language='cpp',
                    extra_compile_args=['-stdlib=libc++', '-std=c++11'])

setup (name = 'PythonHacks',
       version = '0.1',
       description = 'Hack Python',
       author = 'Gil Hoben',
       ext_modules = [module]
       )
