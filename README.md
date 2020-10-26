# binbutton
Qt-based app for operations on data in the clipboard. Designed to aid in quick transformations of scientific data such as spectra or two-dimensional maps.

Data format:
- plain text format
- rows separated by newline character, values in row by any other whitespace
- supports single column, two column or rectangular data. Supports additional column and row for x and y coordinates in case of rectangular data array
- dot as a decimal point separator
- attempts to cleanup text or other characters by ignoring invalid rows or taking only part of the row

Features:
- bin data by averaging each two consecutive rows (hence the project name)
- transform between wavelength in air and corresponding photon energy with (hardcoded for now) constant of λ*E = 1239495.2775 nm * meV. Assumes wavelengths or energies are in the first column.
- transpose data matrix or row/column
- flatten data matrix (sum all columns to one)
- reverse row order
- perform operations on two datasets. One dataset (called background here) can be saved to separate buffer. Then, data in clipboard can be dividedar multiplied by background or it can have background added or subtracted. App will try to resolve cases, when background has other dimensions than data (f. eg. when subtracting single column background from 2D data matrix)
- two text areas user can temporarily paste his data into
- always-on-top behavior, hides into taskbar when closed
- support for custom physical keyboard operating via serial port
