from setuptools import setup, find_packages

import py2exe

class Target:
    def __init__(self, **kw):
        self.__dict__.update(kw)
        # for the versioninfo resources
        self.version = "0.5.0"
        self.company_name = "No Company"
        self.copyright = "no copyright"
        self.name = "py2exe sample files"

myservice = Target(
    # used for the versioninfo resource
    description = "AMMO Gateway Monitor Service",
    # what to build. For a service, the module name (not the
    # filename) must be specified!
    modules = ["GatewayMonitorService"],
    cmdline_style='pywin32',
    )

setup(
    name = "GatewayMonitor",
    version = "0.1",
    packages = find_packages(),
    console = ['gatewaymonitor/GatewayMonitor.py'],
    service = [myservice],
    entry_points = {
      'console_scripts': [
        'GatewayMonitor = gatewaymonitor.GatewayMonitor:main'
      ]
    },
        

    # Project uses reStructuredText, so ensure that the docutils get
    # installed or upgraded on the target machine
    install_requires = ['psutil>=0.6.1'],

    # metadata for upload to PyPI
    author = "Jonathon Williams",
    author_email = "jwilliams@isis.vanderbilt.edu",
    description = "AMMO Gateway monitoring script",
    license = "BSD",
    keywords = "ammo",
    url = "http://ammo.isis.vanderbilt.edu/",   # project home page, if any

    # could also include long_description, download_url, classifiers, etc.
)

