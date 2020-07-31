Trust Policy
=============================
Trust policy refers to policies that restrict the Data only be signed to few permissioned keys.
Recall from the Pub/Sub section that every event is a semantically named Data, and this Data is signed by semantically named key.
For instance, a valid Data may look like:

::

    Data ---+---> Data Name: /ndn/alice/example/1234
            |
            +---> Data Content: integar value
            |
            +---> Data Signature --+---> Signature Info ---> Key Locator: /ndn/alice/KEY
                              |
                              +---> Signature Value: 11c6991...

The ``/ndn/alice/KEY`` is the signing key for Data named ``/ndn/alice/example/1234``.
Trust policy play the role here to regulate the signing relations so that only alice's key can sign the Data under she's prefix ``/ndn/alice``.
This policy check is implemented in the Pub/Sub module with several default options:

#. **Same room:** Data Name and Key Locator should indicate they're in the same room. 
   For example, ``/my-home/kitchen/meta`` should be signed by ``/my-home/kitchen/device-123/KEY``.
#. **Controller only**: Data should only be signed by home controller.
   This applies to scenarios where commands are critical to home security (e.g., unlock doors)
   For example: ``/my-home/front/unlock`` should be signed by ``/my-home/controller/KEY``
#. **Same producer:** Data Name should exactly match Key Locator.
   For example: ``/my-home/kitchen/device-123/version`` should be  signed by ``/my-home/kitchen/device-123/KEY``.
   Notice that this policy has subtle difference from the same room policy, which usually applies to Datas whose Names not explictly reveal their producers.

Pub/Sub module will perform different policy checks based on received data types.
By default command Datas are checked against controller only policy, and other Datas are checked against same producer policy.
