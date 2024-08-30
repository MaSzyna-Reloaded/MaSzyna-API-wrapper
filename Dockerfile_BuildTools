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