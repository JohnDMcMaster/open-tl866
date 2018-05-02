# Always prefer setuptools over distutils
from setuptools import setup
# To use a consistent encoding


setup(
    name="pytl866",
    version=0.1,
    description="Python Interface to Open-TL866 ASCII protocol",
    author="William D. Jones",
    author_email="thor0505@comcast.net",
    license="BSD",
    packages=["pytl866"],
    install_requires=["pyserial"]
)
