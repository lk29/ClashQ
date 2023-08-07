# Build for Windows

## Build openssl

Install [Perl](https://strawberryperl.com/) and [NASM](https://www.nasm.us/pub/nasm/releasebuilds/?C=M;O=D), then clone OpenSSL repository from github:

```
git clone https://github.com/openssl/openssl.git
cd openssl
git checkout OpenSSL_1_1_1u
```

Open "x64 Native Tools Command Prompt" and follow the instructions in INSTALL to build, for example:

```
$ perl Configure VC-WIN64A
$ nmake
```
