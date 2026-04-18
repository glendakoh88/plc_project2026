# BMP Image Filtering System (ANSI C)

## Overview

This project implements a custom image filtering system for **uncompressed 24-bit BMP images** using **ANSI C**.

The program:

* Validates BMP files using a **Finite State Machine (FSM)**
* Parses a custom **filter script**
* Applies transformations directly to pixel data
* Outputs a new processed BMP file

---

## Project Structure

```bash
plc_project2026/
├── main.c
├── bmp.c / bmp.h
├── fsm.c / fsm.h
├── parser.c / parser.h
├── filters.c / filters.h
├── Makefile
├── bmpfilter          # Linux/WSL executable
├── bmpfilter.exe      # Windows executable
├── test.bmp           # Sample input image
├── filters.txt        # Sample filter script
├── output.bmp         # Example output
└── README.md
```

---

## Requirements

* C compiler (GCC recommended)
* Make (for Linux/WSL)
* Windows, Linux, or WSL

---

## Compilation

### On Linux / WSL:

```bash
make
```

### Manual compilation (if make is unavailable):

```bash
gcc -ansi -pedantic -Wall -Werror main.c bmp.c fsm.c parser.c filters.c -o bmpfilter
```

---

## How to Run

### On Windows (PowerShell / VS Code Terminal)

Run the precompiled executable:

```powershell
.\bmpfilter.exe test.bmp filters.txt output.bmp
```

---

### On Linux / WSL (VS Code WSL Terminal)

Run the Linux executable:

```bash
./bmpfilter test.bmp filters.txt output.bmp
```

---

## Input Files

* `test.bmp` → Input BMP image (must be uncompressed, 24-bit)
* `filters.txt` → Script containing filter commands

---

## Output

* `output.bmp` → Processed BMP image

---

## Example Filter Script

```txt
GRAYSCALE
INVERT
BRIGHTNESS 20
FLIP H
```

### Description:

* Convert image to grayscale
* Invert colors
* Increase brightness
* Flip horizontally

---

## How the System Works

1. **Read BMP Headers**
   Extract BMP and DIB headers from input file

2. **Validate Using FSM**
   Ensures file is a valid BMP:

   * Correct signature ("BM")
   * Valid header fields
   * Pixel data within bounds

3. **Parse Filter Script**

   * Reads commands line-by-line
   * Validates syntax and parameters

4. **Apply Filters**

   * Processes pixel data in memory
   * Applies filters in order

5. **Write Output File**

   * Preserves BMP structure
   * Writes updated pixel data

---

## Supported Filters

* `GRAYSCALE` / `GREYSCALE`
* `INVERT`
* `SEPIA`
* `BRIGHTNESS <value>`  (-255 to 255)
* `THRESHOLD <value>`   (0 to 255)
* `CONTRAST <value>`    (-255 to 255)
* `FLIP H` (horizontal)
* `FLIP V` (vertical)

---

## Error Handling

The program detects:

* Invalid BMP files
* Incorrect header values
* Invalid script commands
* Incorrect parameters
* File size inconsistencies

---

## Limitations

* Supports only **uncompressed 24-bit BMP**
* No support for compressed formats (PNG, JPEG)
* Command-line interface only

---

## Future Improvements

* Add more filters (blur, sharpen, edge detection)
* Support additional image formats
* Implement graphical user interface (GUI)
* Optimize memory usage for large images

---

## Team Members

* Celest Ng Song Wei
* Trina Tan Xin Ting
* Janya Mali
* Glenda Koh Yu Fen

---

## Notes

* Script commands are **case-insensitive**
* Extra whitespace is ignored
* Output file will be overwritten if it exists

---

## Reproducibility

To reproduce results:

1. Compile the program (if needed)
2. Use `test.bmp` and `filters.txt`
3. Run the command shown above
4. Check `output.bmp`

---

## Concepts Demonstrated

* File I/O (binary and text)
* Finite State Machines (FSM)
* Custom parser implementation
* Modular ANSI C programming

---
