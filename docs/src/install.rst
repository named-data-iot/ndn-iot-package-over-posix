Download and Build
===============

We use docker container for development environment.
If you're on macOS, you can also use the Non-Docker build approach.

Docker image
------------

A `docker image`_ with compiled ndn-iot-package-over-linux package can be used as the base development environment. 

The image is based on Ubuntu. It contains all dependencies for this package, including dependencies for the `Quickstart Examples`_.
It is **highly recommanded** to use this image to save time for preparing the development environment.

.. _Quickstart Examples: examples.html
.. _docker image: https://github.com/shsssc/ndn-lite-docker-image

To use the image, you can download the latest version using git:

.. code-block:: bash

    $ git clone https://github.com/shsssc/ndn-lite-docker-image
    $ cd ndn-lite-docker-image

To build the image:

.. code-block:: bash

    $ docker build --tag ndnlite:0.1 .

To use the image for development:

.. code-block:: bash

    $ docker run -d -p6060:6060 --name ndnlite-container ndnlite:0.1 #start the container with iot-controller on http://localhost:6060
    $ docker exec -it ndnlite-container /bin/bash #run shell in the container

Non Docker Build (macOS)
-------------

Download the latest version with git:

.. code-block:: bash

    $ git clone --recursive https://github.com/named-data-iot/ndn-iot-package-over-posix.git
    $ cd ndn-iot-package-over-posix

Create a new directory and configure with cmake:

.. code-block:: bash

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Release ..

build all examples:

.. code-block:: bash

    $ make -j2



Instruction for developers
--------------------------

Compile the documentation with Sphinx:

.. code-block:: bash

    $ cd docs && pip3 install -r requirements.txt
    $ make html
    $ open _build/html/index.html
