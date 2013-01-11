from setuptools import setup, find_packages
setup(
    name = "GatewayMonitor",
    version = "0.1",
    packages = find_packages(),
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

