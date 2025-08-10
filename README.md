1. Power On
When the ESP32 boots, app_main() runs and initializes Wi-Fi in both Station and Access Point modes.

2. Connect to Router & Host Own Network
Station Mode: The ESP32 connects to your home Wi-Fi using the SSID/password you configured.

Access Point Mode: Simultaneously, it creates its own Wi-Fi network (e.g., ESP32-AP) that other devices can join directly.

3. Wi-Fi Events Trigger Automatically
The event handler monitors key Wi-Fi events:

STA Start → Begin connection

STA Connected / Disconnected → Log status and retry on disconnect

STA Got IP → Receive and log the assigned IP address via DHCP

4. Launch HTTP Server
Once Wi-Fi is running:

The HTTP server starts with default settings.

The root path / is registered so GET requests to it invoke hello_get_handler().

5. Serve Your Webpage
When someone navigates to the ESP32’s IP (either the AP’s 192.168.4.1 or the STA’s IP returned by the router):

The server responds by sending your custom HTML portfolio page with details about your projects and contacts.

6. View in Any Browser
You—or any client device—can open the ESP32’s web page by entering the correct IP in a browser. The portfolio page is displayed instantly, hosted right from the ESP32.
