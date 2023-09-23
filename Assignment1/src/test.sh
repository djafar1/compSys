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
printf " Hello, World! " > test_files/ascii4.input
printf "Hello, World! Hello, World! Hello, World!" > test_files/Longascii4.input


# ISO-8859-1 tests
printf "\xe6 \n \xe6" > test_files/iso8859.input
printf "\xff\Hello, World! Hello, World! Hello, World!" > test_files/Longiso88592.input

# DATA tests
printf "Hello,\x00World!\n" > test_files/data.input
printf "Hello,\x00World!" > test_files/data2.input

 
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
