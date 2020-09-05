Tutorial
============

After playing with quickstart examples, now we go through ``tutorial-app.c`` by lines to see the details.
This is a typical LED Device Program that subscribes to command and publishes some hello messages in "LED" service. 

Initialization
--------------
The device instance is initialized by loading pre-shared secrets. 
As you can see from the definition of ``load_bootstrapping_info``, it will read some hard coded ``.txt`` file, which will give the initial crypto keys and device identifiers.
``ndn_lite_startup`` is an essential function to initialize various in-library states (e.g., key storage, service list) and platform-specific configurations.

.. code-block:: c

    // PARSE COMMAND LINE PARAMETERS
    int ret = NDN_SUCCESS;
    if ((ret = load_bootstrapping_info()) != 0) {
        return ret;
    }
    ndn_lite_startup();


Connectivity
--------------
Connectivity interfaces are abstracted as ``face``s, 

.. code-block:: c

    // CREAT A MULTICAST FACE
    face = ndn_unix_face_construct(NDN_NFD_DEFAULT_ADDR, true);
    // face = ndn_udp_unicast_face_construct(INADDR_ANY, htons((uint16_t) 2000), inet_addr("224.0.23.170"), htons((uint16_t) 56363));
    // in_port_t multicast_port = htons((uint16_t) 56363);
    // in_addr_t multicast_ip = inet_addr("224.0.23.170");
    // face = ndn_udp_multicast_face_construct(INADDR_ANY, multicast_ip, multicast_port);

In quickstart examples, device instance is connected to controller via Unix socket, given they're one the same host.
Otherwise, you can specify a multicast face ``udp4://224.0.23.170:56363`` for data communication.


Load, Advertise and Register Services
--------------
Our package abstracts devices/applications (noted as instance below) into services - that is, what home service the device/application is associated with. 
This allows us to build applications that can work with any device that supports a given associated home service. 
Services, from instances' prespective, are categorized into *access-requested* that device instance subscribes to that service thus need access key, and *encryption-requested* that means device instance will need a encryption key to publish data into that service.

In this package, before running into the main loop, one should specify the services to be used.
By calling ``sd_add_or_update_self_service()``, instance will advertise given services to the IoT controller when entering the main loop so that IoT controller knows who is providing which service. 
If an instance is interested in publishing messages in certain services, it should call ``ndn_ac_register_encryption_key_request()`` to request encryption key for that service.
If an instance is interested in receiving messages from certain services, access keys should be requested by calling ``ndn_ac_register_access_key_request()``.
Requesting encryption or access keys should be prior to any ``ps_publish_to()`` or ``ps_subscribe_to()`` calls so that the latter two have necessary keys to correctly execute.

**Note: It is the instances who actually publish commands/content, not services. Services are only the abstractions of instances.** 

Some pre-defined services are in ``/<project-root>/ndn-lite/ndn-services.h``. You can define your own services in similar approaches.

.. code-block:: c

    // LOAD SERVICES PROVIDED BY SELF DEVICE
    uint8_t capability[1];
    capability[0] = NDN_SD_LED;

    // SET UP SERVICE DISCOVERY
    sd_add_or_update_self_service(NDN_SD_LED, true, 1); // state code 1 means normal
    ndn_ac_register_encryption_key_request(NDN_SD_LED);
    //ndn_ac_register_access_request(NDN_SD_LED);


Bootstrapping and Callbacks
--------------
Before bootstrapping device onto controller, pre-shared crypto keys and identifiers should be loaded and wrap into the sign-on request. 

.. code-block:: c

    // START BOOTSTRAPPING
    ndn_bootstrapping_info_t booststrapping_info = {
        .pre_installed_prv_key_bytes = secp256r1_prv_key_bytes,
        .pre_installed_pub_key_bytes = secp256r1_pub_key_bytes,
        .pre_shared_hmac_key_bytes = hmac_key_bytes,
    };
    ndn_device_info_t device_info = {
        .device_identifier = device_identifier,
        .service_list = capability,
        .service_list_size = sizeof(capability),
    };
    ndn_security_bootstrapping(&face->intf, &booststrapping_info, &device_info, after_bootstrapping);

``ndn_security_bootstrapping()`` does this job. The first parameters requires a face input where in send the sign-on request to.
The ``after_bootstrapping()`` callback defines the behavior of device instance right after a successful device bootstrapping.
In the quickstart examples, the behavior is subscribes to the LED command and periodically publish content. 

.. code-block:: c

    void
    after_bootstrapping()
    {
        ps_subscribe_to_command(NDN_SD_LED, "", on_light_command, NULL);
        periodic_publish(0, NULL);
        // enable this when you subscribe to content
        //ps_after_bootstrapping();
    }

``on_light_command`` defines the logic upon receiving the command. You can use this as a template when writing command callbacks.

.. code-block:: c

    void
    on_light_command(const ps_event_context_t* context, const ps_event_t* event, void* userdata)
    {
        printf("RECEIVED NEW COMMAND\n");
        printf("Command id: %.*s\n", event->data_id_len, event->data_id);
        printf("Command payload: %.*s\n", event->payload_len, event->payload);
        printf("Scope: %s\n", context->scope);

        int new_val;
        // Execute the function
        if (event->payload) {
            // new_val = *real_payload;
            char content_str[128] = {0};
            memcpy(content_str, event->payload, event->payload_len);
            content_str[event->payload_len] = '\0';
            new_val = atoi(content_str);
        }
        else {
            new_val = 0xFF;
        }
        if (new_val != 0xFF) {
            if ((new_val > 0) != (light_brightness > 0)) {
            if (new_val > 0) {
                printf("Switch on the light.\n");
            }
            else {
                printf("Turn off the light.\n");
            }
            }
            if (new_val < 10) {
            light_brightness = new_val;
            if (light_brightness > 0) {
                printf("Successfully set the brightness = %u\n", light_brightness);
                ps_event_t data_content = {
                .data_id = "a",
                .data_id_len = strlen("a"),
                .payload = &light_brightness,
                .payload_len = 1
                };
                ps_publish_content(NDN_SD_LED, &data_content);
            }
            }
            else {
            light_brightness = 10;
            printf("Exceeding range. Set the brightness = %u\n", light_brightness);
            }
        }
        else {
            printf("Query the brightness = %u\n", light_brightness);
        }
    }

Symmetrically, there's a content subscription callback in ``tutorial-app-sub.c``. You can use that as a template to write content subscription callbacks.

.. code-block:: c

    void
    on_light_data(const ps_event_context_t* context, const ps_event_t* event, void* userdata)
    {
        printf("RECEIVED NEW DATA\n");
        printf("Data id: %.*s\n", event->data_id_len, event->data_id);
        printf("Data payload: %.*s\n", event->payload_len, event->payload);
        printf("Scope: %s\n", context->scope);
    }

Now you can play with ``tutorial-app`` and ``tutorial-app-sub`` in two terminals to see how the Pub/Sub pair works.

Note that ``tutorial-app`` should be online first so that ``tutorial-app-sub`` can request keys on an actual existing service.
Because the "LED" service won't exist until former Device Program register it to the IoT controller.
