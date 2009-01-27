# This is the distutils setup script for pynxt.

DESCRIPTION = """
PyNXT is a Python module that enables developers to communicate with
Lego Mindstorms NXT bricks at a low level. It currently facilitates
scanning the USB chain for a NXT brick and implements the SAM-BA
bootloader communication protocol. It comes with two utilities, fwflash
and fwexec, which can be used to write a firmware to either flash memory
or RAM, and execute it from there.""".strip()

METADATA = {
    "name":             "pynxt",
    "version":          "0.0.1",
    "license":          "GPL",
    "url":              "http://nxos.natulte.net",
    "author":           "David Anderson",
    "author_email":     "dave@natulte.net",
    "description":      "Lego Mindstorms NXT interface",
    "long_description": DESCRIPTION,
}

from os.path import isfile
from distutils.core import setup
from distutils.command.install_data import install_data
from distutils.command.sdist import sdist

# Data installer that installs data in the package directory. Inspired
# by pygame's setup.py.
class package_data_installer(install_data):
    def run(self):
        install_cmd = self.get_finalized_command('install')
        self.install_dir = getattr(install_cmd, 'install_lib')
        return install_data.run(self)

class file_checking_sdist(sdist):
    def run(self):
        if not isfile('flash_driver.bin'):
            raise SystemExit("FATAL: Please build flash_driver.bin first!")
        sdist.run(self)

cmdclass = {
    'install_data': package_data_installer,
    'sdist': file_checking_sdist,
    }

PACKAGEDATA = {
    "cmdclass": cmdclass,
    "packages": ["nxt", "nxt.bin"],
    "data_files": [["nxt", ["flash_driver.bin"]]],
    "scripts": ['fwflash', 'fwexec'],
    }

PACKAGEDATA.update(METADATA)
setup(**PACKAGEDATA)
