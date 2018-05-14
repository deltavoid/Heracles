

注意事项：
1. network_controller.cpp第6行: 需要更改一下default_device，这个我应该写成环境变量
2. 装一下bcc的库
3. 使用方法是通过run.sh 启动程序后在命令行通过lc <pid> 和 be <pid>传入pid
4. console不是必要的，是为了方便调试

bcc的安装：
### Install build dependencies
```
# Trusty and older
VER=trusty
echo "deb http://llvm.org/apt/$VER/ llvm-toolchain-$VER-3.7 main
deb-src http://llvm.org/apt/$VER/ llvm-toolchain-$VER-3.7 main" | \
  sudo tee /etc/apt/sources.list.d/llvm.list
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get update

# All versions
sudo apt-get -y install bison build-essential cmake3 flex git libedit-dev \
  libllvm3.7 llvm-3.7-dev libclang-3.7-dev python zlib1g-dev libelf-dev

# For Lua support
sudo apt-get -y install luajit luajit-5.1-dev
```

### Install and compile BCC
```
git clone https://github.com/iovisor/bcc.git
mkdir bcc/build; cd bcc/build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
```

注意库一定要装全，不然cmake会出错。
make install 是必要的，现在的makefile采用的是通过系统库编译的方式。
