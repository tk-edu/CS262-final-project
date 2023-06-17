# Building
**Note**: The project folder and the folders of each dependency should be on a path with *no spaces* in any folder name. Things will break otherwise.

# XOR Encryption Version

## Dependencies
- WinSock2.h
- Ws2tcpip.h
- Windows.h

## Compiling
Run `make all exe` to build and clean up object files. Alternatively, run `make clean` to clean up object files and the final binary.

# RSA Encryption Version
**Note**:
This version is incomplete, as it does not implement the website-related logic.

## Dependencies
- [libgcrypt](https://www.gnupg.org/download/index.html)
- [libgpg-error](https://www.gnupg.org/download/index.html)

You'll need to compile these libraries (libgpg-error first, as it's a dependency of libgcrypt). On Windows 10 using mingw64 (from [MSYS2](https://www.msys2.org/) installation), I compiled them using the following commands (from the respective library folders):
- For libgpg-error:
    > `./configure --prefix="PATH/TO/libgpg-error-1.46" --enable-install-gpg-error-config; make; make install`
- For libgcrypt:
    > `./configure --prefix="PATH/TO/libgcrypt-1.10.1" --with-libgpg-error-prefix="PATH/TO/libgpg-error-1.46"; make; make install`

After compiling `libgpg-error6-0.dll` and `libgcrypt-20.dll`, place them in the `bin` folder.

## Compiling
Run this command from the project folder:
> `gcc -o bin/main.exe src/main.c $(PATH/TO/libgcrypt-1.10.1/bin/libgcrypt-config --cflags --libs)`

## Debugging with VSCode
Change `PATH\\TO` in the `settings.json` and `tasks.json` files to the respective locations of the specified files/folders.

**Note**:
For the `args` in `tasks.json` that start with `-L` or `-I`, make sure to leave the `-L` and `-I`'s intact, and only fill in your paths after that.