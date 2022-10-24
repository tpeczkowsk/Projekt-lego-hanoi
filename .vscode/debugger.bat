set PATH=c:\Users\Tomek\.vscode\HusarionTools\bin\;%PATH%
cd d:\STUDIA\Semestr 7\Mechatronic Design\Projekt || exit 1
start /wait st-flash write myproject.bin 0x08010000 || exit 1
start st-util
arm-none-eabi-gdb %*