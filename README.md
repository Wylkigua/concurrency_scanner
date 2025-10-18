# Scanner Library

Кроссплатформенная утилита написанная на __С++20__ для сканирования директорий и поиска файлов по базе хэшей.  
Поддерживает многопоточное вычисление MD5-хэшей, асинхронное логирование.

Использует публичный API библиотеки сканирования `scanner_core`
исходный код библиотеки находится `library_scanner/src`, API `library_scanner/include`

# Dependencies
* __OpenSSL__

# Build Windows
Использовался компилятор __MSVC 19.44.35217.0__ _Visual Studio 2022 Developer Command Prompt v17.14.16_ 
```cmd
mkdir build
cmake -S . -B build -G "NMake Makefiles"
cd build
nmake
scanner.exe --base base.cvs --log report.log --path directory
```

# Build Linux
Использовался компилятор __g++ 13.3.0__ _Ubuntu 24.04.3 LTS release 24.04_
```bash
mkdir build && cmake -S . -B build
cmake --build build
./build/scanner --base base.cvs --log report.log --path directory
```

# Tests
Библиотека тестировалась c использованием фреймворка __GTest__ на:
* __MSVC 19.44.35217.0__ _Visual Studio 2022 Developer Command Prompt v17.14.16_ 

* __g++ 13.3.0__ _Ubuntu 24.04.3 LTS release 24.04_

тесты находятся в `library_scanner/tests`
## build test Windows
```cmd
mkdir build
cmake -S . -B build -G "NMake Makefiles"
cd build
nmake
scanner_tests.exe 
```
## build test Linux
```bash
mkdir build && cmake -S . -B build
cmake --build build
./build/scanner_tests
```