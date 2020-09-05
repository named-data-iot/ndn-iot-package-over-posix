Overview
=========

Our architecture provides a unique abstraction of devices from their distinct home service provided and locator in a way that allows developers to build applications that are insulated from the specifics of which device they are using. 
For example, there is home service “LED”. Locators of "LED" service are "/bedroom/device-123" and "/bedroom/device-456".

Applications use the same abstraction.
When an application interacts with the virtual representations of devices, it knows that the devices support certain actions/commands based on its home service associated. 
A device that has the “LED” service provided must support both the “on” and “off” command. 
In this way, all LED lights are the same, and it doesn’t matter what kind of LED light is actually involved.

The virtual representations of devices is in the format of {service, locator}.
Locator aggregation is supported.
For example, {LED, /bedroom} may represent devices {LED, /bedroom/device-123} and {LED, /bedroom/device-456}.
Programs that implement the virtual representations on actual devices is called Device Program.

Core concepts
-------------

**Service**

Services are the interactions that a device allows. 
They provide an abstraction that allows applications to work with devices based on the home services they support, and not be tied to a specific manufacturer or model.

Consider the example of the “Switch” service. 
In simple terms, a switch is a device that can turn on and off. 
It may be that a switch in the traditional sense (for example an in-wall light switch), a connected bulb, or even a music player. 
All of these unique devices have a Device Program, and those Device Program’s support the “Switch” service. 
This allows applications to only require a device that supports the “Switch” service and thus work with a variety of devices including different manufacturer and model-specific “switches”. 
The application can then interact with the device knowing that it supports the “on” and “off” command (more on commands below), without caring about the specific device being used.

This code illustrates how a application might interact with a device that supports the “Switch” service through Pub/Sub APIs:

.. code-block:: c

    // try to turn on bedroom switches with a payload for "on" durability
    uint8_t seconds = 80;
    ps_event_t command_content = {
        .data_id = "on",
        .data_id_len = strlen("on"),
        .payload = &seconds,
        .payload_len = sizeof(seconds);
    };
    ps_publish_command(NDN_SD_SWITCH, "/bedroom", &command_content);

**Pub/Sub**

:ref:`pubsub-label`.

**Command/Content**

Command and content are represented by Pub/Sub events and be published or subscribed separately through Pub/Sub APIs.

Events are represented in {data-id, payload}. 
Data-id specifies the semantic identifier of a piece of data.
For example, data-id for commands that turning switch on can be "on", and data-id for content that switch report their on/off state can be "state". 



