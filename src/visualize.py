# --------------------------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See License.txt in the project root for license information.
# --------------------------------------------------------------------------------------------

"""
This sample demonstrates how to use the Microsoft Azure Event Hubs Client for Python sync API to 
read messages sent from a device. Please see the documentation for @azure/event-hubs package
for more details at https://pypi.org/project/azure-eventhub/

For an example that uses checkpointing, follow up this sample with the sample in the 
azure-eventhub-checkpointstoreblob package on GitHub at the following link:

https://github.com/Azure/azure-sdk-for-python/blob/master/sdk/eventhub/azure-eventhub-checkpointstoreblob/samples/receive_events_using_checkpoint_store.py
"""


# from azure.eventhub import TransportType
# from azure.eventhub import EventHubConsumerClient
# import matplotlib.pyplot as plt
# import matplotlib.animation as animation
# from matplotlib import style
# import numpy as np

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from azure.eventhub import EventHubConsumerClient
import threading


#connection string : Endpoint=sb://ihsuprodswitzerlandnorthres004dednamespace.servicebus.windows.net/;SharedAccessKeyName=iothubowner;SharedAccessKey=CUavAqLbkyImrAodoWgqjkYiyOBu9tE+vAIoTHWSCf0=;EntityPath=iothub-ehub-piiotfree-55805419-6d0c9df87b


# Event Hub-compatible endpoint
# az iot hub show --query properties.eventHubEndpoints.events.endpoint --name {your IoT Hub name}
#EVENTHUB_COMPATIBLE_ENDPOINT = "{your Event Hubs compatible endpoint}"

# Event Hub-compatible name
# az iot hub show --query properties.eventHubEndpoints.events.path --name {your IoT Hub name}
#EVENTHUB_COMPATIBLE_PATH = "{your Event Hubs compatible name}"

# Primary key for the "service" policy to read messages
# az iot hub policy show --name service --query primaryKey --hub-name {your IoT Hub name}
#IOTHUB_SAS_KEY = "{your service primary key}"

# If you have access to the Event Hub-compatible connection string from the Azure portal, then
# you can skip the Azure CLI commands above, and assign the connection string directly here.
CONNECTION_STR = f'Endpoint=sb://ihsuprodswitzerlandnorthres004dednamespace.servicebus.windows.net/;SharedAccessKeyName=iothubowner;SharedAccessKey=CUavAqLbkyImrAodoWgqjkYiyOBu9tE+vAIoTHWSCf0=;EntityPath=iothub-ehub-piiotfree-55805419-6d0c9df87b'


data = []

def on_event_batch(partition_context, events):
    global data
    for event in events:
        try:
            distance = float(
                event.body_as_str().split(":")[1].replace("}", "")
            )
            data.append(distance)

            with open("database.json", "a") as f:
                f.write(str(distance))
                f.write("\n")
            
        except Exception as e:
            print("Parse error:", e)

    partition_context.update_checkpoint()


def on_error(partition_context, error):
    print("Error:", error)


def eventhub_thread():
    client = EventHubConsumerClient.from_connection_string(
        conn_str=CONNECTION_STR,
        consumer_group="$default"
    )
    with client:
        client.receive_batch(
            on_event_batch=on_event_batch,
            on_error=on_error
        )


def animate(i):
    xs = np.arange(len(data))
    ys = np.array(data)

    ax.clear()
    ax.plot(xs, ys)
    if data:
        ymin = 0
        ymax = max(data)
        margin = (ymax - ymin) *0.1

        ax.set_ylim(0, ymax + margin)


# -------- MAIN --------
threading.Thread(target=eventhub_thread, daemon=True).start()

fig, ax = plt.subplots()
ani = FuncAnimation(fig, animate, interval=500)
plt.show()
