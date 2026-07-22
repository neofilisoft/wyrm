from setuptools import setup, find_packages

setup(
    name="wyrmlang",
    version="1.2",
    author="Neofilisoft",
    author_email="breakm82a1@gmail.com",
    description="Wyrm - A simple C-inspired programming language",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    url="https://github.com/yourusername/wyrm",
    packages=find_packages(),
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Education",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Topic :: Software Development :: Interpreters",
        "Topic :: Education",
    ],
    python_requires=">=3.7",
    entry_points={
        'console_scripts': [
            'wyrm=wyrm.main:main',
        ],
    },
)