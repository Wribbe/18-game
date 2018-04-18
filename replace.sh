#!/bin/sh
[ $# -gt 1 ] || {
  echo '[Error:] Need 2 arguments.' && exit
}

from="$1"
to="$2"

read -p "[Info:] Will try to replace < $from > with < $to >, continue? y/n: " yn
case $yn in
  [yY]) ;;
  *) exit
esac

file_list=$(find . -name .git -prune -o \( \! -name .\* -print \))
final_files=''
# Do any files need changing?
for filepath in $file_list; do
  # Ignore directories.
  [ ! -d $filepath ] || continue
  filename=${filepath##*/}
  new_filepath=$filepath
  case $filename in
    *$from*)
      path=${filepath%%$filename*}
      new_filename=$(echo $filename | sed "s/$from/$to/g")
      new_filepath="$path$new_filename"
      read -p "[Info:] Move $filepath to $new_filepath? y/n: " yn
      case $yn in
        [yY])
          err=$(git mv $filepath $new_filepath 2>&1)
          [ $? -eq 0 ] || { echo "[Error:] Will not rename files not in git, <$err>" && exit; }
        ;;
      esac
    ;;
  esac
  final_files="$final_files $new_filepath"
done

# Replace file content.
echo "$final_files" | xargs sed -i "s/$from/$to/g"
