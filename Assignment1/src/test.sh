#!/usr/bin/env bash

# Exit immediately if any command below fails.
set -e

make


echo "Generating a test_files directory.."
mkdir -p test_files
rm -f test_files/*


echo "Generating test files.."
printf "Hello, World!\n" > test_files/ascii.input
printf "Hello, World!" > test_files/ascii2.input
printf "Hello,\x00World!\n" > test_files/data.input
printf "" > test_files/empty.input

# ASCII tests
printf "       " > test_files/OnlySpacesAscii3.input
printf "1234567890" > test_files/ascii3.input
printf " Hello, World! " > test_files/ascii4.input
printf "Hello, World! Hello, World! Hello, World!" > test_files/Longascii4.input
printf "Line 1\nLine 2\nLine 3\n" > test_files/ascii5.input
printf "Special Ch@ract3rs!" > test_files/ascii6.input
printf "\x21\x40\x23\x24\x25\x2A\x28\x29\x2D\x3D\x5B\x5D\x7B\x7D\x7C\x3B\x3A\x27\x22\x2C\x2E\x3C\x3E\x3F\x2F\x5C\x7E" > test_files/ascii9.input #Testing ASCII characters
printf "\x41\x42\x43\x44\x45\x46\x47\x48\x49\n" > test_files/ascii10.input  # ASCII characters A-I


# ISO-8859-1 tests
printf "\xe6 \n \xe6" > test_files/iso8859.input
printf "\xff\Hello, World! Hello, World! Hello, World!" > test_files/Longiso88592.input
printf "\xFF \n" > test_files/iso8859_hex_5.input

# DATA tests
printf "Hello,\x00World!\n" > test_files/data.input
printf "Hello,\x00World!" > test_files/data2.input
printf "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09" > test_files/data3.input

# Secret tests
printf "hemmelighed" > test_files/Secret.input
chmod -r test_files/Secret.input

# UTF-8 tests
printf "⠀\n" > test_files/unicode-character.input
printf "😀\n" > test_files/emoji.input
printf "😀Hello, World! Hello, World!\n" > test_files/LongUnicode.input
# 1-byte sequence should return ASCII 
printf "a\n" > test_files/utf8_1byte.input
# 2-byte sequence
printf "\xC2\xA9\n" > test_files/utf8_2byte.input  # ©
# 3-byte sequence
printf "\xE2\x82\xAC\n" > test_files/utf8_3byte.input  # €
# 4-byte sequence
printf "\xF0\x9F\x8E\x89\n" > test_files/utf8_4byte.input  # 🎉


echo "Running the tests.."
exitcode=0
for f in test_files/*.input
do
  echo ">>> Testing ${f}.."
  file    ${f} | sed -e 's/ASCII text.*/ASCII text/' \
                         -e 's/UTF-8 Unicode text.*/UTF-8 Unicode text/' \
                         -e 's/ISO-8859 text.*/ISO-8859 text/' \
                         -e 's/writable, regular file, no read permission/cannot determine (Permission denied)/' \
                         > "${f}.expected"
  ./file  "${f}" > "${f}.actual"

  if ! diff -u "${f}.expected" "${f}.actual"
  then
    echo ">>> Failed :-("
    exitcode=1
  else
    echo ">>> Success :-)"
  fi
done
exit $exitcode
