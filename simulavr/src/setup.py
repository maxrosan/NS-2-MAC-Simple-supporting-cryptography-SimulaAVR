# -*- coding: UTF-8 -*-
# setup.py: Install-Script

try:
  from setuptools import setup
except:
  from distutils import setup
  
from distutils.sysconfig import get_python_lib

setup(name = 'pysimulavr',
      version = "1.1dev",
      py_modules = ["pysimulavr"],
      data_files = [(get_python_lib(), ["_pysimulavr.so"])],
      author="Klaus Rudolph & others",
      author_email="<simulavr-devel@nongnu.org>",
      license="GPL",
      description="python modul for simulavr")
# EOF
