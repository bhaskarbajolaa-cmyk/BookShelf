# 1. Start with a fresh Ubuntu 22.04 image
FROM ubuntu:22.04

# 2. Prevent interactive prompts from blocking the installation
ENV DEBIAN_FRONTEND=noninteractive

# 3. Install the compiler and all necessary libraries
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libpqxx-dev \
    libpq-dev \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

# 4. Set the working directory inside the container
WORKDIR /app

# 5. Copy your project files into the container
COPY Makefile ./
COPY src/ ./src/
COPY include/ ./include/

# 6. Compile the C++ application
RUN make clean && make

# 7. Open port 8080 for the web server
EXPOSE 8080

# 8. Command to run when the container starts
CMD ["./bin/bookshelf"]