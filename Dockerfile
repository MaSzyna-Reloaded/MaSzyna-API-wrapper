FROM ubuntu:latest
ENV ANDROID_SDK_ROOT=/root/sdk
ENV PROJECT_DIR=/var/maszyna
ENV ANDROID_NDK_VERSION=23.2.8568313
ENV ANDROID_NDK_ROOT=${ANDROID_SDK_ROOT}/ndk/${ANDROID_NDK_VERSION}
ENV OSX_SDK_VERSION=14.5
ENV OSXCROSS_ROOT=/root/osxcross
ENV PATH="${OSXCROSS_ROOT}/target/bin:${PATH}"
LABEL authors="DoS"
RUN apt update
RUN apt upgrade -y
RUN apt install -y scons g++-mingw-w64-x86-64 gcc-mingw-w64 git-all build-essential cmake gcc-multilib g++-multilib libc6-dev-i386 g++-mingw-w64-i686
RUN apt install -y openjdk-17-jdk openjdk-17-jre libncurses5-dev libncursesw5-dev curl unzip zip libssl-dev lzma-dev libxml2-dev zlib1g-dev
RUN echo "Installing Android SDK..."
RUN mkdir -p ${ANDROID_SDK_ROOT} && cd ${ANDROID_SDK_ROOT}
RUN export CMDLINETOOLS=commandlinetools-linux-11076708_latest.zip && \
        curl -LO https://dl.google.com/android/repository/${CMDLINETOOLS} && \
        unzip ${CMDLINETOOLS} && \
        rm ${CMDLINETOOLS} && \
        yes | cmdline-tools/bin/sdkmanager --sdk_root="${ANDROID_SDK_ROOT}" --licenses && \
        cmdline-tools/bin/sdkmanager --sdk_root="${ANDROID_SDK_ROOT}" "ndk;${ANDROID_NDK_VERSION}" 'cmdline-tools;latest' 'build-tools;34.0.0' 'platforms;android-34' 'cmake;3.22.1'
RUN echo "Copying MacOSX14.5 SDK..."
ADD docker-storage/MacOSX${OSX_SDK_VERSION}.sdk.tar.xz /root/files/MacOSX${OSX_SDK_VERSION}.sdk.tar.xz
RUN echo "Installing OSXCross..."
RUN cd /root && git clone https://github.com/tpoechtrager/osxcross && \
        mkdir -p ${OSXCROSS_ROOT} && \
        cd ${OSXCROSS_ROOT} && \
        git checkout ff8d100f3f026b4ffbe4ce96d8aac4ce06f1278b && \
        curl -LO https://github.com/tpoechtrager/osxcross/pull/415.patch && \
        git apply 415.patch && \
        ln -s /root/files/MacOSX${OSX_SDK_VERSION}.sdk.tar.xz /root/osxcross/tarballs
RUN cd ${OSXCROSS_ROOT} && export UNATTENDED=1 && \
        export SDK_VERSION=${OSX_SDK_VERSION} && \
        CLANG_VERSION=16.0.0 ENABLE_CLANG_INSTALL=1 INSTALLPREFIX=/usr ./build_apple_clang.sh
#RUN cd ${OSXCROSS_ROOT} && ./build.sh
#RUN cd ${OSXCROSS_ROOT} && ./build_compiler_rt.sh
#RUN rm -rf ${OSXCROSS_ROOT}/build
