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
<<<<<<< HEAD
printf "Hello,\x00World!\n" > test_files/data.input
printf "" > test_files/empty.input
printf "⠀\n" > test_files/unicode-character.input

### TODO: Generate more test files ###

=======
printf "       " > test_files/OnlySpacesAscii3.input
printf " Hello, World! " > test_files/ascii4.input
printf "\xe6 \n \xe6" > test_files/iso8859.input
printf "Hello,\x00World!\n" > test_files/data.input
printf "" > test_files/empty.input
printf "hemmelighed" > hemmelig_fil.input
chmod -r hemmelig_fil.input
>>>>>>> 8d4e92d6cb24c4153661d2bf81943844f76716dc

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
