## How It Works (Step-by-Step)

1. **Power On**  
   - The `app_main()` function runs automatically on boot.  
   - Initializes Wi-Fi in both **Station** and **Access Point** modes.  

2. **Connect to Router & Host a Network**  
   - **Station Mode** → Connects to your home Wi-Fi using the configured SSID and password.  
   - **Access Point Mode** → Creates its own Wi-Fi network (e.g., `ESP32-AP`) for direct device connections.  

3. **Handle Wi-Fi Events**  
   - The event handler (`wifi_event_handler`) listens for:  
     - **STA Start** – Begin connecting to router.  
     - **STA Connected / Disconnected** – Log events and retry if disconnected.  
     - **STA Got IP** – Store and display assigned IP.  
     - **AP Client Connect / Disconnect** – Log devices joining/leaving the ESP32 network.  

4. **Start HTTP Server**  
   - Configures server defaults using `HTTPD_DEFAULT_CONFIG()`.  
   - Registers the root path `/` to handle GET requests with `hello_get_handler()`.  

5. **Serve Portfolio Webpage**  
   - Visiting the ESP32’s IP triggers the HTML page stored in `hello_get_handler()`.  
   - Displays personal info, background, and embedded systems projects.  

6. **Access from Any Device**  
   - **AP Mode** → Visit `http://192.168.4.1/`  
   - **STA Mode** → Visit the router-assigned IP (shown in UART logs).
