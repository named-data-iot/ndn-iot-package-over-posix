Download and Build
===============

Download
-------

Download the latest version with git:

.. code-block:: bash

    $ git clone --recursive https://github.com/named-data-iot/ndn-iot-package-over-posix.git
    $ cd ndn-iot-package-over-posix

Create a new directory and configure with cmake:

.. code-block:: bash

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release ..

Build
-------
build all examples and unittests:

.. code-block:: bash

    $ make -j2

Optionally, you can check if current APIs comptiable with each other by running all tests

.. code-block:: bash

    $ ./unittest



Instruction for developers
--------------------------

Optionally, you can check if current APIs comptiable with each other by running all tests:

.. code-block:: bash

    $ ./unittest


Compile the documentation with Sphinx:

.. code-block:: bash

    $ cd docs && pip3 install -r requirements.txt
    $ make html
    $ open _build/html/index.html