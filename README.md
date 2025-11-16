# az_lib_modbus

A header-only Modbus TCP library written in C++20 built on top of the standalone Asio for asynchronous client/server communication and access and control of remote coils and registers.

---

### Features

* **C++20 Standard:** Developed for modern C++ environments.
* **Header-Only:** Simple integration—just include the necessary headers.
* **Asynchronous I/O:** Built on **Asio standalone** for non-blocking network operations.
* **Client & Server Roles:** Supports creating both Modbus Clients and Servers.
* **Supported Function Codes (FCs):**
    * **FC 0x01:** Read Coils
    * **FC 0x02:** Read Discrete Inputs
    * **FC 0x03:** Read Holding Registers
    * **FC 0x04:** Read Input Registers
    * **FC 0x05:** Write Coils
    * **FC 0x06:** Write Registers
* **Data Access:** Clear API for reading and writing **coils** and **registers**.

---

### Getting Started (Prerequisites)

This project is header-only but depends on two external libraries. You **must** clone them into a top-level directory named `external/`.

1.  **Clone the Repository:**
    ```bash
    git clone [YOUR-REPO-URL] az_lib_modbus
    cd az_lib_modbus
    ```
2.  **Install Dependencies:**
    The following commands will set up the required **Asio standalone** and **doctest** libraries in the `external/` folder:
    ```bash
    # Access the external directory
    cd external

    # Clone Asio standalone (required for networking)
    git clone [https://github.com/chriskohlhoff/asio.git](https://github.com/chriskohlhoff/asio.git) external/asio

    # Clone doctest (required for testing)
    git clone [https://github.com/onqtam/doctest.git](https://github.com/onqtam/doctest.git) external/doctest
    ```

---

### Usage and Integration

Since `az_lib_modbus` is **header-only**, you integrate it by including the necessary C++ header files in your project.

#### 1. Modbus Client

To create a Modbus TCP Client, include the following headers in your source file:

```cpp
#include "../src/az_modbus_context.hpp"
#include "../src/az_asio_channel.hpp"
#include "../src/az_modbus_client.hpp"
```

#### 2. Modbus Server

To create a Modbus TCP Server, which requires a mechanism to manage the data (database interface), include these headers:

```cpp
#include "../src/az_modbus_context.hpp"
#include "../src/az_asio_server_transport.hpp"
#include "../src/az_modbus_server.hpp"
#include "../src/az_database_interface.hpp"
```

---

### Contributing

Contributions are welcome! Please feel free to open issues or submit pull requests.