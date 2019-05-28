# Always prefer setuptools over distutils
from setuptools import setup
# To use a consistent encoding

setup(
    name="otl866",
    version=0.1,
    description="Python Interface to Open-TL866 ASCII protocol",
    author="William D. Jones",
    author_email="thor0505@comcast.net",
    license="BSD",
    packages=["otl866", "otl866/bootloader"],
    install_requires=[
        "intelhex",
        "pexpect",
        "pyserial",
        "pyusb",
    ],
    entry_points={
        'console_scripts': [
            "otl866=otl866.cli:main",
        ],
    },
)
