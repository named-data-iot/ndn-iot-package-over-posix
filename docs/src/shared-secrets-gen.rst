.. _shared-secrets-label:

Shared Secrets Generation
=======

If you're not using image on build the package on macOS, install dependencies for QR code:

.. code-block:: bash

    $ pip install pyqrcode
    $ pip install pypng

If you have already built the library, you can generate shared secrets by:

.. code-block:: bash

    $ cd /project/root
    $ cd build
    $ ./examples/tutorial-gen-new-shared-info

This will generate a ``tutorial_shared_info.txt`` file in the ``build`` folder. 

``tutorial_shared_info.txt`` contains identity string, public/private key pair and a pre-shared key between identity and IoT controller.

Afterwards, you need to generate a QR code for this .txt file (may need sudo here).

.. code-block:: bash

    $ cd /project/root
    $ python ./QR_encoder.py

Then a ``shared_info.png`` is generated in the project root path.
The QR code encodes all information contained in ``tutorial_shared_info.txt`` and can later be uploaded through IoT controller's Web UI.

Finally, you can rename the ``shared_info.png`` and `tutorial_shared_info.txt`` with whatever names you like and put into ``/<project-root>/devices``.
We recommend naming conventions used by existing examples. 

The .txt file is used by device/application to obtain initial key materials and .png is used by IoT controller to gain shared secrets between itself and devices.