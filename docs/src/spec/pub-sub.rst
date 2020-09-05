.. _pubsub-label:

Publish/Subscribe System
=============================

The Publish/Subscribe system constitute the core of package. 
Principally, every application data exchanges should be wrapped into the Pub/Sub.

Topic
------------
Topic definition is foundation of building any Pub/Sub system.
In this package, Topic is defined as series of name components connected by ``/``.
Based on longest prefix match, a topic can also be a *child topic*, or *parent topic* of another topic.
For example:

    +-------------+---------------+------------------+
    | Topic       | Child Topic   |  Parent Topic    |
    +=============+===============+==================+
    | /A          | /A/B          | /                |
    |             |               |                  |
    |             | /A/B/C        |                  |
    +-------------+---------------+------------------+
    | /A/B        | /A/B/C        | /                |
    |             |               |                  |
    |             |               | /A               |
    +-------------+---------------+------------------+
    | /A/B/C      | None          | /                |
    |             |               |                  |
    |             |               | /A               |
    |             |               |                  |
    |             |               | /A/B             |
    +-------------+---------------+------------------+

In current design, topic hierachy has two level: *room* and *individual device*. Then an example mapping can be:

    +-------------+-----------------------------+------------------+
    | Topic       | Child Topic                 |  Parent Topic    |
    +=============+=============================+==================+
    | /living     | /living/device-398          | /                |
    |             |                             |                  |
    |             | /living/device-24777        |                  |
    +-------------+-----------------------------+------------------+

Subscriber and Publisher
-----------------------------
In this package, any instance that can send data to *topics* is publisher, and any instance that can receive data from *topics* is subscriber.
A device instance can be publisher and subscriber at the same time.

Notice
'''''''''''''''
If the data is reliably delivered is out of the scope of definition.

With a hierachical topic layout, subscrption to any topic is equilavent to subscription to the topic itself and also all child topics.

    +------------------------+-----------------------------+
    | Subscribe to Topic     | Equilavent to Subscribing to|
    +========================+=============================+
    | /A                     | /A/B                        |
    |                        |                             |
    |                        | /A/B/C                      |
    +------------------------+-----------------------------+

Publishing to any topic is equilavent to publishing to the topic itself and also all parent topics.

    +------------------------+-----------------------------+
    | Publish to Topic       | Equilavent to Publishing on |
    +========================+=============================+
    | /A/B                   | /A/B                        |
    |                        |                             |
    |                        | /A                          |
    |                        |                             |
    |                        | /                           |
    +------------------------+-----------------------------+

API Reference
-------------------

API for Pub/Sub should at least three pieces:
#. Topic targeted
#. Data structure for Pub/Sub event data
#. Callbacks

In this package we extend above three into four by adding ``service``, which could reduce subscription/publishing range by specifying a interested service from application.
Pub/Sub event data is defined as the combination of data payload, data-id (to further identify a event data inside a topic).
Data freshness period is also important, but can work with default settings and leave it there.

.. code-block:: c

    typedef struct ps_event {
        const uint8_t* data_id;
        uint32_t data_id_len;
        const uint8_t* payload;
        uint32_t payload_len;
        uint32_t freshness_period;
    } ps_event_t;

Besides Pub/Sub event's data structure, APIs are provided as follows:

.. code-block:: c

    /** subscribe
    * If is not cmd, this function will register a event that periodically send an Interest to the name
    * prefix and fetch data.
    * Subscription Interest Format: /home-prefix/service/DATA/identifier[0,2],MustBeFresh,CanBePrefix.
    *
    * If is cmd, this function will register a Interest filter /home-prefix/service/CMD and listen to
    * notification on new CMD content.
    * Once there is a comming notification and the identifiers is under subscribed identifiers, an
    * Interest will be sent to fetch the new CMD.
    * Cmd Notification Interest Format: /home/service/CMD/NOTIFY/identifier[0,2]/action
    * Cmd fetching Interest Format: /home/service/CMD/identifier[0,2]/action
    */
    void
    ps_subscribe_to_content(uint8_t service, const char* scope,
                            uint32_t interval, ps_on_content_published callback, void* userdata);

The scope refers the *topic* discussed above. Inputs like ``""``, ``"/kitchen"`` are welcome.
``*userdata`` refers to pointer to user-specified data structure which wanted to be thrown into the callback when triggered.
Similarly, other APIs are provided as follows. *Content* and *Command* are treated separately also based on consideration of reduce the subscription/publishing range. 

.. code-block:: c

    void
    ps_subscribe_to_command(uint8_t service, const char* scope, ps_on_command_published callback, void* userdata);

    /** publish data
    * This function will publish data to a content repo.
    * Data format: /home-prefix/service/DATA/my-identifiers/timestamp
    */
    void
    ps_publish_content(uint8_t service, const ps_event_t* event);

    /** publish command to the target scope
    * This function will publish command to a content repo and send out a notification Interest.
    * Cmd Notification Interest Format: /home-prefix/service/NOTIFY/CMD/identifier[0,2]/action
    * Data format: /home-prefix/service/CMD/my-identifiers/timestamp
    */
    void
    ps_publish_command(uint8_t service, const char* scope, const ps_event_t* event);
