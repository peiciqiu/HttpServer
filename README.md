# HttpServer

<!-- Improved compatibility of back to top link: See: https://github.com/othneildrew/Best-README-Template/pull/73 -->
<a name="readme-top"></a>

<!-- ABOUT THE PROJECT -->
## About The Project
This project is a multi-threaded SQL Database and HTTP Server written in C++
* Connection Handling: designed a server daemon thread that listens for new client TCP connections and dispatches a thread from the thread pool to execute connection handling, request parsing, and database query tasks
* Database: implemented a na ̈ıve SQL database that users can query by sending HTTP requests to the web server
* Multi-threading: coordinated database access with mutex-based shared and exclusive locks for data integrity

## Getting Started

You will need a C++ development environment to build this project. For more details about setting up C++ dev env, see [https://code.visualstudio.com/docs/cpp/config-mingw](https://code.visualstudio.com/docs/cpp/config-mingw)

### Installation

1. Clone the repo
   ```sh
   git clone git@github.com:peiciqiu/HttpServer.git
   ```
2. cd into the project
   ```sh
   cd HttpServer
   ```
2. Build the project
   ```sh
   make
   ```
## Project Structure
TODO

<p align="right">(<a href="#readme-top">back to top</a>)</p>
