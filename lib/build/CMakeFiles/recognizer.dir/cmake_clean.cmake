file(REMOVE_RECURSE
  "librecognizer.pdb"
  "librecognizer.so"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/recognizer.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
