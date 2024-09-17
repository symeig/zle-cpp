FROM centos:8

ARG MPLAPACK_VER="2.0.1"

# Update CentOS repositories and install dependencies
RUN sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-* && \
    sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-*
RUN yum update -y
RUN yum install -y gcc gcc-c++ gcc-gfortran make cmake automake libtool gnuplot
RUN yum install -y python3
RUN yum install -y sudo which patch bzip2 git wget time file
RUN yum install -y emacs
RUN yum install -y epel-release
RUN yum install -y diffutils parallel

# Set up user
ARG DOCKER_UID=1000
ARG DOCKER_USER=docker
ARG DOCKER_PASSWORD=docker
RUN useradd -u $DOCKER_UID -m $DOCKER_USER --shell /bin/bash -G wheel,root && \
    echo "$DOCKER_USER:$DOCKER_PASSWORD" | chpasswd && \
    echo "$DOCKER_USER ALL=(ALL) ALL" >> /etc/sudoers && \
    echo "$DOCKER_USER ALL=NOPASSWD: ALL" >> /etc/sudoers

USER ${DOCKER_USER}
WORKDIR /home/${DOCKER_USER}

# Download and install MPLAPACK
RUN wget https://github.com/nakatamaho/mplapack/releases/download/v${MPLAPACK_VER}/mplapack-${MPLAPACK_VER}.tar.xz && \
    tar xvfJ mplapack-${MPLAPACK_VER}.tar.xz && \
    cd mplapack-${MPLAPACK_VER}

ARG CXX="g++"
ARG CC="gcc"
ARG FC="gfortran"
RUN /bin/bash -c 'set -ex && \
    ARCH=`uname -m` && \
    if [ "$ARCH" == "x86_64" ]; then \
        cd /home/$DOCKER_USER/mplapack-${MPLAPACK_VER} && ./configure --prefix=$HOME/MPLAPACK --enable-gmp=yes --enable-mpfr=yes --enable-_Float128=yes --enable-qd=yes --enable-dd=yes --enable-double=yes --enable-_Float64x=yes --enable-test=yes --enable-benchmark=yes; \
    else \
        cd /home/$DOCKER_USER/mplapack-${MPLAPACK_VER} && ./configure --prefix=$HOME/MPLAPACK --enable-gmp=yes --enable-mpfr=yes --enable-_Float128=yes --enable-qd=yes --enable-dd=yes --enable-double=yes --enable-test=yes --enable-benchmark=yes ; \
    fi'

RUN cd /home/$DOCKER_USER/mplapack-${MPLAPACK_VER} && make -j`getconf _NPROCESSORS_ONLN`
RUN cd /home/$DOCKER_USER/mplapack-${MPLAPACK_VER} && make install

RUN sudo cp -r /home/$DOCKER_USER/MPLAPACK/lib/* /usr/local/lib/
RUN sudo cp -r /home/$DOCKER_USER/MPLAPACK/include/* /usr/local/include/
RUN sudo cp -r /home/$DOCKER_USER/MPLAPACK/bin/* /usr/local/bin/

RUN sudo ldconfig

# Copy the entire zle_src folder
COPY --chown=${DOCKER_USER}:${DOCKER_USER} zle_src /home/${DOCKER_USER}/zle_src

# Copy example file for zle
COPY --chown=${DOCKER_USER}:${DOCKER_USER} example.cpp /home/${DOCKER_USER}/example.cpp

# Build and install zle
RUN cd /home/${DOCKER_USER}/zle_src && \
    mkdir build && \
    cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make && \
    sudo make install && \
    sudo ldconfig

# Set environment variables
ENV PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:${PKG_CONFIG_PATH}"
ENV LD_LIBRARY_PATH="/usr/local/lib:${LD_LIBRARY_PATH}"
ENV CPLUS_INCLUDE_PATH="/usr/local/include:${CPLUS_INCLUDE_PATH}"

# Set the default command to bash
CMD ["/bin/bash"]