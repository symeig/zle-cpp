# ZLE (Symbolic Integer-Linear Eigenvalue Calculation)


## Setup

ZLE can be installed directly on your machine using the included cmake file, and requires [MPLAPACK](https://github.com/nakatamaho/mplapack) as an installed library on the system.

This repository also has a Dockerized environment available for getting running quickly which will have both MPLAPACK and ZLE available inside.

1. Ensure you have [Docker](https://www.docker.com/products/docker-desktop/) installed on your system.

2. Clone this repository:
   ```
   git clone git@github.com:symeig/zle-cpp.git
   cd zle-cpp
   ```

3. Build the Docker image:
   ```
   docker build -t mplapack-zle:latest .
   ```
   This process may take a long time, as it downloads, builds, and installs all dependencies, including MPLAPACK.

4. Run a container from the image:
   ```
   docker run -it --rm mplapack-zle:latest
   ```
   This command starts an interactive bash session in the container.

5. Compile the included example:
   ```
   g++ -o example example.cpp $(pkg-config --cflags --libs zle)
   ```

6. Run the example:
   ```
   ./example
   ```

This should output the symbolic eigenvalues of the matrix defined in the example.
