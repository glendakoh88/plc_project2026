# BMP Image Filtering System (ANSI C)

## 📌 Overview

This project implements a custom image filtering system for **uncompressed BMP files** using **ANSI C**.
Users can apply a sequence of filters to an image by specifying commands in a script file.

The system is designed to be:

* Modular
* Extensible
* Transparent (no hidden processing like typical image tools)

---

## 🎯 Features

* Reads and validates BMP files using a **Finite State Machine (FSM)**
* Custom **script parser** for filter commands
* Applies transformations directly to pixel data
* Robust handling of invalid inputs and edge cases
* Fully compliant with **ANSI C** (`-Wall -Werror -ansi -pedantic`)

---

## 📂 Project Structure

```
.
├── main.c          # Entry point
├── bmp.c / bmp.h   # BMP file handling
├── fsm.c / fsm.h   # FSM validation logic
├── parser.c / parser.h  # Script parsing
├── filter.c / filter.h  # Image filters
├── Makefile        # Build instructions
└── examples/       # Sample BMP and script files
```

---

## ⚙️ Requirements

* GCC or compatible C compiler
* Unix-like environment (Linux / Mac) or Windows with MinGW
* Make utility

---

## 🔧 Compilation

Run the following command:

```bash
make
```

This will compile the program and generate the executable.

---

## ▶️ Usage

```bash
./program input.bmp script.txt output.bmp
```

### Arguments:

* `input.bmp` → Input BMP image
* `script.txt` → Filter script file
* `output.bmp` → Output filtered image

---

## 🧾 Example Script

```
GRAYSCALE
INVERT
BRIGHTNESS 20
```

### Description:

1. Convert image to grayscale
2. Invert colors
3. Increase brightness by 20

---

## 🧠 How It Works

1. **Read BMP Headers**
   Extract BMP and DIB headers from the input file

2. **Validate with FSM**
   Ensure file integrity (format, size, offsets, etc.)

3. **Parse Script**
   Read and validate filter commands

4. **Apply Filters**
   Modify pixel data based on commands

5. **Write Output**
   Save processed image as a valid BMP file

---

## ⚠️ Error Handling

The program detects and reports:

* Invalid BMP files
* Incorrect header values
* Unknown filter commands
* Invalid parameters
* File size inconsistencies

---

## 🚧 Limitations

* Supports only **uncompressed BMP (24-bit)**
* No support for compressed formats (PNG, JPEG)
* Command-line interface only

---

## 🚀 Future Improvements

* Add more filters (blur, sharpen, edge detection)
* Support additional image formats
* Implement GUI interface
* Optimize memory usage for large images

---

## 👥 Team Members

* Celest Ng Song Wei
* Trina Tan Xin Ting
* Janya Mali
* Glenda Koh Yu Fen

---

## 📖 Notes

* Ensure input BMP files are valid and uncompressed
* Script commands are case-insensitive
* Whitespace in scripts is ignored

---

## 📌 How to Reproduce

1. Compile the program using `make`
2. Use sample files from `/examples`
3. Run the command shown above

---

## 🏁 Conclusion

This project demonstrates the use of:

* File I/O (binary and text)
* Finite State Machines
* Custom parsing
* Modular C programming

to build a robust and extensible image processing system.

---
