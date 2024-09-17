# ZLE (Symbolic Eigenvalue Calculation for Integer Linear Eigenvalues)


## Setup

ZLE can be installed directly on your machine, with MPLAPACK.

This repository also has a Dockerized environment available
for getting running quickly.

1. Ensure you have Docker installed on your system.

2. Clone this repository:
   ```
   git clone https://github.com/your-repo/zle-docker.git
   cd zle-docker
   ```

3. Build the Docker image:
   ```
   docker build -t zle-env:latest .
   ```
   This process may take several minutes as it installs all necessary dependencies and builds MPLAPACK and ZLE.

## Usage

1. Run a container from the image:
   ```
   docker run -it --rm zle-env:latest
   ```
   This command starts an interactive bash session in the container.

2. Inside the container, you can now use the ZLE library in your C++ programs.

### Example

Here's a simple example of how to use the ZLE library:

1. Create a file named `example.cpp` with the following content:

   ```cpp
   #include <zle>
   #include <iostream>
   #include <vector>
   #include <string>

   int main() {
       int n = 3;
       std::vector<std::string> symbols = {"x0", "x1", "x2"};
       std::vector<std::vector<SymX>> A = {
           {SymX{1, "x0"}, SymX{1, "x1"}, SymX{1, "x2"}},
           {SymX{1, "x1"}, SymX{1, "x0"}, SymX{1, "x2"}},
           {SymX{1, "x2"}, SymX{1, "x1"}, SymX{1, "x0"}}
       };

       auto result = zle_eigs(A, symbols);

       std::cout << "Eigenvalues:" << std::endl;
       for (const auto& ev : result) {
           for (int coeff : ev) {
               std::cout << coeff << " ";
           }
           std::cout << std::endl;
       }

       return 0;
   }
   ```

2. Compile the example:
   ```
   g++ -o example example.cpp $(pkg-config --cflags --libs zle)
   ```

3. Run the example:
   ```
   ./example
   ```

This should output the symbolic eigenvalues of the matrix defined in the example.
